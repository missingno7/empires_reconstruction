/* game_flow.h -- the game's re-entry protocol (Phase 11).
 *
 * Historically game_run() armed `game_abort_jmpbuf` with setjmp() and three
 * call sites unwound to it with longjmp(): the value selects which re-entry
 * path game_run() takes.  The port keeps the mechanism (a host jmp_buf on
 * the game thread) but names the codes and funnels every unwind through
 * game_abort(), so the protocol is explicit and can later be replaced by
 * structured returns without touching the call sites again.
 */
#ifndef PORTABLE_GAME_FLOW_H
#define PORTABLE_GAME_FLOW_H

enum GameRunResult {
    GAME_FRESH        = 0,   /* setjmp's direct return: first entry */
    GAME_RESTART      = 1,   /* LEVEL.C / PLAYERSL.C: back to the slot menu (new game paths) */
    GAME_RETURN_MAP   = 2,   /* BOARD.C / PLAYERSL.C: resume with the current slot at the map */
    GAME_EXIT         = 3    /* PLAYERSL.C: leave game_run() entirely */
};

/* Unwind the game thread to game_run()'s re-entry point.  Never returns. */
void game_abort(enum GameRunResult how);

#endif
