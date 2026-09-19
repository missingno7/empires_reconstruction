# Matching C wave 102

Ten instruction-level `RET`-bounded routines were split from four compound raw
spans and promoted as independent matching-C sources: `F_988F`, `F_98CB`,
`F_CB5C`, `F_CBBB`, `F_CC17`, `F_CC6B`, `F_CD23`, `F_DAD7`, `F_DB17`, and
`F_DB35`. Together they add 899 bytes. Each source uses an exact second OMF
label to preserve its split boundary. `F_DB17` has now been lifted to
symbolic TASM: it turns its formerly opaque near call into the real
`_fe1f2` OMF external relationship while retaining the 30-byte extent and
its one self-relative fixup. The other wave-102 proof units have no external
fixups or loader relocations. `F_DF98` from the adjacent span is tracked
separately in wave 103 because it has library far-call relocations.

The complete EXE and DAT archives remain equal; raw executable ownership is
now 18,397 bytes.
