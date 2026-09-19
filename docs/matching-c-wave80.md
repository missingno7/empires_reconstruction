# Eightieth matching-C wave

`F_D825` occupies load offsets `0xD825..0xD85F` and appends one compact record
to the `DS:2F30` table. Its complete 58-byte symbolic TASM source preserves
the historical counter increment, five-byte record layout, two packed word
writes, returned end pointer, and final `ret`. It has no external fixups or
MZ loader relocations.

The complete EXE and both DAT archives remain byte-identical after promotion.
