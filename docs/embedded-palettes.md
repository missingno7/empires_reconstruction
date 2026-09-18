# Embedded DAC components

Two contiguous 768-byte EXE components now have canonical local sources in
`raw/exe-data/PALETTE_DAC6_011E.json` and `PALETTE_DAC6_041E.json`.
Each source preserves 256 ordered RGB triples, with six-bit channels, using
`dac6-rgb256-v1`. Duplicate entries remain distinct; no palette optimization,
gamma conversion, channel scaling or index remapping occurs.

The classification comes from matching code: `F_0281` passes `g11e` or `g41e`
to `F_01BC`, which invokes video BIOS interrupt 10h with AX=1012h and CX=0100h.
The established DGROUP frame maps these references to file ranges
`[0xFD4E,0x1004E)` and `[0x1004E,0x1034E)`. Both blocks contain only six-bit
channel values. `python tools/recover_palettes.py` freshly compiles and binds
both functions and compares their bytes before publishing the promotion.
The metadata-only evidence receipt is `embedded-palettes.json`.

The two bindings in `F_0281` now identify their data owners and an addend.
Binding derives the DGROUP offsets from component placement. This removes
duplicated literal addresses, but placement still comes from the fixed EXE
manifest; it does not recover a historical linker or source module.

Normal reconstruction encodes the JSON and verifies its bytes before launching
the compiler. `tools/exe_data.py` is independent of originals and accepts valid
channel edits; the current exact-build verifier rejects modified output.
`python tools/extract_raw.py` prepares local sources from the pinned original
for checkout bootstrap. That explicit extraction command resets local sources.
Game-derived JSON stays ignored, together with raw data and build outputs.

This advances 1,536 bytes from opaque fallback to decoded, rebuildable source.
The EXE now has 251 owners, including two EXACT_DATA components and 67 raw
regions totaling 56,241 bytes. Unresolved machine extents still cover 34,965
raw bytes; the other 21,276 remain unclassified.
