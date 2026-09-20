# Matching-C wave 146

`LIB_STRLEN` remains independently reproducible as a matching-C proof component.
The canonical source is a fresh Turbo C translation unit with inline bytes for
the historical compact-model far-pointer scan. The pinned compiler emits a
27-byte `_TEXT` contribution with public `_strlen`, no initialized data, no
BSS, and no OMF fixups. Its complete object segment is byte-identical to the
`CC.LIB` `STRLEN` module and to the original executable extent.

This is source evidence and a library cross-check. The canonical production
owner is now the pinned `CC.LIB` `STRLEN` module: ordinary unresolved `_strlen`
demand selects it directly, with no source-generated library replacement or
temporary library rewrite. The matching-C fragment remains a regression proof,
not a final game-source owner.

The fixed reconstruction, archive outputs, and relocation table remain exact.
