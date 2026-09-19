# Executable-data wave 131

This wave removes 122 bytes from opaque EXE fallback without classifying any
bytes as code. It promotes two zero-padding runs, two little-endian control
tables and four terminated ASCII strings from the help/menu data spans.

The strict `zero-pad-v1`, `u16le-table-v1` and `ascii-nul-v1` encoders reproduce
all eight extents byte for byte. The raw EXE frontier is now 10,051 bytes across
16 owners. The matching-C frontier remains 58,895 bytes across 345 owners, with
zero matching-ASM owners.
