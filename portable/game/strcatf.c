/* strcatf.c -- portable port of src/STRCATF.C.
 *
 * F_A036 -- concatenate a null-terminated list of far strings into dest.
 * Pre-ANSI variadic style in the historical source; ported to real C
 * variadic syntax (<stdarg.h>) since the argument-count/type discipline
 * (each argument is a `char far *`, terminated by a null pointer) maps
 * directly onto va_list/va_arg.  Kept variadic per the porting brief (the
 * SLOTMENU.C declaration conflict -- a fixed 5-parameter prototype -- is
 * the historical file's own inconsistency; the DEFINITION here is the one
 * that wins, per tu-porting-rules.md and docs/portable/funcs-inventory.md
 * sec 5).
 */
#include "game.h"

#include <stdarg.h>

/* F_A036 */
dos_long str_concat_far_list(dos_char *dest, dos_char *first, ...)
{
    dos_char *q = dest;
    dos_char *piece = first;
    va_list ap;
    dos_int i = 0;

    va_start(ap, first);
    while (piece && i++ < 2000) {
        q = far_stpcpy_capped(q, piece);
        piece = va_arg(ap, dos_char *);
    }
    va_end(ap);
    return (dos_long)(q - dest);
}
