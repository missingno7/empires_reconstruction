/* actor_state_table[]: 32-byte board-cell records at DS:B3AF (DGROUP offset 45999).
   F_B99F names the shape struct Record {flag; rest[31]} and also reaches
   into it as a flat near byte buffer for cell indices that are not record-
   aligned; F_338A and F_B7F9 only ever touch it as that same flat byte
   buffer (F_338A's c<<5 IS the flag byte of record c, but F_B7F9's
   fixed indices are not record-aligned), so they keep byte indexing via a
   cast to the flat form. */
struct gb3af_entry { unsigned char flag; char rest[31]; };
extern struct gb3af_entry near actor_state_table[];   /* DS:B3AF */
