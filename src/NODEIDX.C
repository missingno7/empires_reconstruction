extern int campaign_round_node_cursor;
int campaign_node_index(void) { return (campaign_round_node_cursor & 7) / 2; }
