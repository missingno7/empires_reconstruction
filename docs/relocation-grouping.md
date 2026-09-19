# Relocation ordering as module evidence

`python tools/discover_relocation_groups.py` finds descending relocation runs
that cross matching-C owner boundaries. It emits minimal candidate intervals,
including intervening functions without relocations. These are lower bounds
on possible shared compilation, not exact historical module boundaries.
The [constraints](relocation-group-constraints.json) identify three intervals.

Two candidates now pass fresh compilation and the complete source-DATA link:

| Candidate | Functions | TEXT | Checked fixups | Original relocation order |
|---|---:|---:|---:|---|
| C_75F3_7856 | 5 | 611 | 53 | All 3 entries match |
| RELOC_F_AD25_F_ADCF | 2 | 544 | 48 | All 6 entries match |

The second interval needed one shared, guarded declaration of the existing
27-byte record type. Both independent functions still compile exactly. The
first candidate retains its verified 43-byte DATA layout through the existing
DATA separation adapter. Sequential replacement puts both real compiler TEXT
modules into one link: seven separate TEXT objects become two, the full load
image stays exact, and each group's relocation order emerges correctly.

Reproduce both replacements:

```powershell
python tools/probe_shared_module_link.py --source-data
python tools/probe_shared_module_link.py --source-data --recipe recipes/modules/C_AD25_AF45.json --input-report build/shared-source-data-link-report_C_75F3_7856.json
```

Receipts: [five-function group](shared-relocation-C_75F3_7856.json),
[combined link after the second group](shared-relocation-RELOC_F_AD25_F_ADCF.json).
The fixed game build also remains byte-identical. Full relocation-table order
is still different, beginning at file 0x3A where DATA/code interleaving differs.

The third candidate, F_DDD9 through F_DF98, is not yet a compatible semantic-C
module. F_DF98 is an inline-ASM byte capsule despite its manifest category.
Combining it with the preceding semantic C causes TASM duplicate definitions
of LXMUL@ and LDIV@. The concrete diagnostic is recorded here rather than
claiming a recovered module or changing bytes to silence the assembler.
