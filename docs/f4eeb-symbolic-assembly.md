# F_4EEB symbolic assembly

`F_4EEB` is a 120-byte record-table walk that selects sprite entries and calls
`F_03CC` and `F_03A5`.  Its earlier matching C wrapper used `asm db` for
register-sensitive instructions because Turbo C automatically preserved `SI`
and `DI` when it could see those registers.

The canonical form is now [F_4EEB.ASM](../src/F_4EEB.ASM).  It states the
compact-model prologue, record loop, field offsets, calls, and exit sequence as
symbolic TASM.  It deliberately keeps the original register convention rather
than presenting speculative C types.

Fresh TASM proof confirms the complete 120-byte `_TEXT` contribution and the
six original fixups: near calls to `_f01ce`, `_f03cc`, and `_f03a5`, plus
DGROUP offsets for `_gb3ae`, `_gbfba`, and `_gb07c`.  The full structural
Turbo Link build remains byte-identical, including all 106 relocation entries.
