# Relocatable sound DATA source

`DATA_01139E_SOUND` replaces the 1,832-byte raw extent at file offsets
`0x1139E..0x11AC6`. Code cross-references and the stored pointer values expose
a stable mechanical layout:

- 71 initialized state words;
- 24 note-divisor words and two pointers to their equal banks;
- the `0x388` OPL port;
- 36 near pointers into the following lookup sequences;
- a 13-byte lookup prefix, the target-delimited lookup sequences, and nine
  trailing words.

The canonical JSON stores values and symbolic target names. It does not store
the original final DGROUP offsets. `tools/sound_data.py` compiles the component
with zero placeholders and 38 `offset16` references. `tools/data_omf.py` emits
those references as OMF FIXUPP subrecords, and Turbo Link 2.0 resolves them
after placing the `_DATA` contribution.

The decoder checks all pointer targets against the observed component extent.
Unit tests verify decode/encode identity and the OMF fixup types. Both the fixed
oracle build and `tools/probe_exact_structural_link.py` reproduce the complete
original executable and its SHA-256 after the promotion.

The neutral table names record verified structure without claiming unknown
musical or gameplay meaning for individual lookup sequences.
