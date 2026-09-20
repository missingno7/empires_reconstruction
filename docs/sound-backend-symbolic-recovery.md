# M_C77A_C898 symbolic sound-backend module

`M_C77A_C898` replaces the five-owner compatible Turbo C contribution
`C_C77A_C898` in the canonical structural link. The 346-byte TASM module
contains:

- `F_C77A`, which selects and initializes a sound backend;
- `F_C7CB`, which reloads each voice from the active backend table;
- `F_C834`, which resets voices while preserving their count;
- `F_C877`, which pumps disabled voice updates; and
- `F_C898`, which writes an OPL register with the historical settling reads.

Fresh TASM emits the original public offsets 0, 81, 186, 253, and 286. Its
thirteen `_TEXT` FIXUPP records agree in site, target, addend, and relative
kind with the former shared Turbo C object. The module's non-fixup bytes match
the original bound extent, and `python tools/build_exe.py verify` retains the
byte-identical EXE, all 106 MZ relocations, and their order.

This is compatible reconstructed-module evidence from the contiguous sound
code and shared `0x1762..0x1830` state. It does not identify the historical
source filename.
