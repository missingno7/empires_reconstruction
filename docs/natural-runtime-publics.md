# Natural runtime-block publics

The final generic code-symbol adapter is removed. `src/RUNTIME_BLOCK.C` now
declares its 27 callable entry publics directly in inline assembly. Each public
is attached to the corresponding 3-byte jump-table entry, including the
semantic aliases `box`, `bar`, `clear`, `fill`, `wipe`, `blit`, and `copy`.

Turbo C 2.0 passes these declarations to TASM 1.0, which writes ordinary OMF
PUBDEF records at offsets `0x00` through `0x39`. The canonical source also
exports `_runtime_block_end`; the fixed verifier uses that public to delimit the
6,571-byte owned contribution. No final load address is part of this rule.

The current complete structural-link report has:

```text
object transforms:       0
unresolved symbols:      0
first code divergence:   none
_TEXT:                   00000..0FA22
whole EXE:               byte identical
SHA-256: 1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10
```

The linker probe still computes internal-label evidence as a diagnostic, but
`add_publics` changes no object because every required public already exists in
the fresh compiler/TASM output. The prior PUBDEF injection path is therefore
inactive for every object.

This closes the forced symbol-adapter frontier. Remaining structural shortcuts
are object/module grouping, the unpartitioned BSS/public source, candidate
DATA/code interleaving, and the narrow arithmetic FIXUPP ordering adapter.

