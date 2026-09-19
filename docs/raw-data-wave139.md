# Executable-data wave 139

Wave 139 partitions two isolated zero-initialized components: the 7-byte prefix
before the next four-byte table and the 4-byte suffix after the disk-prompt
records. Relocation-sensitive table bytes between them remain raw.

The strict `zero-pad-v1` encoder proves both components byte for byte. The raw
EXE frontier is now 5,115 bytes across 18 owners; matching-C remains 58,895
bytes across 345 owners with zero matching-ASM owners.
