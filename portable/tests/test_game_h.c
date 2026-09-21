/* test_game_h.c -- verifies portable/include/game.h compiles standalone
 * (tu-porting-rules.md sec 1: every portable/game/*.c file's first
 * non-comment line is `#include "game.h"`, so the umbrella header itself
 * must compile cleanly with no other includes).  Not a runtime test of any
 * behavior -- a successful build IS the test.
 */
#include "game.h"

int main(void)
{
    return 0;
}
