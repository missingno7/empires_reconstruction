/* input.h -- keyboard service (src/KEYBOARD.C + src/KEYIRQ.C) over SDL events.
 *
 * Two historical surfaces are preserved:
 *  1. Level/edge state written by the INT 9 handler (keyboard_irq_handler):
 *     key_up_held, key_up_left_held, key_up_right_held, key_up_released, gb6a,
 *     keyboard_state[1] (Ctrl held), with the g856-gated scancode aliases.
 *  2. The BIOS INT 16h keystroke FIFO of (ascii, scancode) pairs read by
 *     keyboard_read_blocking_hotkeys / keyboard_poll_nonblocking /
 *     keyboard_buffer_drain.  A key the handler "handles" is swallowed (never
 *     reaches the FIFO) unless keyboard_state[0] (the chain gate) is set.
 *
 * The platform layer translates SDL key events into IBM PC set-1 scancodes
 * plus the BIOS ASCII value and calls input_key_event(); everything else is
 * game semantics living in portable/game/keyboard.c.
 */
#ifndef PORTABLE_INPUT_H
#define PORTABLE_INPUT_H

#include "dos_types.h"

/* Historical DGROUP state (defined in portable/game/keyboard.c). */
extern dos_int key_up_held;          /* DS:0B68 */
extern dos_int gb6a;                 /* DS:0B6A: down-key state (scancode 0x50 / 0x4A alias) */
extern dos_int key_up_left_held;     /* DS:0B6C */
extern dos_int key_up_right_held;    /* DS:0B6E */
extern dos_int key_up_released;      /* DS:0B70 */
extern dos_int keyboard_state[2];    /* DS:0B72: [0] chain gate, [1] Ctrl held (gb74) */
extern dos_char b856;                /* DS:0856 (g856): enables the numeric-keypad aliases */
#define gb74 keyboard_state[1]
#define g856 b856

/* src/KEYBOARD.C / KEYIRQ.C entry points, semantics unchanged. */
void    keyboard_chain_enable(void);
void    keyboard_chain_disable(void);
dos_int keyboard_chain_active(void);
void    keyboard_irq_install(void);      /* starts accepting platform events */
void    keyboard_irq_restore(void);
dos_int keyboard_read_blocking_hotkeys(void);  /* blocks; ASCII, or 0x100|scan for extended keys; F1..F10 dispatch when menu_list_active() */
dos_int keyboard_poll_nonblocking(void);       /* 0 if empty, else the same encoding without consuming */
void    keyboard_buffer_drain(void);

/* ---- fed by the platform (main thread) ---- */
/* One make/break event.  scancode = IBM set-1 code (0x01..0x58, no E0/E1
 * prefix: extended keys are delivered as their base code, e.g. arrows as
 * 0x48/0x4B/0x4D/0x50 just like the historical E0-prefix-discarding handler
 * saw them).  ascii = the BIOS INT 16h AL value for a make event (0 for
 * keys with no ASCII, e.g. F-keys/arrows), ignored for break events.
 * Repeats are delivered as additional make events. */
void input_key_event(uint8_t scancode, bool down, uint8_t ascii);

/* Block the game thread until the FIFO is non-empty (platform hook; tests
 * may make it return immediately). */
void input_platform_wait_key(void);

#endif
