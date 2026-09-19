# Matching C wave 101

The 41-byte `F_7DD3` and 51-byte `F_8C04` spans are now explicit matching-C
continuations for the `F_7964` and `F_880A` loops. Disassembly proves that
each span contains the missing cleanup and `RET`; second OMF labels preserve
the exact split boundaries without changing the preceding loop owners.

Both fresh objects have no external fixups or loader relocations. Full EXE and
DAT equality pass, leaving 19,549 executable bytes raw.
