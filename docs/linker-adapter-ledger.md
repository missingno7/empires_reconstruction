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
| `DGSCF.OBJ` synthetic DGROUP | Previously supplied initialized DATA, BSS and selected publics | Temporary DGROUP sizing and bindings | Real source DATA plus TASM BSS contributions | **Removed from the exact experiment**; retained only by the aggregate diagnostic baseline |
| Anchored TASM BSS reserve | Historical BSS translation units remain unknown | Supplies the verified 37,250-byte reserve and 247 canonical public anchors through real TASM OMF | Partition storage and publics among evidenced historical modules | **Active source component**; its canonical public map is checked against linker bindings, and no custom/synthetic OMF object remains in the exact path |
| Oracle-copied initialized DATA tail | Originally supplied initialized bytes to the aggregate scaffold | Holds the unresolved initialized-data extent | Ordered source encoders and compiler DATA contributions | **Removed in the source DATA experiment**; canonical and source-link raw DATA are both zero |
| Recovered symbol aliases | Recovered objects previously used numeric/source-local names while startup and callers used historical publics | Resolved verified call targets without changing code bytes | Name the actual library or reconstructed-owner public in source | **Removed**. Source now names historical CC.LIB publics and reconstructed entry publics directly; numeric entry bindings are canonical owner/public references |
| EXTDEF case normalization | Turbo C previously emitted case variants that differed from explicit recovered publics | Resolved case-sensitive OMF externals under the linker candidate | Reconstruct the declaration/public spelling in source | **Removed from the current structural link** |
| Injected internal publics | The 6,571-byte runtime dispatch block contains multiple callable entries but compiles as one aggregate function | Exposed 27 exact internal entries to the relocatable experiment | Emit the publics naturally from canonical source | **Removed**. Inline assembly now produces all 27 OMF PUBDEFs and the structural path performs zero object transforms |
| Candidate DATA/code object interleaving | Historical object boundaries and response-file order remain partly unknown | Reproduces the four observed cross-segment relocation-order constraints | Recover containing modules and their natural object order | **Active**; exact TLINK output, but nonrelocating boundaries remain ambiguous |
| Arithmetic FIXUPP subrecord ordering | The inline-ASM capsule emits ascending explicit fixups unlike ordinary Turbo C output | Reproduces the historical descending ten-entry run before TLINK | Recover source/assembly that naturally emits the observed OMF record topology | **Active and narrowly checked**; changes no bytes, publics, targets, addends or sites |

The no-demand result is narrower than a recovered build: it proves that the
demand object is unnecessary under the remaining adapters. The older aggregate
scaffold still differs; the source-DATA/interleaving path is the exact result.
The former `_getkey`, `_mode`, `_delay`, and runtime-library aliases are now
direct source references. Exact entry bindings use their owner/public identity,
leaving internal-entry PUBDEF injection as the sole code-symbol adapter at that
checkpoint. Canonical inline assembly now emits those final 27 publics, so the
current path has no symbol adapter; see
[the natural-public checkpoint](natural-runtime-publics.md). The earlier zero-diagnostic claim missed two
map-only fixup overflows; both are eliminated and now covered by detection.
