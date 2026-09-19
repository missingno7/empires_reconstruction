# Source DATA and code object interleaving

Turbo Link 2.0 now matches the first 72 relocation-table entries in order,
with the complete load image, fixed MZ fields, all 106 relocation pairs, and
segment/code placement unchanged. The first remaining EXE difference is at
file offset `0x142`. There are no differences outside the relocation table.

The [candidate recipe](../recipes/data/interleaving-candidate.json) moves four
advancing prefixes of existing DATA objects between code contributions:

| DATA through | After code | Before next relocating code |
|---|---|---|
| RAW_01075A | F_6181 | F_699E |
| PAD_10EDD | F_7BFC | F_9EC3 |
| DATA_0101220_ENERGY_PROMPT | F_9EC3 | F_A09D |
| RAW_011FAE | F_ADCF | F_DDD9 |

DATA contribution order is preserved. TLINK reads the reordered objects and
generates the executable itself; the experiment adds no padding, final
addresses, or post-link edits. Every interval ends at an existing source
boundary that preserves the observed alignment. Nonrelocating contributions leave the exact historical
module boundaries ambiguous, so this is an ordering constraint rather than
proof of historical translation units.

After the two shared-module links described in
[relocation grouping](relocation-grouping.md), run:

```powershell
python tools/probe_data_interleaving.py
python -m unittest tests.test_data_interleaving tests.test_relocation_groups
```

The [receipt](data-interleaving.json) records the linked byte comparison.
The former four-record component, one-byte compiled initializer, and 43-byte
raw owner are now one six-record typed table plus a four-byte u16 trailer.
The next divergence is the arithmetic interval F_DDD9 through F_DF98, whose
inline-ASM capsule currently blocks a single shared compilation. Raw DATA, the
unpartitioned BSS reserve, and symbol adapters still prevent claiming a
recovered build.
