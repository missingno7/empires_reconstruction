# Executable-data wave 135

Wave 135 removes a 3,155-byte all-zero region from the remaining mixed data
owner. It sits between the post-message fields and the next nonzero table, has
no loader relocations, and is represented as a deterministic zero-initialized
source component. Interleaved zero fields in the surrounding tables remain raw
until their record boundaries are established.

The strict `zero-pad-v1` encoder proves the complete region byte for byte. The
raw EXE frontier is now 5,397 bytes across 16 owners; matching-C remains 58,895
bytes across 345 owners with zero matching-ASM owners.
