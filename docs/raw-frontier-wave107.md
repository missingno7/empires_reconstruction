# Raw executable frontier after wave 106

Wave 106 removes 299 bytes from the raw fallback by decoding the Borland
runtime banner/error block and its adjacent padding and word table. The
remaining raw fallback is 11,123 bytes across nine owners:

| Range | Bytes | Current evidence |
|---|---:|---|
| RAW_00D81B, RAW_00DA17 | 1 + 1 | Single alignment gaps between recovered code extents. |
| RAW_01034E | 1,129 | Numeric tables and DGROUP-relative pointer data; 20 MZ relocations land here. |
| RAW_0107E2 | 538 | Help/menu strings and associated tables. |
| RAW_010F15 | 224 | Player/sign-in dialog strings. |
| RAW_010FF6 | 650 | Player-name/menu strings and initialized tables. |
| RAW_01129F | 3,171 | Gameplay messages, strings, and table data. |
| RAW_011F10 | 5,354 | Repeated help text, tables, and zero-filled data. |
| RAW_0134FB | 55 | Terminal initialized table data. |

The upstream machine inventory still has no remaining complete code extent
wholly inside these ranges. The next mechanical frontier is relocation-aware
table and text decoding; the executable routine frontier remains saturated at
56,330 matching-C bytes.
