# Linker adapter ledger

The newer [source DATA experiment](source-data-link.md) eliminates the copied
initialized tail and reproduces the entire load image, with 72/106 correct
relocations and no extra sites. The aggregate baseline below remains available
for comparison; BSS and recovered-symbol adapters remain in both paths.

Current full-scaffold checkpoint: Turbo Link 2.0 links without the historical
demand object, with zero errors/unresolved symbols and exact segment bases.
The linked bytes still differ (56/106 relocations; first load difference 0xC8).
See [the checkpoint](tlink20-full-scaffold.md) and [generated metrics](structural-status.json).


The structural path now has an exact code-prefix placement under the locally
verified Turbo Link 2.0 candidate. The remaining adapters are tracked here so
each can be removed against a measurable linker invariant.

| Adapter | Why it exists now | Satisfies | Replacement evidence | Current status |
|---|---|---|---|---|
| `LIBDEMAND.OBJ` historical-library demand | Previously requested historical library publics explicitly | Requests the publics that select the observed `CC.LIB` modules | Actual reconstructed EXTDEFs must preserve selection and order | **Unnecessary in the full scaffold**: fresh demand and no-demand Turbo Link 2.0 outputs have identical maps and EXE hashes, with zero unresolved symbols |
| `DGSCF.OBJ` synthetic DGROUP | Most initialized data and BSS ownership is not yet represented by relocatable objects | Supplies temporary `_DATA`, `_BSS`, and stack sizing plus selected data publics | Replace its 13,932 initialized bytes and 37,252 BSS bytes with canonical data/BSS source objects | **Active**; bounded OMF records are accepted by Turbo Link 2.0. `_BSSEND` is two bytes above the original fixup value despite an identical stack base |
| Oracle-copied initialized DATA tail | Originally supplied initialized bytes to the aggregate scaffold | Holds the unresolved initialized-data extent | Ordered source encoders and compiler DATA contributions | **Removed in the source DATA experiment**; retained only in the baseline comparison path. Local raw source components still require decoding |
| Recovered symbol aliases (`_delay` → `_f6c57`, `_main`) | Recovered objects use numeric/source-local names while startup and callers use historical publics | Resolves verified call targets without changing code bytes | Recover the containing translation unit and its actual public names | **Temporary, tracked per binding** |
| EXTDEF case normalization | Turbo C emits case variants that differ from explicit recovered publics | Resolves case-sensitive OMF externals under the linker candidate | Reconstruct the historical declaration/public spelling in the source module | **Temporary** |
| Injected internal numeric publics (`_Fxxxx`) | Separate recovered objects refer to internal addresses before their owning module is known | Exposes exact code-offset targets to the relocatable experiment | Group contiguous owners into shared historical modules or expose proven real publics | **Temporary** |

The no-demand result is narrower than a recovered build: it proves that the
demand object is unnecessary under the remaining adapters. Without DGSCF there
are still 425 unresolved references. With it, the map agrees but the bytes do
not: 56 relocations versus 106, and the first load-image difference is at 0xC8.
The `_getkey` and `_mode` collisions are now resolved using each caller's
binding evidence and owner-relative labels. These remain temporary aliases,
not historical symbol recovery. The earlier zero-diagnostic claim missed two
map-only fixup overflows; both are eliminated and now covered by detection.
