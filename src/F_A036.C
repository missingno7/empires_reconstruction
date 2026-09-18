extern char *fa004();long fa036(p,a) char *p,*a;{register int i;char **t;char *q;i=0;t=&a;q=p;while(*t && i++<2000)q=fa004(q,*t++);return q-p;}
