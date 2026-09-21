/* voice_param_record[]: 14-byte per-voice OPL records at DS:C91B.  F_E095 indexes the
   whole record by a runtime member index; F_E0C0/F_E27B/F_E2D6/F_E324/F_E44B
   name the members c0..c13, F_E1F2/F_E372 name them f0..fd -- all char,
   same 14 offsets, so this header keeps one flat indexable array and every
   file addresses it by its own constant/variable index. */
struct gc91b_entry {
    char f[14];         /* offsets 0..13, per-voice OPL record bytes */
};

extern struct gc91b_entry voice_param_record[];      /* DS:C91B */
