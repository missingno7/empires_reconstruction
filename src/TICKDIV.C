/* F_1EB4 -- the tick divided by eight.  Signed: cwd/idiv, not shr. */
extern int campaign_round_node_cursor;

int tick_div8(void)
{
    return campaign_round_node_cursor / 8;
}
