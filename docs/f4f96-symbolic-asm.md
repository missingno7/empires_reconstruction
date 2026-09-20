# F_4F96 symbolic option parser

`F_4F96` is a 299-byte game-owned command-line option parser. Its original
byte capsule has been replaced with [`asm/F_4F96.ASM`](../asm/F_4F96.ASM), a
complete symbolic TASM representation.

The routine scans the far argument-vector at `DS:006D`, accepts `-` and `/`
prefixes, and dispatches the following one-character option sets:

- `CEIMSTV` (case-insensitive) select one of five mode bytes at `DS:BFCD` or
  clear the word at `DS:1778`.
- `AIT` (case-insensitive) selects the secondary word state at `DS:1778`.

The original implementation keeps its keys and handler destinations in two
inline code-segment tables. The destinations are historical linked code
offsets (`5034h`, `502Ch`, and related values), so the recovered object has no
OMF fixups, exactly as the original contribution does. They remain explicit
in the source because converting them to TASM label relocations would change
the object topology. Recovering a module boundary that makes those tables
natural relocatable source is a separate structural-link task.

Fresh TASM 1.0 assembly has one `_f4f96` public, no OMF fixups or loader
relocations, and its complete `_TEXT` contribution equals the original 299
bytes. The focused regression is
[`tests/test_f4f96_symbolic_asm.py`](../tests/test_f4f96_symbolic_asm.py).
