# Executable-data wave 138

Wave 138 partitions the remaining `0x103CE–0x10489` suffix as 46 fixed
four-byte records followed by a three-byte zero pad. The record block is
relocation-free and ends exactly where the next raw owner begins; its strict
fixed-record and zero-pad sources round-trip every byte.

The raw EXE frontier is now 5,126 bytes across 16 owners; matching-C remains
58,895 bytes across 345 owners with zero matching-ASM owners.
