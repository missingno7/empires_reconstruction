/* nodeidx.c -- src/NODEIDX.C: campaign round node cursor's index within its
 * pair (low 3 bits, halved).
 */
#include "game.h"

dos_int campaign_node_index(void)
{
    return (campaign_round_node_cursor & 7) / 2;
}
