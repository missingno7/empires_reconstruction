/* F_DA49 -- latch three bytes of global voice state and re-emit them. */
extern void fe420();
extern void fe262();
extern char gc6aa;                      /* DS:C6AA */
extern char gc6b5;                      /* DS:C6B5 */
extern char gc6b4;                      /* DS:C6B4 */

void fda49(a, b, c)
char a;
char b;
char c;
{
    gc6aa = a;
    gc6b5 = b;
    gc6b4 = c;
    fe420();
    fe262();
}
