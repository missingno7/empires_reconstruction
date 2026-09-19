# Symbolic-assembly wave 114

F_4E9F is now a symbolic-assembly owner. Its complete 76-byte record-table
walk assembles with TASM and binds all four data references plus the F_4AA8
near-call exactly. This replaces the earlier Turbo C proof wrapper, whose
byte-coded DI instructions obscured the actual control flow. The fresh object
has no DOS loader relocations.

The earlier Turbo C source remains useful evidence that the routine can be
expressed in C, but it was not readable final source: its DI-sensitive body was
an `asm db` capsule. The symbolic TASM implementation names every branch,
record offset, table reference and call while preserving the same object-level
contract.

Evidence: [original matching-C proof](matching-wave114-evidence.json) and
[fresh symbolic-assembly proof](matching-wave40-asm-evidence.json).
