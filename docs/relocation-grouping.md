# Relocation ordering as module evidence

`python tools/discover_relocation_groups.py` finds descending relocation runs
that cross matching-C owner boundaries. It emits minimal candidate intervals,
including intervening functions without relocations. These are lower bounds
on possible shared compilation, not exact historical module boundaries.
The [constraints](relocation-group-constraints.json) identify three intervals.

All three candidates now pass fresh compilation and the complete source-DATA link:

| Candidate | Functions | TEXT | Checked fixups | Original relocation order |
|---|---:|---:|---:|---|
| C_75F3_7856 | 5 | 611 | 53 | All 3 entries match |
| RELOC_F_AD25_F_ADCF | 2 | 544 | 48 | All 6 entries match |
| RELOC_F_DDD9_F_DF98 | 4 | 700 | 17 | Checked adapter makes all 10 match |

The second interval needed one shared, guarded declaration of the existing
27-byte record type. Both independent functions still compile exactly. The
first candidate retains its verified 43-byte DATA layout through the existing
DATA separation adapter. Sequential replacement puts both real compiler TEXT
modules into one link: seven separate TEXT objects become two, the full load
image stays exact, and each group's relocation order emerges correctly.

The arithmetic interval now also compiles as one exact 700-byte object. Its
inline-ASM capsule suppresses duplicate helper declarations in shared mode and
lets Turbo C supply the final `ret`; standalone compilation remains unchanged.
The grouped object has the exact public offsets and 17 checked fixups, and it
replaces four TEXT objects without moving downstream code. Its ten MZ entries
are ascending in the fresh compiler object because the current inline-ASM
capsule differs from ordinary Turbo C FIXUPP emission. A checked relocatable
adapter orders those explicit subrecords descending without changing any
relocation semantics. TLINK then emits the original order. This is still a
compatible module, not yet a strongly evidenced historical module.

Reproduce both replacements:

```powershell
python tools/probe_shared_module_link.py --source-data
python tools/probe_shared_module_link.py --source-data --recipe recipes/modules/C_AD25_AF45.json --input-report build/shared-source-data-link-report_C_75F3_7856.json
python tools/probe_shared_module_link.py --source-data --recipe recipes/modules/C_DDD9_E095.json --input-report build/shared-source-data-link-report_RELOC_F_AD25_F_ADCF.json
```

Receipts: [five-function group](shared-relocation-C_75F3_7856.json),
[combined link after the second group](shared-relocation-RELOC_F_AD25_F_ADCF.json).
The [arithmetic group receipt](shared-relocation-RELOC_F_DDD9_F_DF98.json)
records the exact load image and the explicit FIXUPP adapter. The fixed game
build also remains byte-identical. With verified DATA/code interleaving, the
historical linker now emits the complete byte-identical EXE.
