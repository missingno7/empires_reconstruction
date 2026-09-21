/* F_A036 -- concatenate a null-terminated list of far strings into dest.
   Pre-ANSI variadic style: the argument list is walked from &first until a
   null pointer; each piece is appended with fa004. Returns the total length. */
extern char far *fa004(char far *dst, char far *src);
long str_concat_far_list(char far *dest, char far *first, ...)
{
    register int i;
    char far * far *t;
    char far *q;
    i = 0;
    t = &first;
    q = dest;
    while (*t && i++ < 2000)
        q = fa004(q, *t++);
    return q - dest;
}
