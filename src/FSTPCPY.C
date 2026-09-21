char far *far_stpcpy_capped(char far *dst, char far *src) { register int i; i=0; while ((*dst++ = *src++) && i++ < 1000) ; return dst-1; }
