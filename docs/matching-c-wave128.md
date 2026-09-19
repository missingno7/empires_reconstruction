# Matching-C wave 128

F_60A9 is now a matching-C owner. Its complete 216-byte animated-tile ticker
compiles from a deterministic Turbo C source unit using inline assembly for the
original BP/SI/DI register choreography, LDS/LES table walks and stack argument
rewrite. Fresh binding preserves six data fixups, four near calls and the one
DGROUP loader relocation, and the full extent matches the prior ASM owner byte
for byte.

Matching-C coverage is now 58,121 bytes across 342 owners. Matching-ASM
coverage is 774 bytes across 3 owners.
