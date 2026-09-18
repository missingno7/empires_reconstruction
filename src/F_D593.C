/* F_D593 -- start one sound: reset the module at F_CB48, count the request
   at DS:237C, wait one tick and hand off to F_C877. */
extern void fcb48();
extern void f6c26();
extern void fc877();
extern int g237c;                       /* DS:237C */

void fd593()
{
    fcb48();
    g237c++;
    f6c26(1);
    fc877();
}
