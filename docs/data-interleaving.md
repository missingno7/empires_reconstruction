# Source DATA and code object interleaving

Turbo Link 2.0 now matches the first 36 relocation-table entries in order,
with the complete load image, fixed MZ fields, all 106 relocation pairs, and
segment/code placement unchanged. The first remaining EXE difference is at
file offset `0xB2`. There are no differences outside the relocation table.

The [candidate recipe](../recipes/data/interleaving-candidate.json) moves two
advancing prefixes of existing DATA objects between code contributions:

| DATA through | After code | Before next relocating code |
|---|---|---|
| RAW_01075A | F_6181 | F_699E |
| PAD_10EDD | F_7BFC | F_9EC3 |

DATA contribution order is preserved. TLINK reads the reordered objects and
generates the executable itself; the experiment adds no padding, final
addresses, or post-link edits. The second interval ends at an already owned
alignment component. Nonrelocating contributions leave the exact historical
module boundaries ambiguous, so this is an ordering constraint rather than
proof of historical translation units.

After the two shared-module links described in
[relocation grouping](relocation-grouping.md), run:

```powershell
python tools/probe_data_interleaving.py
python -m unittest tests.test_data_interleaving tests.test_relocation_groups
```

The [receipt](data-interleaving.json) records the linked byte comparison.
The next divergence concerns ordering across the current four-record DATA
component and adjacent raw DATA. Their physical record boundaries need
verification before changing canonical ownership. Raw DATA, the unpartitioned
BSS reserve, and symbol adapters still prevent claiming a recovered build.
