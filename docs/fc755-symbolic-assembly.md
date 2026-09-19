# F_C755 symbolic assembly receipt

`F_C755` is now a 37-byte symbolic TASM owner.  The prior Turbo C wrapper
contained only `asm db` records and did not expose any of the routine's linker
relationships.

The recovered control flow clears `g1784`, sets `g1786` to `-1`, then walks a
word table starting at `g17ac` for the `g177a` configured entries.  Each entry
is cleared before calling the recovered `F_C6B9` service routine.

The fresh TASM contribution has five relocatable fixups: four DGROUP offsets
at instruction offsets 5, 11, 19, and 23, followed by the near self-relative
call to `_f_c6b9` at offset 28.  The four data offsets are independently
bound by `F_C27D` and/or `F_C834`; the callee is also independently bound by
`F_C27D`, `F_C834`, and `F_C877`.

This is symbolic assembly rather than a claim about an original source file.
The public labels retain their mechanically established DGROUP-offset names
until the wider sound-state data model has been recovered.
