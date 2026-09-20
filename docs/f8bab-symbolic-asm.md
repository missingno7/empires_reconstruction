# F_8BAB symbolic TASM recovery

`F_8BAB` is a 1,275-byte return-terminated routine recovered as symbolic
TASM. Its fresh object binds exactly to the original load range and the normal
Turbo Link 2.0 build remains byte-identical.

The recovery partitions two embedded dispatch records that a linear
disassembler had misread as instructions. The table at `0x8CCF` has six
key/handler entries; the table at `0x8F14` has eight. Their handler entries
are ordinary same-segment OMF fixups and resolve naturally when linked.

All 63 direct calls now use 33 recovered near external publics. Each `EXTRN`
is declared individually as `:near`; grouping them on a single declaration
caused TASM 1.0 to treat all but the final name as far and grew the object by
34 bytes. The completed source emits 77 fixups: 63 direct calls and 14 local
table entries, while preserving the exact 1,275-byte extent.

The `JMP_NEAR` macro preserves one historical near jump where TASM otherwise
chooses a short encoding. Its target remains assembler-resolved; it is not an
instruction-byte capsule.
