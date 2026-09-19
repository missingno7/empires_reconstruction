# Executable-data wave 137

Wave 137 partitions the 28 one-byte attributes paired with wave 136's 28
pointer targets. The fixed-record source ends immediately before the next
two-byte/control record stream, preserving that mixed table as raw.

The strict `fixed-records-v1` encoder proves the attribute table byte for byte.
The raw EXE frontier is now 5,313 bytes across 18 owners; matching-C remains
58,895 bytes across 345 owners with zero matching-ASM owners.
