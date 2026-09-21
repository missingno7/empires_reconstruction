/* tickdiv.c -- src/TICKDIV.C: the campaign-round node cursor divided by 8.
 *
 * F_1EB4 -- signed division (historical comment: "Signed: cwd/idiv, not
 * shr.").  int-semantics-inventory.md fact 14: C99+ signed `/` already
 * rounds toward zero, matching 8086 `idiv`, so no helper is needed -- the
 * only requirement is that campaign_round_node_cursor stay signed
 * (dos_int, never dos_uint or a `>>` shift).
 */
#include "game.h"

dos_int tick_div8(void)
{
    return campaign_round_node_cursor / 8;
}
