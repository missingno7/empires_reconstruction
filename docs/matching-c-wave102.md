# Matching C wave 102

Ten instruction-level `RET`-bounded routines were split from four compound raw
spans and promoted as independent matching-C sources: `F_988F`, `F_98CB`,
`F_CB5C`, `F_CBBB`, `F_CC17`, `F_CC6B`, `F_CD23`, `F_DAD7`, `F_DB17`, and
`F_DB35`. Together they add 899 bytes. Each source uses an exact second OMF
label to preserve its split boundary. `F_DB17` and `F_DB35` have now been
lifted to symbolic TASM. They turn formerly opaque near calls into the real
`_fe1f2` and `_fc898` OMF external relationships while retaining their
30-byte and 43-byte extents, each with one self-relative fixup. The other
wave-102 proof units have no external fixups or loader relocations. `F_DF98`
from the adjacent span is tracked separately in wave 103 because it has
library far-call relocations.

The complete EXE and DAT archives remain equal; raw executable ownership is
now 18,397 bytes.
