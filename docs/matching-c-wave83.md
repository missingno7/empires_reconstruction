# Eighty-third matching-C wave

`F_6036` occupies load offsets `0x6036..0x60A9` and traverses the sprite and
object tables before emitting compact draw records. The complete 115-byte Turbo
C component binds the `DS:BFC8` and `DS:40D0` far-table bases, owned `F_D825`,
numeric `F_03CC`, and the `DS:0A20` counter store. Its object also reproduces
the DGROUP segment load and the one corresponding MZ relocation.

The full EXE and both DAT archives remain byte-identical after promotion.
