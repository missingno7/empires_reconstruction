# Wave 109 matching-C conversion proof

F_6B4A is now a matching-C owner. Its complete 28-byte non-blocking BIOS
keyboard poll was translated from the previously verified TASM extent into
Turbo C inline assembly statements. A fresh Turbo C/TASM object has one public,
no fixups, no loader relocations, and exact full-extent equality.

The conversion changes the executable source representation without changing
any bytes or placement. Matching-C coverage is now 56,358 bytes across 323
owners; matching-ASM coverage is 2,537 bytes across 22 owners.
