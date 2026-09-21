/* input_sdl.c -- SDL3 keyboard events -> IBM PC set-1 scancode + BIOS ASCII.
 *
 * All key-tracking/FIFO semantics live in portable/game/keyboard.c; this
 * file only knows how to translate an SDL_Event into the (scancode, down,
 * ascii) triple input_key_event() expects.  Only this file (and
 * input_sdl.h) may include <SDL3/SDL.h>.
 */
#include "input_sdl.h"

#include "input.h"

/* IBM PC set-1 scancode for a subset of SDL scancodes (letters, digits,
 * F-keys, navigation cluster, keypad, modifiers, US punctuation).  Returns
 * 0 (not a valid set-1 code) for anything not covered by this phase. */
static uint8_t set1_scancode_from_sdl(SDL_Scancode sc)
{
    switch (sc) {
    /* letters */
    case SDL_SCANCODE_A: return 0x1E; case SDL_SCANCODE_B: return 0x30;
    case SDL_SCANCODE_C: return 0x2E; case SDL_SCANCODE_D: return 0x20;
    case SDL_SCANCODE_E: return 0x12; case SDL_SCANCODE_F: return 0x21;
    case SDL_SCANCODE_G: return 0x22; case SDL_SCANCODE_H: return 0x23;
    case SDL_SCANCODE_I: return 0x17; case SDL_SCANCODE_J: return 0x24;
    case SDL_SCANCODE_K: return 0x25; case SDL_SCANCODE_L: return 0x26;
    case SDL_SCANCODE_M: return 0x32; case SDL_SCANCODE_N: return 0x31;
    case SDL_SCANCODE_O: return 0x18; case SDL_SCANCODE_P: return 0x19;
    case SDL_SCANCODE_Q: return 0x10; case SDL_SCANCODE_R: return 0x13;
    case SDL_SCANCODE_S: return 0x1F; case SDL_SCANCODE_T: return 0x14;
    case SDL_SCANCODE_U: return 0x16; case SDL_SCANCODE_V: return 0x2F;
    case SDL_SCANCODE_W: return 0x11; case SDL_SCANCODE_X: return 0x2D;
    case SDL_SCANCODE_Y: return 0x15; case SDL_SCANCODE_Z: return 0x2C;

    /* digit row */
    case SDL_SCANCODE_1: return 0x02; case SDL_SCANCODE_2: return 0x03;
    case SDL_SCANCODE_3: return 0x04; case SDL_SCANCODE_4: return 0x05;
    case SDL_SCANCODE_5: return 0x06; case SDL_SCANCODE_6: return 0x07;
    case SDL_SCANCODE_7: return 0x08; case SDL_SCANCODE_8: return 0x09;
    case SDL_SCANCODE_9: return 0x0A; case SDL_SCANCODE_0: return 0x0B;

    /* F-keys */
    case SDL_SCANCODE_F1: return 0x3B; case SDL_SCANCODE_F2: return 0x3C;
    case SDL_SCANCODE_F3: return 0x3D; case SDL_SCANCODE_F4: return 0x3E;
    case SDL_SCANCODE_F5: return 0x3F; case SDL_SCANCODE_F6: return 0x40;
    case SDL_SCANCODE_F7: return 0x41; case SDL_SCANCODE_F8: return 0x42;
    case SDL_SCANCODE_F9: return 0x43; case SDL_SCANCODE_F10: return 0x44;
    case SDL_SCANCODE_F11: return 0x57; case SDL_SCANCODE_F12: return 0x58;

    case SDL_SCANCODE_ESCAPE: return 0x01;
    case SDL_SCANCODE_RETURN: return 0x1C;
    case SDL_SCANCODE_SPACE: return 0x39;
    case SDL_SCANCODE_BACKSPACE: return 0x0E;
    case SDL_SCANCODE_TAB: return 0x0F;

    /* navigation cluster */
    case SDL_SCANCODE_UP: return 0x48; case SDL_SCANCODE_DOWN: return 0x50;
    case SDL_SCANCODE_LEFT: return 0x4B; case SDL_SCANCODE_RIGHT: return 0x4D;
    case SDL_SCANCODE_HOME: return 0x47; case SDL_SCANCODE_END: return 0x4F;
    case SDL_SCANCODE_PAGEUP: return 0x49; case SDL_SCANCODE_PAGEDOWN: return 0x51;
    case SDL_SCANCODE_INSERT: return 0x52; case SDL_SCANCODE_DELETE: return 0x53;

    /* keypad: the numlock-off codes double as the navigation cluster, same
       as the real IBM set-1 table. */
    case SDL_SCANCODE_KP_8: return 0x48; case SDL_SCANCODE_KP_2: return 0x50;
    case SDL_SCANCODE_KP_4: return 0x4B; case SDL_SCANCODE_KP_6: return 0x4D;
    case SDL_SCANCODE_KP_7: return 0x47; case SDL_SCANCODE_KP_9: return 0x49;
    case SDL_SCANCODE_KP_1: return 0x4F; case SDL_SCANCODE_KP_3: return 0x51;
    case SDL_SCANCODE_KP_5: return 0x4C; case SDL_SCANCODE_KP_0: return 0x52;
    case SDL_SCANCODE_KP_PERIOD: return 0x53;
    case SDL_SCANCODE_KP_PLUS: return 0x4E; case SDL_SCANCODE_KP_MINUS: return 0x4A;
    case SDL_SCANCODE_KP_MULTIPLY: return 0x37;

    /* modifiers: L/R share one set-1 code, matching the historical
       handler's E0-prefix-discarding view of the keyboard. */
    case SDL_SCANCODE_LCTRL: case SDL_SCANCODE_RCTRL: return 0x1D;
    case SDL_SCANCODE_LSHIFT: return 0x2A; case SDL_SCANCODE_RSHIFT: return 0x36;
    case SDL_SCANCODE_LALT: case SDL_SCANCODE_RALT: return 0x38;
    case SDL_SCANCODE_CAPSLOCK: return 0x3A;
    case SDL_SCANCODE_NUMLOCKCLEAR: return 0x45;
    case SDL_SCANCODE_SCROLLLOCK: return 0x46;

    /* US punctuation */
    case SDL_SCANCODE_MINUS: return 0x0C; case SDL_SCANCODE_EQUALS: return 0x0D;
    case SDL_SCANCODE_LEFTBRACKET: return 0x1A; case SDL_SCANCODE_RIGHTBRACKET: return 0x1B;
    case SDL_SCANCODE_SEMICOLON: return 0x27; case SDL_SCANCODE_APOSTROPHE: return 0x28;
    case SDL_SCANCODE_GRAVE: return 0x29; case SDL_SCANCODE_BACKSLASH: return 0x2B;
    case SDL_SCANCODE_COMMA: return 0x33; case SDL_SCANCODE_PERIOD: return 0x34;
    case SDL_SCANCODE_SLASH: return 0x35;

    default: return 0;
    }
}

/* BIOS INT 16h AL value for a make event, US layout.  0 for keys with no
 * ASCII (F-keys, navigation cluster, modifiers, keypad with NumLock off). */
static uint8_t bios_ascii_for_make(SDL_Scancode sc, SDL_Keymod mod)
{
    bool shift = (mod & SDL_KMOD_SHIFT) != 0;
    bool caps = (mod & SDL_KMOD_CAPS) != 0;
    bool ctrl = (mod & SDL_KMOD_CTRL) != 0;
    bool numlock = (mod & SDL_KMOD_NUM) != 0;

    /* Letters: Ctrl wins over Shift/CapsLock, matching INT 16h's
       Ctrl+letter -> 1..26 (AL), ignoring case. */
    if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) {
        int idx = (int)(sc - SDL_SCANCODE_A);
        bool upper;

        if (ctrl) {
            return (uint8_t)(idx + 1);
        }
        upper = shift ^ caps;
        return (uint8_t)((upper ? 'A' : 'a') + idx);
    }

    switch (sc) {
    case SDL_SCANCODE_1: return shift ? '!' : '1';
    case SDL_SCANCODE_2: return shift ? '@' : '2';
    case SDL_SCANCODE_3: return shift ? '#' : '3';
    case SDL_SCANCODE_4: return shift ? '$' : '4';
    case SDL_SCANCODE_5: return shift ? '%' : '5';
    case SDL_SCANCODE_6: return shift ? '^' : '6';
    case SDL_SCANCODE_7: return shift ? '&' : '7';
    case SDL_SCANCODE_8: return shift ? '*' : '8';
    case SDL_SCANCODE_9: return shift ? '(' : '9';
    case SDL_SCANCODE_0: return shift ? ')' : '0';

    case SDL_SCANCODE_RETURN: return 0x0D;
    case SDL_SCANCODE_BACKSPACE: return 0x08;
    case SDL_SCANCODE_TAB: return 0x09;
    case SDL_SCANCODE_ESCAPE: return 0x1B;
    case SDL_SCANCODE_SPACE: return 0x20;

    case SDL_SCANCODE_MINUS: return shift ? '_' : '-';
    case SDL_SCANCODE_EQUALS: return shift ? '+' : '=';
    case SDL_SCANCODE_LEFTBRACKET: return shift ? '{' : '[';
    case SDL_SCANCODE_RIGHTBRACKET: return shift ? '}' : ']';
    case SDL_SCANCODE_SEMICOLON: return shift ? ':' : ';';
    case SDL_SCANCODE_APOSTROPHE: return shift ? '"' : '\'';
    case SDL_SCANCODE_GRAVE: return shift ? '~' : '`';
    case SDL_SCANCODE_BACKSLASH: return shift ? '|' : '\\';
    case SDL_SCANCODE_COMMA: return shift ? '<' : ',';
    case SDL_SCANCODE_PERIOD: return shift ? '>' : '.';
    case SDL_SCANCODE_SLASH: return shift ? '?' : '/';

    /* keypad math keys always have an ASCII, independent of NumLock. */
    case SDL_SCANCODE_KP_MULTIPLY: return (uint8_t)'*';
    case SDL_SCANCODE_KP_MINUS: return (uint8_t)'-';
    case SDL_SCANCODE_KP_PLUS: return (uint8_t)'+';

    /* keypad digit cluster: ASCII only with NumLock on, else it is the
       navigation cluster (ascii 0, set1 code shared with the arrow keys). */
    case SDL_SCANCODE_KP_0: return numlock ? (uint8_t)'0' : 0;
    case SDL_SCANCODE_KP_1: return numlock ? (uint8_t)'1' : 0;
    case SDL_SCANCODE_KP_2: return numlock ? (uint8_t)'2' : 0;
    case SDL_SCANCODE_KP_3: return numlock ? (uint8_t)'3' : 0;
    case SDL_SCANCODE_KP_4: return numlock ? (uint8_t)'4' : 0;
    case SDL_SCANCODE_KP_5: return numlock ? (uint8_t)'5' : 0;
    case SDL_SCANCODE_KP_6: return numlock ? (uint8_t)'6' : 0;
    case SDL_SCANCODE_KP_7: return numlock ? (uint8_t)'7' : 0;
    case SDL_SCANCODE_KP_8: return numlock ? (uint8_t)'8' : 0;
    case SDL_SCANCODE_KP_9: return numlock ? (uint8_t)'9' : 0;
    case SDL_SCANCODE_KP_PERIOD: return numlock ? (uint8_t)'.' : 0;

    default:
        /* F-keys, arrows/Home/End/PgUp/PgDn/Ins/Del, modifiers: no ASCII. */
        return 0;
    }
}

void input_sdl_handle_event(const SDL_Event *e)
{
    uint8_t set1;
    bool down;
    uint8_t ascii;

    if (e->type != SDL_EVENT_KEY_DOWN && e->type != SDL_EVENT_KEY_UP) {
        return;
    }

    set1 = set1_scancode_from_sdl(e->key.scancode);
    if (set1 == 0) {
        return; /* no set-1 mapping in this phase: ignore */
    }

    down = (e->type == SDL_EVENT_KEY_DOWN);
    ascii = down ? bios_ascii_for_make(e->key.scancode, e->key.mod) : 0;

    input_key_event(set1, down, ascii);
}
