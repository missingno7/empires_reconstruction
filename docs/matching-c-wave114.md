# Matching-C wave 114

F_4E9F is now a matching-C owner. Its complete 76-byte record-table walk
compiles with Turbo C and binds all four data references plus the F_4AA8
near-call exactly. DI-addressing instructions are emitted as deterministic
inline bytes so the compiler does not add a register-save pair; the fresh
object has no DOS loader relocations.

Matching-C coverage is now 56,615 bytes across 328 owners. Matching-ASM
coverage is 2,280 bytes across 17 owners.
