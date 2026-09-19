# Seventy-sixth structured-data wave

The 47-byte range at file offsets `69329..69376` is now partitioned into three
independently encoded components:

| Source | File range | Length | Proof |
|---|---:|---:|---|
| `DATA_10ED1` | `0x10ED1..0x10EDD` | 12 | six LE16 values equal recovered code addresses `39025`, `39055`, `39115`, `39176`, `39330`, and `39266` |
| `PAD_10EDD` | `0x10EDD..0x10EE0` | 3 | zero-filled alignment to the next 16-byte boundary |
| `DATA_10EE0` | `0x10EE0..0x10F00` | 32 | two 16-byte permutations, each containing `0..15` exactly once |

The sources use the existing strict `u16le-table-v1`, `zero-pad-v1`, and
`fixed-records-v1` encoders. Each output matches its complete original extent;
the full EXE and both DAT archives remain byte-identical. The only remaining
raw EXE owner is the 55-byte terminal data tail at `0x134FB`.
