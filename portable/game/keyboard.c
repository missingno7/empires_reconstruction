/* keyboard.c -- keyboard service (src/KEYBOARD.C + src/KEYIRQ.C) over the
 * platform-fed input_key_event() interface (portable/include/input.h).
 *
 * Two historical surfaces are reproduced here:
 *  1. Level/edge state written by keyboard_irq_handler (F_699E): the body
 *     below is that switch translated literally, scancode for scancode,
 *     including the g856-gated numeric-keypad aliases and the Ctrl+S sound
 *     toggle.
 *  2. The BIOS INT 16h keystroke FIFO (F_6B1A/F_6B4A/F_6B66/F_6B74's
 *     callers): a 15-entry (ascii, scancode) ring, sized like the real BIOS
 *     type-ahead buffer, guarded by a mutex/condvar since input_key_event()
 *     runs on the SDL main thread while the FIFO is read from the game
 *     thread (docs/portable/architecture.md, "Input model").
 *
 * Not-yet-ported callees (sound_effects_toggle, menu_list_active,
 * menu_loop_run) are stubbed in keyboard_stubs.c.
 */
#include "input.h"
#include "sync.h"

/* ---- DGROUP state: key_up_held/gb6a/key_up_left_held/key_up_right_held/
 * key_up_released/keyboard_state (DS:0B68..0B76) and b856 (DS:0856) are
 * initialized DATA objects and therefore defined by the generated
 * portable/generated/game_data.c (all zero historically). ---- */

/* Not-yet-ported callees; see portable/game/keyboard_stubs.c. */
extern void    sound_effects_toggle(void);
extern dos_int menu_list_active(void);
extern void    menu_loop_run(dos_int index);

/* ---- BIOS-style keystroke FIFO ---- */
#define KBD_FIFO_CAP 15  /* the historical BIOS ring holds 15 usable entries */

typedef struct {
    uint8_t ascii;
    uint8_t scan;
} kbd_fifo_entry;

static kbd_fifo_entry s_fifo[KBD_FIFO_CAP];
static int            s_fifo_head;   /* index of the oldest entry */
static int            s_fifo_count;
static sync_mutex     s_fifo_mutex;
static sync_cond      s_fifo_cond;
static bool           s_fifo_ready;  /* mutex/cond lazily initialized */

static void fifo_ensure_init(void)
{
    if (!s_fifo_ready) {
        sync_mutex_init(&s_fifo_mutex);
        sync_cond_init(&s_fifo_cond);
        s_fifo_ready = true;
    }
}

/* Push one (ascii, scan) pair.  Drops the newest entry when the ring is
 * full, matching the BIOS "buffer full" beep-and-drop behaviour. */
/* The ROM BIOS INT 9 handler that the historical driver chained to never
 * buffers shift-state keys: Ctrl, Shift, Alt, CapsLock, NumLock, ScrollLock
 * (and Insert only toggles state on some BIOSes -- it IS buffered, so it is
 * not listed).  Model that here since there is no ROM to chain to. */
static bool bios_is_modifier(uint8_t scan)
{
    switch (scan) {
    case 0x1d: case 0x2a: case 0x36: case 0x38: case 0x3a: case 0x45: case 0x46:
        return true;
    default:
        return false;
    }
}

static void fifo_push(uint8_t ascii, uint8_t scan)
{
    fifo_ensure_init();
    sync_mutex_lock(&s_fifo_mutex);
    if (s_fifo_count < KBD_FIFO_CAP) {
        int tail = (s_fifo_head + s_fifo_count) % KBD_FIFO_CAP;
        s_fifo[tail].ascii = ascii;
        s_fifo[tail].scan = scan;
        s_fifo_count++;
        sync_cond_signal(&s_fifo_cond);
    }
    sync_mutex_unlock(&s_fifo_mutex);
}

/* Non-blocking peek at the head entry (BIOS INT 16h AH=1: report, don't
 * consume). */
static bool fifo_peek(uint8_t *ascii, uint8_t *scan)
{
    bool got;

    fifo_ensure_init();
    sync_mutex_lock(&s_fifo_mutex);
    got = s_fifo_count > 0;
    if (got) {
        *ascii = s_fifo[s_fifo_head].ascii;
        *scan = s_fifo[s_fifo_head].scan;
    }
    sync_mutex_unlock(&s_fifo_mutex);
    return got;
}

/* Non-blocking pop of the head entry (BIOS INT 16h AH=0 once ZF says a key
 * is ready). */
static bool fifo_pop(uint8_t *ascii, uint8_t *scan)
{
    bool got;

    fifo_ensure_init();
    sync_mutex_lock(&s_fifo_mutex);
    got = s_fifo_count > 0;
    if (got) {
        *ascii = s_fifo[s_fifo_head].ascii;
        *scan = s_fifo[s_fifo_head].scan;
        s_fifo_head = (s_fifo_head + 1) % KBD_FIFO_CAP;
        s_fifo_count--;
    }
    sync_mutex_unlock(&s_fifo_mutex);
    return got;
}

static void fifo_reset(void)
{
    fifo_ensure_init();
    sync_mutex_lock(&s_fifo_mutex);
    s_fifo_head = 0;
    s_fifo_count = 0;
    sync_mutex_unlock(&s_fifo_mutex);
}

/* Block the game thread until the FIFO is non-empty.  keyboard_read_
 * blocking_hotkeys() calls this instead of waiting on the condvar itself so
 * a caller with data already queued never blocks (tests pre-fill the FIFO
 * and rely on exactly that). */
void input_platform_wait_key(void)
{
    fifo_ensure_init();
    sync_mutex_lock(&s_fifo_mutex);
    while (s_fifo_count == 0) {
        sync_cond_wait(&s_fifo_cond, &s_fifo_mutex);
    }
    sync_mutex_unlock(&s_fifo_mutex);
}

/* ---- F_699E: keyboard_irq_handler, translated literally ----
 * The historical handler read the raw make/break byte from port 0x60 into
 * `scan` and used scan&0x80 to tell a break from a make of the same key.
 * The platform layer already splits that into (scancode, down); rebuild
 * `scan` so the break-only `if(scan&0x80)` tests below stay a literal copy
 * of src/KEYBOARD.C. */
void input_key_event(uint8_t scancode, bool down, uint8_t ascii)
{
    uint8_t scan = (uint8_t)(scancode | (down ? 0x00 : 0x80));
    dos_int dn = (dos_int)(down ? 1 : 0);
    int handled = 1;

    switch (scan & 0x7f) {
    case 0x58: if (!g856) break; /* fallthrough: g856-gated alias of 0x47 */
    case 0x47: key_up_held = key_up_left_held = dn; if (scan & 0x80) key_up_released = 1; break;
    case 0x49: key_up_held = key_up_right_held = dn; if (scan & 0x80) key_up_released = 1; break;
    case 0x29: if (!g856) break; /* fallthrough: g856-gated alias of 0x48 */
    case 0x48: key_up_held = dn; if (scan & 0x80) key_up_released = 1; break;
    case 0x2b: if (!g856) break; /* fallthrough: g856-gated alias of 0x4b */
    case 0x4b: key_up_left_held = dn; break;
    case 0x4e: if (!g856) break; /* fallthrough: g856-gated alias of 0x4d */
    case 0x4d: key_up_right_held = dn; break;
    case 0x4a: if (!g856) break; /* fallthrough: g856-gated alias of 0x50 */
    case 0x50: gb6a = dn; break;
    case 0x46: case 0x54: break;
    case 0x1f: if (gb74 && dn) sound_effects_toggle(); else handled = 0; break;
    case 0x1d: gb74 = dn; /* fallthrough */
    default: handled = 0; break;
    }

    /* Historical: !handled or the chain gate calls the saved INT 9 vector,
     * which the real BIOS eventually turns into a type-ahead-buffer entry
     * for a make event; handled-and-gate-closed acks the controller and the
     * key never reaches the buffer.  We have no real BIOS to chain to, so
     * "chain" here means "push into our own FIFO" -- break events are
     * excluded explicitly since the BIOS never buffers a break. */
    if ((!handled || keyboard_state[0]) && down && !bios_is_modifier(scancode)) {
        fifo_push(ascii, scancode);
    }
}

/* ---- F_6990 / F_6997 / F_6B74 ---- */
void keyboard_chain_enable(void)  { keyboard_state[0] = 1; }
void keyboard_chain_disable(void) { keyboard_state[0] = 0; }
dos_int keyboard_chain_active(void) { return keyboard_state[0]; }

/* ---- F_695E / F_697D ----
 * No real IRQ vector to save/restore here.  Install resets the level/edge
 * state and chain gate to their post-boot values and empties the FIFO;
 * restore just empties the FIFO (there is nothing else to undo). */
void keyboard_irq_install(void)
{
    key_up_held = 0;
    key_up_left_held = 0;
    key_up_right_held = 0;
    key_up_released = 0;
    gb6a = 0;
    keyboard_state[0] = 0;
    keyboard_state[1] = 0;
    fifo_reset();
}

void keyboard_irq_restore(void)
{
    fifo_reset();
}

/* ---- F_6B1A: keyboard_read_blocking_hotkeys ----
 * asm xor ah,ah / int 16h / or al,al / jnz L_ascii returns the ASCII with AH
 * cleared when the BIOS supplied one.  Otherwise AL=0, AH=scan; the F1..F10
 * range check ("cmp al,3bh; jb" unsigned, "cmp al,44h; jg" signed -- both
 * equivalent to a plain unsigned range test since scan is always < 0x80)
 * gates a call to menu_list_active()/menu_loop_run(), and the extended-key
 * encoding v=0x100|scan is returned either way. */
dos_int keyboard_read_blocking_hotkeys(void)
{
    uint8_t ascii, scan;
    dos_int v;

    input_platform_wait_key();
    fifo_pop(&ascii, &scan); /* non-empty: single-reader game thread */

    if (ascii != 0) {
        return (dos_int)ascii; /* AH cleared */
    }

    v = (dos_int)(0x100 | scan);
    if (scan >= 0x3B && scan <= 0x44 && menu_list_active()) {
        menu_loop_run((dos_int)(v - 0x13B));
    }
    return v;
}

/* ---- F_6B4A: keyboard_poll_nonblocking ----
 * "mov ah,1 / int 16h / jz L_none" tests the BIOS's ZF (no key ready)
 * without consuming; same AL/AH-to-return-value shuffle as F_6B1A
 * otherwise. */
dos_int keyboard_poll_nonblocking(void)
{
    uint8_t ascii, scan;

    if (!fifo_peek(&ascii, &scan)) {
        return 0;
    }
    if (ascii != 0) {
        return (dos_int)ascii;
    }
    return (dos_int)(0x100 | scan);
}

/* ---- F_6B66: keyboard_buffer_drain ---- */
void keyboard_buffer_drain(void)
{
    uint8_t ascii, scan;

    while (keyboard_poll_nonblocking()) {
        fifo_pop(&ascii, &scan);
    }
}
