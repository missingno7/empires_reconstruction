extern int dialog_list_pick(int, int, char far *);
extern char g2326[];
int help_topic_obstacles_show(void)
{
    return dialog_list_pick(4, 2, g2326);
}
