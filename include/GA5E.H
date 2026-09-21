/* ga5e[]: 35-byte directory records at DS:0A5E (DGROUP offset 2654,
   storage_objects/ARRAY_FAEB.phys + 0x9A3).  F_6266 and F_656C only ever
   take the address of a whole record (never dereference a field); F_643A
   reaches into the same blob as a flat byte buffer (message at
   DGROUP:0AC7 is ga5e+0x69), so it keeps its arithmetic in bytes via a
   cast rather than indexing by record. */
struct ga5e_entry { char b[0x23]; };
extern struct ga5e_entry ga5e[];        /* DS:0A5E */
