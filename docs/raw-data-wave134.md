# Executable-data wave 134

Wave 134 removes 99 bytes from opaque EXE fallback as two isolated alignment
bytes, one zero-initialized run, and four NUL-terminated ASCII records. The
records include the two option-page labels, the artifact-recovery message and
the music label; relocation-backed and mixed-control-byte spans remain raw.

The strict `zero-pad-v1` and `ascii-nul-v1` encoders prove every promoted extent
byte for byte. The raw EXE frontier is now 8,552 bytes across 15 owners;
matching-C remains 58,895 bytes across 345 owners with zero matching-ASM owners.
