/* F_8A37 -- clear the 4 x 6 slot grid.  Two statements, so two full address
   computations; i is si and j is di (SI is allocated before DI). */
struct slot {
    char a;
    char b;
};

extern struct slot gc316[4][6];
extern int gc130;
extern char gc132, gc133, gc134, gc135, gc346;

void f8a37(void)
{
    register int i, j;

    gc130 = 0;
    gc134 = 0;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 6; j++) {
            gc316[i][j].a = 0xff;
            gc316[i][j].b = 0;
        }
    gc132 = 0xff;
    gc133 = 0;
    gc346 = 0;
    gc135 = 0;
}
