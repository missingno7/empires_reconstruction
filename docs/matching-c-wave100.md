# Matching C wave 100

`F_4B0C` is now a matching-C owner for its complete 915-byte non-returning
interpreter loop. The source emits the exact loop and publishes a second OMF
label at the verified extent boundary, so no compiler-generated return bytes
are included. It has no external fixups or loader relocations. Full EXE and DAT
equality pass; raw executable ownership is now 19,641 bytes.
