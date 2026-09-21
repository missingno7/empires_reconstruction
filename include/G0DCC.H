/* g0dcc[40]: 24-byte records at DS:0DCC ("40 records of 24 bytes at
   DS:0DCC", layout/production-plan.json).  F_9A0E walks each record as 11
   (index, selector) byte pairs; F_9B68 tests byte 22 as a bitmask; F_9DCC
   reads byte 23 as a signed frame offset. */
struct g0dcc_entry {
    struct {
        unsigned char idx;   /* F_9A0E: far-pointer-table index (was e[j].a) */
        char sel;            /* F_9A0E: signed handler selector (was e[j].b) */
    } e[11];                 /* offsets 0..21 */
    char f16;                /* offset 22: F_9B68 bit-tested byte (signed, matches original codegen) */
    char f17;                /* offset 23: F_9DCC signed frame offset (P2.b at index 11) */
};

extern struct g0dcc_entry g0dcc[];
