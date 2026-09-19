# Raw executable frontier audit

After wave 105, the remaining 11,422 raw bytes are partitioned into the
following mechanically observed ranges:

| Range | Bytes | Current evidence |
|---|---:|---|
| `RAW_00D81B`, `RAW_00DA17` | 1 + 1 | Single zero-byte gaps between recovered code extents; alignment candidates. |
| `RAW_00FC23` | 299 | Borland runtime banner and DOS error strings. |
| `RAW_01034E` | 1,129 | Numeric tables and DGROUP-relative pointer data; 20 MZ relocations land in this data area. |
| `RAW_0107E2` | 538 | Help/menu strings and associated tables. |
| `RAW_010F15` | 224 | Player/sign-in dialog strings. |
| `RAW_010FF6` | 650 | Player-name/menu strings and initialized tables. |
| `RAW_01129F` | 3,171 | Gameplay messages, strings, and table data. |
| `RAW_011F10` | 5,354 | Repeated help text, tables, and zero-filled data. |
| `RAW_0134FB` | 55 | Terminal initialized table data. |

The upstream machine inventory has no remaining complete code extent wholly
inside these spans. The executable routine frontier is therefore currently
exhausted at 56,330 matching-C bytes; further mechanical progress should
decode these data/text ranges and their relocation-backed tables rather than
claiming them as opaque C code. The exact EXE and both DAT archives remain
verified after every prior promotion.
