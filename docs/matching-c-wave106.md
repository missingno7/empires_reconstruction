# Wave 106 executable data proof

The remaining RAW_00FC23 span is now partitioned into five deterministic
sources: two zero-padding runs, two terminated ASCII strings, and a 70-word
little-endian table. The sources encode all 299 original bytes exactly:

| Component | Bytes | Encoder | Classification |
|---|---:|---|---|
| DATA_00FC23_PAD | 17 | zero-pad-v1 | alignment padding |
| DATA_00FC34 | 43 | ascii-nul-v1 | Borland runtime banner |
| DATA_00FC5F | 45 | ascii-nul-v1 | Borland runtime error text |
| DATA_00FC8C_PAD | 54 | zero-pad-v1 | alignment padding |
| DATA_00FCC2 | 140 | u16le-table-v1 | runtime word table |

The promotion uses recipes/c/matching-wave106.json and the existing strict
data encoders. No code or relocation bytes are inferred from these sources.
AEPROG.EXE, AE000.DAT, and AE001.DAT remain byte-identical after the
promotion.
