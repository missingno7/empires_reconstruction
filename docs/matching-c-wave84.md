# Eighty-fourth matching-C wave

`F_6181` occupies load offsets `0x6181..0x622C` and updates a sprite record
before issuing its two draw paths. The complete 171-byte Turbo C component binds
the `DS:BFC8` and `DS:40D0` far-table bases and numeric `F_03B4`/`F_03CC` calls,
preserves the DGROUP segment load, and reproduces its one MZ relocation.

The full EXE and both DAT archives remain byte-identical after promotion.
