/* F_D99B -- bring the OPL chip up: clear the note table, reset, silence the
   nine voices one at a time, then enable. */
extern void fddc7();
extern void fdefa();
extern void fd9d9();
extern void fda49();
extern void fe52a();
extern void fda20();
extern void fd9e1();

void fd99b()
{
    register int i;

    fddc7();
    fdefa();
    fd9d9(0);
    fda49(0, 0, 0);
    for (i = 0; i < 9; i++)
        fe52a(i);
    fda20(1);
    fd9e1(1);
}
