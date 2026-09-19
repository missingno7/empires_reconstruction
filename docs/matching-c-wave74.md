# Seventy-fourth structured-data wave

`DATA_01BA` is the two-byte tail immediately following the recovered
`F_01A4` DOS setup routine. The setup reads the word through `CS:01BA`, so the
bytes are a standalone initialized data object rather than an unresolved code
continuation. Its canonical source is `src/data/DATA_01BA.json`, encoded by the
deterministic `u16le-table-v1` adapter.

The source emits one little-endian zero word (`00 00`) at load offsets `0x01BA`
through `0x01BC`. The manifest now classifies this complete extent as
`EXACT_DATA`; the previous raw owner is gone. The fresh encoder matches the
original extent and the full rebuilt `AEPROG.EXE`, `AE000.DAT`, and `AE001.DAT`
remain byte-identical. This wave adds two structured bytes and removes two raw
bytes without changing the fixed-layout proof.

The remaining raw EXE regions are data-heavy spans rather than held code
candidates. Their next useful step is mechanical table/pointer classification
with exact encoders and round-trip proofs.
