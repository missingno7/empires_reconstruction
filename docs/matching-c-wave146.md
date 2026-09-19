# Matching-C wave 146

`LIB_STRLEN` is now independently reproducible as a matching-C component.
The canonical source is a fresh Turbo C translation unit with inline bytes for
the historical compact-model far-pointer scan. The pinned compiler emits a
27-byte `_TEXT` contribution with public `_strlen`, no initialized data, no
BSS, and no OMF fixups. Its complete object segment is byte-identical to the
`CC.LIB` `STRLEN` module and to the original executable extent.

This is a source proof and a library cross-check; the proprietary library
binary remains available for independent verification, but it is no longer
the ownership source for this extent.

The fixed reconstruction, archive outputs, and relocation table remain exact.
