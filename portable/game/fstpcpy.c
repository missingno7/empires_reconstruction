/* fstpcpy.c -- src/FSTPCPY.C: strcpy that returns a pointer to the copied
 * string's terminating NUL, capped at 1000 characters (a defensive limit,
 * not a historical buffer size).
 */
#include "game.h"

dos_char *far_stpcpy_capped(dos_char *dst, dos_char *src)
{
    dos_int i;
    i = 0;
    while ((*dst++ = *src++) && i++ < 1000)
        ;
    return dst - 1;
}
