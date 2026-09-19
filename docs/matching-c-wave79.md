# Seventy-ninth source-recovery wave

`F_D818` occupies load offsets `0xD818..0xD825` and is the complete reset
helper for the table at `DS:2F30`. Its symbolic TASM source reproduces the
13-byte frame, `DI` setup, byte clear, and return exactly. The owner has no
external fixups and no MZ loader relocations. Turbo C cannot emit this exact
frame from the readable C form, so symbolic assembly is the canonical source.

The complete EXE and both DAT archives remain byte-identical after promotion.
