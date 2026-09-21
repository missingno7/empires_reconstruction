extern int dialog_list_pick(int, int, char far *);
extern char g233b[];
int help_topic_puzzles_show(void)
{
    return dialog_list_pick(6, 3, g233b);
}
