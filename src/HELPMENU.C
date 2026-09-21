/* src/HELPMENU.C: F1 help: topic list dialog and the four topic wrappers.
   One translation unit; the sections below were the separate member
   sources of grouped module C_D3DA_D49D and keep their original ids. */

/* ---- F_D3DA (original code at 0xD3DA) ---- */
extern int dialog_list_run();
extern void resource_load_record_alloc();
extern void free();extern char *gc5ce[];struct R{char *p;char a[5];char n;};extern struct R g22f0;dialog_list_pick(a,n,p) int a,n;char far *p;{char *q;register int i,r;resource_load_record_alloc(64,&q);for(i=0;i<n;i++)gc5ce[i]=q+((int *)q+a)[i]+2;g22f0.p=p;g22f0.n=n;r=dialog_list_run(&g22f0);free(q);return r;}


/* ---- F_D45C (original code at 0xD45C) ---- */
extern int dialog_list_pick(int, int, char far *);
extern char g2302[];
int help_topic_keyboard_show(void)
{
    return dialog_list_pick(0, 2, g2302);
}


/* ---- F_D471 (original code at 0xD471) ---- */
extern int dialog_list_pick(int, int, char far *);
extern char g2315[];
int help_topic_playing_show(void)
{
    return dialog_list_pick(2, 2, g2315);
}


/* ---- F_D487 (original code at 0xD487) ---- */
extern int dialog_list_pick(int, int, char far *);
extern char g2326[];
int help_topic_obstacles_show(void)
{
    return dialog_list_pick(4, 2, g2326);
}


/* ---- F_D49D (original code at 0xD49D) ---- */
extern int dialog_list_pick(int, int, char far *);
extern char g233b[];
int help_topic_puzzles_show(void)
{
    return dialog_list_pick(6, 3, g233b);
}
