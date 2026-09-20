# M_C5D1_C706 symbolic sound-control module

`M_C5D1_C706` replaces the compatible shared-C proof candidate
`C_C5D1_C706` in the canonical structural link. It is a 388-byte ordinary TASM
contribution containing `F_C5D1` (167 bytes), `F_C678` (65), `F_C6B9` (77), and
`F_C706` (79).

The recovered code forms a compact sound-control cluster:

- `F_C5D1` decodes a command byte and updates the SI-relative
  `17A4h..17ECh` state.
- `F_C678` enables a voice through the speaker gate or alternate backend.
- `F_C6B9` disables a voice or submits a reset.
- `F_C706` writes, queues, or encodes a PIT frequency value.

Fresh TASM emits the original `_TEXT` extent and public offsets 0, 167, 232,
and 309. The source has no OMF FIXUPP records, matching the historical shared-C
object: calls outside the contribution are deliberate same-segment relative
expressions, while calls among the four recovered entries are ordinary symbolic
local calls. The canonical `python tools/build_exe.py verify` link remains
byte-identical, including all 106 relocations in historical order.

This is compatible reconstructed-module evidence from contiguity, shared
sound-control state, and direct local control flow. It does not assert the
historical source filename.
