extern char *gbfc0;extern unsigned char *f2a2d();f2a70(){unsigned char *p;p=gbfc0+*gbfc0*4+1;p=f2a2d(*p);p+=*p*3+1;p+=*p*12+1;return (int)(p+*p*3+1);}
