# Matching-C wave 147

`LIB_RAND` is now independently reproducible as a semantic matching-C
component. The source models the compact-model `srand` state assignment and
the historical linear-congruential `rand` step. Fresh Turbo C emits the exact
57-byte `_TEXT` contribution, the four-byte initialized `_DATA` state value,
and no BSS.

The object has publics `_srand` at offset 0 and `_rand` at offset 17, the
external `LXMUL@`, and all eight historical OMF fixups. Its complete object
segments and binding metadata match the `CC.LIB` `RAND` module and the original
executable extent at load offset `0xF8FE`.

The fixed reconstruction remains the oracle. The linker probe replaces the
temporary library module's `_TEXT` and `_DATA` contributions with this fresh
object while preserving TLINK's natural library extraction position.
