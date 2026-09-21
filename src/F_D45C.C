extern int dialog_list_pick(int, int, char far *);
extern char g2302[];
int help_topic_keyboard_show(void)
{
    return dialog_list_pick(0, 2, g2302);
}
