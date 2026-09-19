# Eighty-second matching-C wave

`F_6F4B` occupies load offsets `0x6F4B..0x6FC3` and decodes the packed stream
following `F_6EFF`. The complete 120-byte Turbo C component preserves its
`LDS`/`LES` transitions, `XLATB` table lookups, nibble rotations, and output
loops. It has no external fixups or MZ loader relocations.

The complete EXE and both DAT archives remain byte-identical after promotion.
