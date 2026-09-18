extern unsigned char *gbfbc;f250c(i) int i;{unsigned char a;unsigned char *p;p=gbfbc+i*3+0x2ac;if(a=*p){if(a&=15){*p=(*p&0xf0)|(6-a);if(*p&15)*p^=32;}else *p=(*p&0xf0)|6;}}
