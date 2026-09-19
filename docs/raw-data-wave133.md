# Executable-data wave 133

Wave 133 removes 126 bytes from opaque EXE fallback as independently encoded
zero-padding runs, relocation-free little-endian word tables, and the
NUL-terminated `New User Message` record. The split boundaries preserve the
remaining relocation-backed and mixed-control-byte records as raw owners.

The strict `zero-pad-v1`, `u16le-table-v1` and `ascii-nul-v1` encoders prove
every promoted extent byte for byte. The raw EXE frontier is now 8,651 bytes
across 15 owners; matching-C remains 58,895 bytes across 345 owners with zero
matching-ASM owners.
