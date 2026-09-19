# Seventy-ninth matching-C wave

`F_D818` occupies load offsets `0xD818..0xD825` and is the complete reset
helper for the table at `DS:2F30`. Its fresh Turbo C object reproduces the
13-byte frame, `DI` setup, byte clear, and return exactly. The owner has no
external fixups and no MZ loader relocations.

The complete EXE and both DAT archives remain byte-identical after promotion.
