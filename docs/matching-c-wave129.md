# Matching-C wave 129

F_6DCC is now a matching-C owner. Its complete 307-byte CS-state LZ decoder
compiles from a deterministic Turbo C source unit. Inline assembly preserves the
four decoder state cells in `_TEXT`, segment-register copies and the BP
dictionary register, which the historical routine uses outside ordinary C
calling conventions. The fresh extent has no fixups or loader relocations and
matches the prior ASM owner byte for byte.

Matching-C coverage is now 58,428 bytes across 343 owners. Matching-ASM
coverage is 467 bytes across 2 owners.
