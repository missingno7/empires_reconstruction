/* dos_types.h -- integer discipline for the portable tree.
 *
 * The historical game is Turbo C 2.0 compact-model code: char is 8-bit and
 * SIGNED, int/unsigned are 16-bit, long/unsigned long are 32-bit.  Modern
 * compilers widen int to 32 bits, so ported code MUST NOT use plain int for
 * any quantity that the historical source stored as int/unsigned.  Declare
 * historical objects with the dos_* aliases below.
 *
 * The aliases only fix STORAGE width.  C integer promotion still evaluates
 * dos_int + dos_int as a 32-bit int, which is harmless when the result is
 * immediately stored back into a dos_int (the store truncates exactly like
 * the 8086 did) but NOT when the wider intermediate is compared, shifted,
 * divided, or widened to long.  Use the helpers below at the sites the
 * historical comments (or differential tests) show to be width-sensitive,
 * e.g. RESOURCE.C's `(int)o2 - (int)o1` (truncate each long first, then
 * subtract in 16 bits).  Do not sprinkle them everywhere.
 */
#ifndef PORTABLE_DOS_TYPES_H
#define PORTABLE_DOS_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef int8_t   dos_char;    /* Turbo C `char` (signed by default) */
typedef uint8_t  dos_uchar;   /* `unsigned char` */
typedef int16_t  dos_int;     /* `int` */
typedef uint16_t dos_uint;    /* `unsigned` */
typedef int32_t  dos_long;    /* `long` */
typedef uint32_t dos_ulong;   /* `unsigned long` */

/* Explicit 16-bit truncation.  `(dos_int)(x)` also works; the named form
 * documents intent at sites where the truncation is the whole point. */
static inline dos_int  dos_i16(int32_t v) { return (dos_int)(uint16_t)v; }
static inline dos_uint dos_u16(int32_t v) { return (dos_uint)v; }

/* 16-bit two's-complement arithmetic with wraparound, signed result. */
static inline dos_int dos_add16(dos_int a, dos_int b) { return dos_i16((int32_t)a + b); }
static inline dos_int dos_sub16(dos_int a, dos_int b) { return dos_i16((int32_t)a - b); }
static inline dos_int dos_mul16(dos_int a, dos_int b) { return dos_i16((int32_t)a * b); }

/* Unsigned 16-bit wraparound arithmetic. */
static inline dos_uint dos_uadd16(dos_uint a, dos_uint b) { return (dos_uint)(a + b); }
static inline dos_uint dos_usub16(dos_uint a, dos_uint b) { return (dos_uint)(a - b); }

/* Byte views of a 16-bit word the way 8086 register halves expose them. */
static inline uint8_t dos_lo8(uint16_t v) { return (uint8_t)(v & 0xff); }
static inline uint8_t dos_hi8(uint16_t v) { return (uint8_t)(v >> 8); }

/* Little-endian loads/stores for record and archive parsing.  All on-disk
 * and in-memory game structures are little-endian; never reinterpret
 * buffers through wider pointer types. */
static inline uint16_t dos_rd16(const uint8_t *p) { return (uint16_t)(p[0] | (p[1] << 8)); }
static inline uint32_t dos_rd32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
static inline void dos_wr16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
static inline void dos_wr32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

#endif /* PORTABLE_DOS_TYPES_H */
