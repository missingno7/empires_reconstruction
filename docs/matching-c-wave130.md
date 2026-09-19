# Matching-C wave 130

The last two ASM owners are now C-owned. F_6BCF is the complete 87-byte timer
interrupt handler; F_699E is the complete 380-byte keyboard interrupt handler.
Turbo C's `interrupt` prologue and epilogue reproduce the register saves and
IRET scaffolding. Inline assembly preserves the port I/O, interrupt control,
far chains and the keyboard handler's CS-relative dispatch table. Fresh
full-extent OMF binding proves eleven fixups and one DGROUP loader relocation
for each handler, with byte-identical extents.

Matching-C coverage is now 58,895 bytes across 345 owners. Matching-ASM
coverage is 0 bytes across 0 owners.
