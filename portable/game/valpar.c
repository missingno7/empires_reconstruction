/* valpar.c -- src/VALPAR.C: parity of an argument. */
#include "game.h"

/* F_1EC0 -- parity of the argument. */
dos_int value_parity(dos_int n)
{
    return n & 1;
}
