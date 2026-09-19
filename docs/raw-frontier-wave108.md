# Raw executable frontier after wave 107

Wave 107 removes 700 dialog-text bytes from raw ownership. The remaining
raw fallback is 10,423 bytes across ten owners:

| Range | Bytes | Current evidence |
|---|---:|---|
| RAW_00D81B, RAW_00DA17 | 1 + 1 | Single alignment gaps between recovered code extents. |
| RAW_01034E | 1,129 | Numeric tables and DGROUP-relative pointer data; 20 MZ relocations land here. |
| RAW_0107E2 | 538 | Help/menu strings and associated control tables. |
| RAW_010FA5 | 80 | Remaining sign-in dialog control/table records. |
| RAW_010FF6, RAW_011037 | 43 + 51 | Player-name dialog control/table records. |
| RAW_01129F | 3,171 | Gameplay messages, strings, and table data. |
| RAW_011F10 | 5,354 | Repeated help text, tables, and zero-filled data. |
| RAW_0134FB | 55 | Terminal initialized table data. |

The upstream machine inventory still has no remaining complete code extent
wholly inside these ranges. The next mechanical frontier is relocation-aware
table and text decoding; matching-C coverage remains saturated at 56,330
bytes.
