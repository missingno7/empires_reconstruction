# Component-owned code references

454 declarations across 172 callers now refer to the selected entry publics of
216 matching C/ASM owners. Together with 72 library references and eight
structured-data references, 534 of the 1,665 declared
owner-symbol bindings resolve through component ownership.

`python tools/recover_code_bindings.py` replaces a numeric code address only
when it is exactly an existing C/ASM owner's entry and the external symbol
matches that owner's selected public. Interior addresses, raw targets and
library publics are left unchanged. Existing evidence strings are retained.
The command compiles all 256 source proof units and checks the complete EXE
before publishing the updated manifest. Repeating it makes no further changes.
The migration receipt in `code-binding-evidence.json` records each old address
and its replacement owner, with the verified whole-file hash.

The binder derives both near offsets and far-pointer offsets from the target
owner. It checks the selected public, zero entry addend, code owner type and
the established zero-based `_TEXT` frame. OMF displacement and encoded operand
addends remain separate and are applied as before. Far references still must
produce exactly the MZ relocation sites declared by the reconstructed header.

This removes duplicated placement facts from call-site declarations. It does
not recover source-module boundaries, library ordering, a historical linker,
or the remaining unknown symbols. Target locations still come from the fixed
ownership manifest. `linker_resolved_bindings` therefore remains zero; the
separate `owned_component_bindings` metric is now 534. No raw bytes are promoted by
this step, and the three-file reconstructed game remains byte-identical.
