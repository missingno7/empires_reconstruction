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

The construction-time EXE-fixture dependency has been removed. The normal
`--no-verify` build derives temporary baseline dimensions from structured MZ
and manifest metadata; `assets/AEPROG.EXE` is now only an optional verification
oracle. The [fixture audit](fixture-dependency-audit.json) records each former
read and its replacement. This changes the construction-adapter count from
one to zero. Source-level TASM declarations have also removed the empty-DGROUP
metadata adapter; the two remaining topology adapters below remain active.

| Adapter | Why it exists now | Satisfies | Replacement evidence | Current status |
|---|---|---|---|---|
| `LIBDEMAND.OBJ` historical-library demand | Previously requested historical library publics explicitly | Requests the publics that select the observed `CC.LIB` modules | Actual reconstructed EXTDEFs must preserve selection and order | **Unnecessary in the full scaffold**: fresh demand and no-demand Turbo Link 2.0 outputs have identical maps and EXE hashes, with zero unresolved symbols |
| `DGSCF.OBJ` synthetic DGROUP | Previously supplied initialized DATA, BSS and selected publics | Temporary DGROUP sizing and bindings | Real source DATA plus TASM BSS contributions | **Removed from the exact experiment**; retained only by the aggregate diagnostic baseline |
| Anchored TASM BSS reserve | Historical BSS translation units were initially unknown | Supplied the verified 37,250-byte BSS range and 247 canonical public anchors | Partition every interval into an anchored source contribution | **Removed.** The source contribution plan now owns all 37,250 bytes; aggregate remainder is zero. Exact historical translation-unit ownership remains a separate source-recovery question. |
| Oracle-copied initialized DATA tail | Originally supplied initialized bytes to the aggregate scaffold | Holds the unresolved initialized-data extent | Ordered source encoders and compiler DATA contributions | **Removed in the source DATA experiment**; canonical and source-link raw DATA are both zero |
| Recovered symbol aliases | Recovered objects previously used numeric/source-local names while startup and callers used historical publics | Resolved verified call targets without changing code bytes | Name the actual library or reconstructed-owner public in source | **Removed**. Source now names historical CC.LIB publics and reconstructed entry publics directly; numeric entry bindings are canonical owner/public references |
| EXTDEF case normalization | Turbo C previously emitted case variants that differed from explicit recovered publics | Resolved case-sensitive OMF externals under the linker candidate | Reconstruct the declaration/public spelling in source | **Removed from the current structural link** |
| Injected internal publics | The 6,571-byte runtime dispatch block contains multiple callable entries but compiles as one aggregate function | Exposed 27 exact internal entries to the relocatable experiment | Emit the publics naturally from canonical source | **Removed**. Inline assembly now produces all 27 OMF PUBDEFs and the structural path performs zero object transforms |
| Candidate DATA/code object interleaving | Historical object boundaries and response-file order remain partly unknown | Reproduces the four observed cross-segment relocation-order constraints | Recover containing modules and their natural object order | **Active**; exact TLINK output, but nonrelocating boundaries remain ambiguous |
| Arithmetic FIXUPP subrecord ordering | The inline-ASM capsule emits ascending explicit fixups unlike ordinary Turbo C output | Reproduces the historical descending ten-entry run before TLINK | Recover a coherent source/assembly module that naturally emits the observed OMF record topology | **Active and narrowly checked.** The verified symbolic [`F_DF98` candidate](fdf98-symbolic-asm.json) proves TASM 1.0 emits its two records in the required descending order without rewriting. It cannot yet replace the complete 700-byte shared arithmetic object, because splitting that object changes the cross-function relocation sequence. |
| Turbo C-compatible empty DGROUP metadata | Some standalone TASM owners originally omitted Turbo C's empty `_DATA`/`_BSS` declarations and `DGROUP` membership | Kept their linker group topology compatible during recovery | Add those declarations to each reconstructed TASM source | **Removed.** All 24 standalone TASM modules now emit the required topology naturally; the structural path applies zero DGROUP metadata rewrites |

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

The first source-compatible BSS island is recorded in
[the ownership candidates](bss-ownership-candidates.json): `F_01CE` has a
fully bounded 34-byte prefix containing `cur_idx` and `g3904[16]`.
`recipes/data/bss-contributions.json` now models the contribution order and
assembles that prefix as `F01CEBSS.OBJ`; the remaining reserve begins at
logical byte 34. This is compatible symbolic ownership, not proof of the
historical translation-unit boundary.
