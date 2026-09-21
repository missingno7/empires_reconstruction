extern int dialog_list_run();
extern void resource_load_record_alloc();
extern void free();extern char *gc5ce[];struct R{char *p;char a[5];char n;};extern struct R g22f0;fd3da(a,n,p) int a,n;char far *p;{char *q;register int i,r;resource_load_record_alloc(64,&q);for(i=0;i<n;i++)gc5ce[i]=q+((int *)q+a)[i]+2;g22f0.p=p;g22f0.n=n;r=dialog_list_run(&g22f0);free(q);return r;}