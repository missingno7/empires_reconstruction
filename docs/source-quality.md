# Executable source-quality inventory

`python tools/report_source_quality.py` classifies game-owned matching-C
sources independently from byte ownership. It distinguishes syntactic
`ASM_DB_CAPSULE`, `C_WITH_SYMBOLIC_INLINE_ASM`, and `MECHANICAL_C` forms, while
retaining pinned Borland components as legitimate `HISTORICAL_LIBRARY` inputs.

The generated [receipt](source-quality.json) is an inventory, not a semantic
claim: a C source without inline assembly can still be mechanically recovered.
The report is the conversion queue for replacing complete `asm db` routines
with matching C or intentional symbolic TASM, with whole-EXE identity required
for every promotion.
