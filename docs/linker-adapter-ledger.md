# Linker adapter ledger

The [exact structural-link experiment](exact-structural-link.md) eliminates the
copied initialized tail and makes Turbo Link 2.0 emit the byte-identical EXE,
including all 106 ordered relocations. The aggregate baseline remains available
for comparison; the exact experiment still has the adapters listed below.

The older aggregate full-scaffold checkpoint links without the demand object
but still differs. It remains a diagnostic baseline beside the exact
source-DATA path. See [the baseline](tlink20-full-scaffold.md) and
[generated metrics](structural-status.json).


The structural path now has an exact code-prefix placement under the locally
verified Turbo Link 2.0 candidate. The remaining adapters are tracked here so
each can be removed against a measurable linker invariant.

| Adapter | Why it exists now | Satisfies | Replacement evidence | Current status |
|---|---|---|---|---|
| `LIBDEMAND.OBJ` historical-library demand | Previously requested historical library publics explicitly | Requests the publics that select the observed `CC.LIB` modules | Actual reconstructed EXTDEFs must preserve selection and order | **Unnecessary in the full scaffold**: fresh demand and no-demand Turbo Link 2.0 outputs have identical maps and EXE hashes, with zero unresolved symbols |
| `DGSCF.OBJ` synthetic DGROUP | BSS ownership is not yet partitioned into real source modules | Supplies the temporary `_BSS` reserve and selected BSS publics | Replace its 37,250 BSS bytes with canonical module contributions | **Active** in the exact experiment; initialized DATA is empty and supplied by source objects |
| Oracle-copied initialized DATA tail | Originally supplied initialized bytes to the aggregate scaffold | Holds the unresolved initialized-data extent | Ordered source encoders and compiler DATA contributions | **Removed in the source DATA experiment**; retained only in the baseline comparison path. Local raw source components still require decoding |
| Recovered symbol aliases (`_delay` → `_f6c57`, `_main`) | Recovered objects use numeric/source-local names while startup and callers use historical publics | Resolves verified call targets without changing code bytes | Recover the containing translation unit and its actual public names | **Temporary, tracked per binding** |
| EXTDEF case normalization | Turbo C emits case variants that differ from explicit recovered publics | Resolves case-sensitive OMF externals under the linker candidate | Reconstruct the historical declaration/public spelling in the source module | **Temporary** |
| Injected internal numeric publics (`_Fxxxx`) | Separate recovered objects refer to internal addresses before their owning module is known | Exposes exact code-offset targets to the relocatable experiment | Group contiguous owners into shared historical modules or expose proven real publics | **Temporary** |
| Candidate DATA/code object interleaving | Historical object boundaries and response-file order remain partly unknown | Reproduces the four observed cross-segment relocation-order constraints | Recover containing modules and their natural object order | **Active**; exact TLINK output, but nonrelocating boundaries remain ambiguous |
| Arithmetic FIXUPP subrecord ordering | The inline-ASM capsule emits ascending explicit fixups unlike ordinary Turbo C output | Reproduces the historical descending ten-entry run before TLINK | Recover source/assembly that naturally emits the observed OMF record topology | **Active and narrowly checked**; changes no bytes, publics, targets, addends or sites |

The no-demand result is narrower than a recovered build: it proves that the
demand object is unnecessary under the remaining adapters. The older aggregate
scaffold still differs; the source-DATA/interleaving path is the exact result.
The `_getkey` and `_mode` collisions are now resolved using each caller's
binding evidence and owner-relative labels. These remain temporary aliases,
not historical symbol recovery. The earlier zero-diagnostic claim missed two
map-only fixup overflows; both are eliminated and now covered by detection.
