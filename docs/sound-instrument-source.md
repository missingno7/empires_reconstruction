# Sound voice and instrument DATA source

`DATA_012C03_SOUND_INSTRUMENTS` replaces the 1,923-byte raw extent at file
offsets `0x12C03..0x13386`. The layout is mechanically supported by matching-C
cross-references and exact boundaries:

- 17 bytes complete the nine two-byte slot-glyph entries whose first byte lies
  at the end of the preceding zero-initialized extent;
- three 18-byte arrays begin at DGROUP offsets `0x2FE4`, `0x2FF6`, and `0x3008`;
- 33 records of 56 bytes begin at `0x301A`;
- two signed `-1` words terminate the component.

Each record is represented as 28 signed 16-bit values. `F_D8F0` independently
proves the 56-byte stride and writes the field at record offset 42 through the
alias at `0x3044`. `F_DA66` consumes the two final record words at offsets 52
and 54. These facts establish record width and selected fields without
inventing names for the remaining parameters.

`tools/sound_instruments.py` strictly validates the array counts, 33-by-28
record matrix, signed range, terminator, and total extent. The fixed build and
the exact Turbo Link 2.0 path both reproduce the original executable after the
promotion.
