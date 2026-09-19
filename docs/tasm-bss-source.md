# Turbo Assembler BSS source

The exact structural link no longer supplies game BSS through `DGSCF.OBJ` or a
custom OMF writer. `tools/bss_asm.py` generates a TASM source module from the
canonical 37,250-byte reserve and the recovered public map. The pinned TASM 1.0
assembles a word-aligned public `_BSS` segment in `DGROUP`; Turbo Link 2.0 then
derives the unchanged BSS base, runtime BSS placement, stack base and MZ fields.

The source-DATA receipt records 247 public labels, source SHA-256
`2ab5ba7915049465aa6baa2b7614f81bb7424df38171a61a9dd91112a0726ccd`,
and object SHA-256
`9bfbd2175de4d9a644294e7cc5cf19c9a6431e22031b975b163785d0d2942c7b`.
The object declares 37,250 uninitialized bytes and contributes no load-image
payload. The staged source timestamp is fixed because TASM records it in an OMF
comment; two fresh assemblies produce the same object hash. The full
historical-linker output remains byte-identical.

This removes the synthetic DGROUP/BSS object from the exact path. It does not
prove that the game historically used one BSS translation unit. The public map
still consolidates recovered aliases from separate source proofs, and the
reserve is not internally partitioned into historical modules. Those are the
next ownership constraints rather than reasons to retain a synthetic object.
