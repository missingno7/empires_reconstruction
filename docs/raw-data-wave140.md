# Executable-data wave 140

Wave 140 partitions two owner-start zero runs: 19 bytes before the next
message/table block and 3 bytes before the `FFFF` sentinel. Interior zero fields
remain raw until their record boundaries are established.

The strict `zero-pad-v1` encoder proves both components byte for byte. The raw
EXE frontier is now 5,093 bytes across 16 owners; matching-C remains 58,895
bytes across 345 owners with zero matching-ASM owners.
