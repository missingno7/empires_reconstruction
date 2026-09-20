# M_D61C_D79C symbolic record renderer

`M_D61C_D79C` replaces the compatible shared-C candidate `C_D61C_D79C` in the
canonical link. The 507-byte TASM source recovers two adjacent renderers:

- `F_D61C` sets the 24-cell `0x2380` lattice to its highlighted attribute,
  renders the ES:DI record stream, then restores the normal attribute; and
- `F_D79C` performs the same record-stream rendering without changing the
  lattice.

The TASM macro expresses the repeated cell-attribute stores while preserving
the original 8-bit first displacement and 16-bit later displacements. The
record loops retain the historical direct same-segment renderer calls as
symbolic relative expressions, so the module emits no OMF FIXUPP records.

Fresh TASM emits public offsets 0 and 384 and the exact 507-byte `_TEXT`
extent. The canonical verified link remains byte-identical with all 106 MZ
relocations in their historical order. This is compatible reconstructed-module
evidence; it does not identify the original source filename.
