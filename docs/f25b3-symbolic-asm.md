# F_25B3 symbolic assembly recovery

`F_25B3` is a 761-byte, no-fixup game routine that updates ten adjacent
three-byte records and redraws the affected cells. Its four visible paths
apply horizontal/vertical changes selected from the record's high control bits,
then restore the record or update the backing grid around the drawn cell.

The canonical [`asm/F_25B3.ASM`](../asm/F_25B3.ASM) is assembled by pinned TASM
1.0. It has public `_f_25b3`, a 761-byte `_TEXT` contribution, no OMF fixups,
and exact bytes. The call destinations were members of its original object
contribution, so the assembly keeps them as direct same-segment relations with
`CALL_REL`; this preserves their original non-EXTDEF OMF form while documenting
the known targets (`F_257D` and runtime drawing helpers).

The former matching-C `asm db` proof is retired from the canonical source tree.
`tests/test_f25b3_symbolic_asm.py` assembles the new module, binds it through
the normal ownership machinery, and requires exact bytes and no loader
relocations.
