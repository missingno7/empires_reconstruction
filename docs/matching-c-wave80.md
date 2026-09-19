# Eightieth matching-C wave

`F_D825` occupies load offsets `0xD825..0xD85F` and appends one compact record
to the `DS:2F30` table. The complete 58-byte Turbo C component preserves the
historical counter increment, three-byte record layout, two packed word writes,
and returned end pointer. It has no external fixups or MZ loader relocations.

The complete EXE and both DAT archives remain byte-identical after promotion.
