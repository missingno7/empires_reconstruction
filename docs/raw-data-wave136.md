# Executable-data wave 136

Wave 136 partitions 56 bytes from `RAW_01034E` as a 28-entry little-endian
pointer table. The values are strictly increasing code targets and the extent
contains no loader relocations; the following bytes begin a separate mixed
table and remain raw.

The strict `u16le-table-v1` encoder proves the pointer table byte for byte. The
raw EXE frontier is now 5,341 bytes across 17 owners; matching-C remains 58,895
bytes across 345 owners with zero matching-ASM owners.
