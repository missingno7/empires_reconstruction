/* F_E52A -- silence one voice: zero its two OPL key/level registers. */
extern void fc898();

void fe52a(v)
int v;
{
    fc898(v + 0xb0, 0);
    fc898(v + 0xa0, 0);
}
