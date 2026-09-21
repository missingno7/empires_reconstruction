/* cclib.h -- Turbo C 2.0 CC.LIB semantics the ported game code relies on.
 *
 * tu-porting-rules.md sec 4: `movmem`, `setmem`, `memcpy`, `memmove`,
 * `strlen`, `strcpy`, `strcat`, `strcmp`, `itoa`, `ultoa`, `ltoa`,
 * `sprintf`, `rand`, `srand`, `atoi` "use the portable cclib.h (Turbo C
 * semantics: rand() is the Turbo C LCG, itoa/ultoa produce the same
 * digits/radix behaviour, movmem handles overlap). Standard functions with
 * identical semantics map to <string.h>/<stdlib.h> directly."
 *
 * docs/portable/funcs-inventory.md sec 6 has the full call-site-count /
 * mapping table this header implements.
 *
 * Include order hazard this header protects against: `#define rand
 * cc_rand` etc. must never see -- and must never retroactively corrupt --
 * <stdlib.h>'s OWN `int rand(void);` declaration (different return width:
 * dos_int is int16_t, not `int`).  A C preprocessor #define only affects
 * text processed AFTER it, so this header includes <stdlib.h>/<string.h>/
 * <ctype.h> itself, BEFORE defining any macro, so the real libc
 * declarations are always established first regardless of what other
 * header pulled this one in or in what order.  After that, ported code's
 * literal `rand(...)`/`srand(...)`/`itoa(...)`/`ltoa(...)`/`ultoa(...)`
 * calls expand to the cc_* functions below; the real libc symbols stay
 * declared (and unused under their own names), which is harmless.
 */
#ifndef PORTABLE_CCLIB_H
#define PORTABLE_CCLIB_H

#include "dos_types.h"

#include <stdlib.h>   /* must precede the #define rand/srand below */
#include <string.h>   /* movmem/setmem build on memmove/memset */
#include <ctype.h>    /* toupper: identical semantics, used unmapped */

/* ---- movmem(src,dst,n) / setmem(dst,n,val) --------------------------
 * CC.LIB's argument order is NOT memmove's (movmem is src,dst; memmove is
 * dst,src) -- real functions, not macros, so the swap can't be gotten
 * backwards at a call site by accident. */
void cc_movmem(const void *src, void *dst, dos_uint n);
void cc_setmem(void *dst, dos_uint n, dos_char val);
#define movmem cc_movmem
#define setmem cc_setmem

/* ---- rand()/srand(seed): Turbo C 2.0's exact 32-bit LCG -------------
 * seed = seed * 0x015A4E35 + 1; return (seed >> 16) & 0x7fff;
 * Initial seed is 1 (i.e. as if `srand(1)` had been called), matching
 * Turbo C's C startup default -- see cclib.c and test_cclib.c for the
 * first-5-values-for-seed-1 sequence this must reproduce exactly. */
dos_int cc_rand(void);
void cc_srand(dos_uint seed);
#define rand cc_rand
#define srand cc_srand

/* ---- itoa/ltoa/ultoa(value, buf, radix): Turbo C's exact output -----
 * Lowercase digits for radix > 10; '-' prefix only for a signed negative
 * value in radix 10 (itoa/ltoa only -- ultoa's value is never negative).
 * `buf` must hold the historical worst case: itoa/ltoa in radix 2 need up
 * to 17 bytes (16 bits/33 for long, +1 sign, +1 NUL); callers already size
 * their buffers per the historical call sites. */
dos_char *cc_itoa(dos_int value, dos_char *buf, dos_int radix);
dos_char *cc_ltoa(dos_long value, dos_char *buf, dos_int radix);
dos_char *cc_ultoa(dos_ulong value, dos_char *buf, dos_int radix);
#define itoa cc_itoa
#define ltoa cc_ltoa
#define ultoa cc_ultoa

/* ---- farmalloc/farfree/farcoreleft (tu-porting-rules.md sec 4) ------
 * `farmalloc(n) -> malloc(n)`; `farfree -> free`.  farcoreleft() has no
 * mapping: its one caller (VIDEO.C's video_mode_select downgrade path) is
 * wholesale-replaced (docs/portable/funcs-inventory.md sec 2), so no
 * ported .c file ever needs it -- retired per tu-porting-rules.md sec 6,
 * intentionally NOT declared here. */
#define farmalloc(n) malloc(n)
#define farfree(p) free(p)

/* ---- identical-semantics standard functions, used UNMAPPED ----------
 * memcpy/memmove/memset/strlen/strcpy/strcat/strcmp/strncpy/sprintf/atoi/
 * abs/labs/toupper: <string.h>/<stdlib.h>/<ctype.h> above already declare
 * these with Turbo C-compatible semantics; ported code keeps calling them
 * by their standard names with no macro indirection. */

#endif /* PORTABLE_CCLIB_H */
