# Executable-data wave 141

Wave 141 partitions the six zero bytes between the embedded palette pair and
the recovered control pointer table. The boundary is aligned with the end of
the palette owner and the start of the table; the remaining table fields stay
raw.

The strict `zero-pad-v1` encoder proves the alignment component byte for byte.
The raw EXE frontier is now 5,087 bytes across 16 owners; matching-C remains
58,895 bytes across 345 owners with zero matching-ASM owners.
