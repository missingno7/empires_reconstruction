# Matching C wave 103

`F_DF98` adds its complete 253-byte far-call/state-update routine as matching
C. The source preserves the exact body while emitting verified `LXMUL@` and
`LDIV@` library far-call fixups. The proof records two OMF fixups and the two
required MZ loader relocations at load offsets 57266 and 57344.

The full EXE and both DAT archives remain byte-identical; raw executable
ownership is now 19,296 bytes.
