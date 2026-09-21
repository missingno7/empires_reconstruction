/* ui_panel_glyph_records[]: 2-byte records at DS:2FD2.  F_DA66 reads both bytes as glyph
   indices; F_DD72 reads both as tone-table indices (voice_set_field arguments);
   M_DAD7_DB35 reads byte 1 as a state/selector index (voice_level_table/voice_byte_table lookup). */
struct g2fd2_entry {
    char b0;   /* offset 0: F_DA66 .a / F_DD72 .a */
    char b1;   /* offset 1: F_DA66 .b / F_DD72 .b / M_DAD7_DB35 .selector */
};

extern struct g2fd2_entry ui_panel_glyph_records[];       /* DS:2FD2 */
