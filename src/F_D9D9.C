/* F_D9D9 -- a one-call wrapper around fe420.  The frame is not optional: the
   original pushes BP, and TC 2.0 emits a frame only when the function has a
   parameter or a local, so this one has a parameter the body ignores. */
extern void fe420();

void fd9d9(a)
int a;
{
    fe420();
}
