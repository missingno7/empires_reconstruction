# Eighty-first matching-C wave

`F_6EFF` occupies load offsets `0x6EFF..0x6F4B` and masks eight packed words,
then expands the following nibble stream through the historical `XLATB` loops.
Its fresh Turbo C object reproduces all 76 bytes, including the far-pointer and
ES:DI state setup, with no external fixups or MZ loader relocations.

The complete EXE and both DAT archives remain byte-identical after promotion.
