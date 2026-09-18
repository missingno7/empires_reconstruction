# Fifty-eighth matching-C audit

This audit records the next raw-code candidates after wave57. No ownership is
promoted by this pass.

`F_D386` is an 84-byte parser-shaped extent, but its exact instruction stream
depends on machine state that ordinary Turbo C does not declare: `CLD`,
`LDS SI,[DS:BFC4]`, `LES SI,[BX+72B2]`, `LODSB`/`LODSW`, `LOOP`, and the live
`ES` segment used by the table lookup. It also calls the raw load-offset `03C9`
entry and contains a second entry-shaped tail after the first `RET`. A C claim
would therefore require both the segment/register calling convention and a
proved internal boundary before a complete extent could be asserted.

A fresh structured C probe using far-pointer records, the observed table base,
and the owned `F_03C9` call emitted 191 bytes for the 84-byte extent. That
measured mismatch is retained as negative evidence; the probe is not a source
component and no opaque slice is hidden inside a C owner.

The `F_CA03`/`F_CA35`/`F_CA51`/`F_CA83`/`F_CA91`/`F_CA9B` cluster has standard
stack frames in places, but its inputs are live `AL`, `BL`, `CH`, or `ES:DI`
state rather than declared stack arguments. `F_C232` has the same `ES:DI` and
`LOOP` dependency. The direct BIOS/port candidates (`F_034F`, `F_C8D4`,
`F_6B7A`, `F_6BAC`) remain hardware-facing sequences. These observations are
concrete ABI or boundary blockers; no opaque bytes are hidden inside a C owner.

The next useful recovery step for this set is matching ASM or recovered module
calling convention evidence. The matching-C frontier remains open, while all
previously proven C owners continue to rebuild the exact EXE and DAT files.
