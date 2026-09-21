extern int dialog_list_pick(int, int, char far *);
extern char g2315[];
int help_topic_playing_show(void)
{
    return dialog_list_pick(2, 2, g2315);
}
