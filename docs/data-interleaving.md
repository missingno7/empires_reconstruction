# Source DATA and code object interleaving

Turbo Link 2.0 now matches all 106 relocation-table entries in order,
with the complete load image, fixed MZ fields, all 106 relocation pairs, and
segment/code placement unchanged. The complete EXE is byte-identical.

The [candidate recipe](../recipes/data/interleaving-candidate.json) moves four
advancing prefixes of existing DATA objects between code contributions:

| DATA through | After code | Before next relocating code |
|---|---|---|
| DATA_01075A_FILE_ERROR_CONTROL | F_6181 | F_699E |
| PAD_10EDD | F_7BFC | F_9EC3 |
| DATA_0101220_ENERGY_PROMPT | F_9EC3 | F_A09D |
| DATA_011FAE_CACHED_INDEX | F_ADCF | F_DDD9 |

DATA contribution order is preserved. TLINK reads the reordered objects and
generates the executable itself; the experiment adds no padding, final
addresses, or post-link edits. Every interval ends at an existing source
boundary that preserves the observed alignment. Nonrelocating contributions leave the exact historical
module boundaries ambiguous, so this is an ordering constraint rather than
proof of historical translation units.

After the three shared-module links described in
[relocation grouping](relocation-grouping.md), run:

```powershell
python tools/probe_data_interleaving.py
python -m unittest tests.test_data_interleaving tests.test_relocation_groups
```

The [receipt](data-interleaving.json) records the linked byte comparison.
The former four-record component, one-byte compiled initializer, and 43-byte
raw owner are now one six-record typed table plus a four-byte u16 trailer.
The arithmetic interval F_DDD9 through F_DF98 now compiles as one exact object.
A checked OMF adapter orders its explicit FIXUPP subrecords like historical
Turbo C output; TLINK then produces the byte-identical file. Raw DATA, the
unpartitioned BSS reserve and object-order evidence still
prevent claiming a recovered build.

Construction now reads the ordered relocation expectation from
`layout/mz-header.json`; the original EXE is opened only when an optional
verification receipt is requested.
