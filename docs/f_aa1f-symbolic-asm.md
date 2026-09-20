# F_AA1F symbolic assembly recovery

`F_AA1F` (`0xAA1F..0xAB66`) is now a 327-byte symbolic TASM owner.  It is a
state-key dispatch loop: five word keys select five near handlers through a
CS-relative handler table.  The six direct calls are emitted as real near
external relations to `_fa15e`, `_f6b66`, `_faf45`, `_fa036`, `_f86c9`, and
`_fa24e`.

The table begins at owner offset `0x49`.  Its keys are `000D`, `0012`, `001B`,
`0148`, and `0150`; its handlers begin at `0xAAB5`, `0xAB3B`, `0xAA94`,
`0xAA87`, and `0xAA7C`.  Treating the two tables as instructions had obscured
the `inc si` at `0xAA7C`; restoring that instruction and targeting the
post-table mode-two path from the first-handler conditional produces the
required 327-byte extent.

Fresh TASM assembly has 16 OMF fixups.  Binding its six external calls and ten
local table/code relations reproduces every owner byte and fixup, and the
normal structural Turbo Link build remains the final whole-EXE proof.
