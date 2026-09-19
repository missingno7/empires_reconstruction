# Byte-identical Turbo Link 2.0 experiment

`python tools/probe_exact_structural_link.py` now produces a byte-identical
`AEPROG.EXE` through the pinned historical Turbo Link 2.0. The command builds
relocatable source-DATA objects, replaces eleven individual TEXT objects with
three fresh shared Turbo C objects, interleaves DATA contributions at four
verified source boundaries, and invokes TLINK. It does not edit the linked EXE.

The resulting 79,154-byte file has the original SHA-256:

```text
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10
```

The fixed MZ fields, all 106 relocation entries in order, the 78,642-byte load
image, complete TEXT, and initialized DATA all compare equal. The
[machine-readable receipt](exact-structural-link.json) records each stage and
the byte-level comparison.

This is the first byte-identical historical-linker output, not completion of
the reconstruction. It still uses a candidate object-interleaving recipe, one
checked FIXUPP-order adapter for the inline-ASM arithmetic module, the
mostly aggregate BSS ownership with 247 canonical public anchors, and four evidenced DATA/code ordering
constraints. Canonical EXE and source-link raw DATA are both zero, and the
structural path performs zero object symbol transforms. All standalone TASM
owners now declare their Turbo C-compatible empty `_DATA`/`_BSS` and `DGROUP`
topology in source. The first 34 BSS bytes are a compatible symbolic
`F01CEBSS.OBJ` contribution; `GAMEBSS.OBJ` is the 37,216-byte anchored
remainder. The arithmetic adapter changes only the order of explicit OMF fixup
subrecords; it rejects threads and verifies that segment bytes, publics,
externals, targets, addends, and sites remain identical before TLINK runs.
The former synthetic DGROUP/BSS object has been replaced by a real
[TASM BSS source contributions](tasm-bss-source.md); their internal historical
ownership and further partitioning remain open.

The fixed-placement build remains an independent oracle and still rebuilds all
three game files byte-identically. The next structural work is to replace the
remaining adapters with recovered module/source ownership while requiring this
exact TLINK result to remain unchanged.
