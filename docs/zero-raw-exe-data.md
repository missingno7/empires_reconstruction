# Zero opaque EXE fallback

The canonical `AEPROG.EXE` manifest now has no `RAW` owner. The final eight
owners, totaling 257 bytes, are `typed-data-v1` source components containing
explicit byte/word arrays, signed sentinels and symbolic references.

The largest was the 139-byte pair of menu descriptor tables. Its 20 far
pointers are source-level targets, including four pointers internal to the
component. The other components describe file-error control state, two control
codes, a level-control table, keyboard and user control records, a cached-index
sentinel, and three runtime limit words. Four more far pointers are symbolic;
one targets the reconstructed GAME_BSS reserve.

`tools/typed_data.py` emits unbound pointer fields and OMF FIXUPP records. It
does not store final segment values. Focused tests compare every encoded extent
with the original and inspect all 24 emitted pointer fixups. The fixed build
and the exact Turbo Link 2.0 experiment remain byte-identical, and the source
DATA report records zero `raw-local` bytes.

This closes opaque EXE byte ownership. It does not establish historical C
declarations or translation-unit boundaries. Recovered aliases, case mapping,
injected publics, candidate object interleaving, the arithmetic FIXUPP-order
adapter, and unpartitioned BSS remain separate structural work.
