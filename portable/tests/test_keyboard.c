/* test_keyboard.c -- portable/game/keyboard.c unit tests.
 *
 * Feeds input_key_event() sequences and checks the two historical surfaces
 * (level/edge state and the BIOS-style FIFO) against src/KEYBOARD.C's
 * switch.  No assets needed; always runs.
 */
#include <stdio.h>
#include <stdlib.h>

#include "input.h"

static int s_failures;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            s_failures++; \
        } \
    } while (0)

static void test_arrows_and_released(void)
{
    keyboard_irq_install();

    /* Home (0x47): key_up_held + key_up_left_held, key_up_released only on
       break. */
    input_key_event(0x47, true, 0);
    CHECK(key_up_held == 1 && key_up_left_held == 1 && key_up_released == 0);
    input_key_event(0x47, false, 0);
    CHECK(key_up_held == 0 && key_up_left_held == 0 && key_up_released == 1);

    keyboard_irq_install();

    /* PgUp (0x49): key_up_held + key_up_right_held, key_up_released on
       break. */
    input_key_event(0x49, true, 0);
    CHECK(key_up_held == 1 && key_up_right_held == 1 && key_up_released == 0);
    input_key_event(0x49, false, 0);
    CHECK(key_up_held == 0 && key_up_right_held == 0 && key_up_released == 1);

    keyboard_irq_install();

    /* Up arrow (0x48): key_up_held only, key_up_released on break. */
    input_key_event(0x48, true, 0);
    CHECK(key_up_held == 1 && key_up_released == 0);
    input_key_event(0x48, false, 0);
    CHECK(key_up_held == 0 && key_up_released == 1);

    keyboard_irq_install();

    /* Left arrow (0x4B): key_up_left_held only, key_up_released untouched. */
    input_key_event(0x4B, true, 0);
    CHECK(key_up_left_held == 1 && key_up_released == 0);
    input_key_event(0x4B, false, 0);
    CHECK(key_up_left_held == 0 && key_up_released == 0);

    keyboard_irq_install();

    /* Right arrow (0x4D): key_up_right_held only, key_up_released
       untouched. */
    input_key_event(0x4D, true, 0);
    CHECK(key_up_right_held == 1 && key_up_released == 0);
    input_key_event(0x4D, false, 0);
    CHECK(key_up_right_held == 0 && key_up_released == 0);
}

static void test_g856_aliases(void)
{
    /* b856 == 0: the alias codes do nothing (the historical
       "if(!g856) break;" skips straight past the shared case body) and,
       being handled+gate-closed, never reach the FIFO either. */
    keyboard_irq_install();
    b856 = 0;

    input_key_event(0x58, true, 0); /* alias of 0x47 */
    CHECK(key_up_held == 0 && key_up_left_held == 0);
    CHECK(keyboard_poll_nonblocking() == 0);

    input_key_event(0x29, true, 0); /* alias of 0x48 */
    CHECK(key_up_held == 0);
    CHECK(keyboard_poll_nonblocking() == 0);

    input_key_event(0x2B, true, 0); /* alias of 0x4B */
    CHECK(key_up_left_held == 0);
    CHECK(keyboard_poll_nonblocking() == 0);

    input_key_event(0x4E, true, 0); /* alias of 0x4D */
    CHECK(key_up_right_held == 0);
    CHECK(keyboard_poll_nonblocking() == 0);

    input_key_event(0x4A, true, 0); /* alias of 0x50 */
    CHECK(gb6a == 0);
    CHECK(keyboard_poll_nonblocking() == 0);

    /* b856 == 1: the aliases act exactly like their target scancode. */
    keyboard_irq_install();
    b856 = 1;

    input_key_event(0x58, true, 0);
    CHECK(key_up_held == 1 && key_up_left_held == 1);

    keyboard_irq_install();
    b856 = 1;
    input_key_event(0x29, true, 0);
    CHECK(key_up_held == 1);

    keyboard_irq_install();
    b856 = 1;
    input_key_event(0x2B, true, 0);
    CHECK(key_up_left_held == 1);

    keyboard_irq_install();
    b856 = 1;
    input_key_event(0x4E, true, 0);
    CHECK(key_up_right_held == 1);

    keyboard_irq_install();
    b856 = 1;
    input_key_event(0x4A, true, 0);
    CHECK(gb6a == 1);

    b856 = 0;
}

static void test_chain_gate(void)
{
    keyboard_irq_install();

    /* 0x48 (Up arrow) is a "handled" scancode: with the chain gate closed
       it never reaches the FIFO even on a make event. */
    keyboard_chain_disable();
    input_key_event(0x48, true, 0);
    CHECK(keyboard_poll_nonblocking() == 0);

    /* With the gate open, the same handled key does reach the FIFO. */
    keyboard_chain_enable();
    input_key_event(0x48, true, 0);
    CHECK(keyboard_poll_nonblocking() == (dos_int)(0x100 | 0x48));
    keyboard_buffer_drain();
    CHECK(keyboard_poll_nonblocking() == 0);

    keyboard_chain_disable();
}

static void test_unhandled_reaches_fifo(void)
{
    keyboard_irq_install();

    /* 'a' (0x1E) is not in the switch at all -> default -> handled=0 ->
       reaches the FIFO regardless of the chain gate. */
    CHECK(keyboard_chain_active() == 0);
    input_key_event(0x1E, true, (uint8_t)'a');
    CHECK(keyboard_poll_nonblocking() == (dos_int)'a');
    keyboard_buffer_drain();
    CHECK(keyboard_poll_nonblocking() == 0);
}

static void test_ctrl_tracking(void)
{
    keyboard_irq_install();

    input_key_event(0x1D, true, 0);
    CHECK(keyboard_state[1] == 1);
    input_key_event(0x1D, false, 0);
    CHECK(keyboard_state[1] == 0);

    /* 0x1D falls through to default (unhandled), but the ROM BIOS never
       buffers shift-state keys, so the FIFO must stay empty. */
    CHECK(keyboard_poll_nonblocking() == 0);
}

static void test_f1_hotkey_encoding(void)
{
    keyboard_irq_install();

    /* F1 (0x3B) has no ASCII, so it takes the "extended key" FIFO path:
       ascii=0, scancode=0x3B. */
    input_key_event(0x3B, true, 0);

    /* Pre-filled: keyboard_read_blocking_hotkeys() must not block. */
    CHECK(keyboard_read_blocking_hotkeys() == 0x13B);
}

static void test_poll_no_consume_and_drain(void)
{
    keyboard_irq_install();

    input_key_event(0x1E, true, (uint8_t)'a'); /* 'a' */

    CHECK(keyboard_poll_nonblocking() == (dos_int)'a');
    CHECK(keyboard_poll_nonblocking() == (dos_int)'a'); /* still there */

    keyboard_buffer_drain();
    CHECK(keyboard_poll_nonblocking() == 0);
}

static void test_fifo_capacity(void)
{
    int i;

    keyboard_irq_install();

    /* Push 20 make events (capacity is 15); the extra 5 must be dropped,
       keeping the oldest 15 (a full ring drops the incoming key, like the
       BIOS beep-and-drop). */
    for (i = 0; i < 20; ++i) {
        input_key_event(0x1E, true, (uint8_t)('A' + i));
    }

    for (i = 0; i < 15; ++i) {
        dos_int v = keyboard_poll_nonblocking(); /* peek, doesn't consume */
        CHECK(v == (dos_int)('A' + i));
        keyboard_read_blocking_hotkeys(); /* pop one (data present: no block) */
    }

    CHECK(keyboard_poll_nonblocking() == 0);
}

int main(void)
{
    test_arrows_and_released();
    test_g856_aliases();
    test_chain_gate();
    test_unhandled_reaches_fifo();
    test_ctrl_tracking();
    test_f1_hotkey_encoding();
    test_poll_no_consume_and_drain();
    test_fifo_capacity();

    if (s_failures != 0) {
        fprintf(stderr, "%d check(s) failed\n", s_failures);
        return 1;
    }
    printf("test_keyboard: OK\n");
    return 0;
}
