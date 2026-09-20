# Shared sound-control symbolic recovery constraint

`C_C5D1_C706` is a 388-byte compatible shared Turbo C contribution containing
`F_C5D1` (167 bytes), `F_C678` (65), `F_C6B9` (77), and `F_C706` (79). A
promotion of one member invalidates the current compatible shared-C proof, so
any symbolic replacement must reproduce the whole ordered contribution.

The disassembly establishes a compact sound-control cluster:

- `F_C5D1` dispatches a command byte, updates the SI-relative `17A4h..17ECh`
  state, and calls the later three helpers.
- `F_C678` controls the speaker gate or queues a per-voice value.
- `F_C6B9` clears the speaker gate or dispatches a voice update.
- `F_C706` writes the PIT channel or queues the frequency value.

The canonical object has a 388-byte `_TEXT` extent and its public entry offsets
are 0, 167, 232, and 309. The future symbolic source must retain this order,
those offsets, all explicit DGROUP and call fixups, and exact whole-EXE output.
It must be tested as a replacement for `C_C5D1_C706`, never as separate owner
promotions.
