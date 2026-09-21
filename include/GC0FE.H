/* gc0fe: a far pointer at DS:0FE (DGROUP offset 49406,
   storage_objects/M_2BC2E.phys) that different subsystems repurpose.
   F_7932 only ever stores or copies the pointer value (as char far *), so
   this header declares it that way; F_7964/F_7D91 dereference it as the
   20-byte record catalog below, cast at the point of use.

   F_7BFC also dereferences gc0fe, but as an unrelated {int n; struct R *p}
   shape whose fields do not agree byte-for-byte with the catalog record
   below (offset 0 is a struct-R text pointer there vs. two header ints
   here) -- a genuine conflict, not unified; F_7BFC keeps its own local
   struct declarations. */
extern char far *gc0fe;

/* The 20-byte record entry F_7964 walks via gc0fe->records[] and F_7D91
   turns into a dialog.  F_7964 names it struct record, F_7D91 struct A;
   fields agree at every offset once F_7D91's untouched e/f pair is read as
   F_7964's opaque callbacks pointer. */
struct gc0fe_record {
    int h0, h1, h2;              /* offset 0..5: opaque header ints (F_7964 .head / F_7D91 .a,.b,.c) */
    int count;                   /* offset 6: row count (F_7964 .count / F_7D91 .d, dialog lines) */
    char far *text;              /* offset 8: label text (F_7964 .middle / F_7D91 .p, dialog text) */
    int (** far callbacks)();    /* offset 12: row handlers (F_7964 .callbacks; unused by F_7D91) */
    int width;                   /* offset 16: box width (F_7964 .width / F_7D91 .g, dialog w) */
    int x;                       /* offset 18: box x (F_7964 .x / F_7D91 .h, dialog cx) */
};

/* The catalog gc0fe points at when used this way: a record count and a far
   pointer to the record array (F_7964's struct catalog). */
struct gc0fe_catalog {
    int count;                         /* offset 0 */
    struct gc0fe_record far *records;  /* offset 2 */
};
