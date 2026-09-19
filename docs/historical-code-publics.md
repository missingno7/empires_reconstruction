# Historical code and library publics

The recovered C sources now name the actual public symbols already provided by
the pinned `CC.LIB` or by another recovered source owner. This removes every
ordinary EXTDEF rename from the Turbo Link 2.0 structural path.

The library side covers 35 references in 23 owners, including `memmove`,
`longjmp`, `setmem`, `movmem`, `srand`, `biostime`, `setjmp`, `farcoreleft`,
`getdisk`, `farfree`, `hardretn`, `hardresume`, `open`, `close`, `read`,
`lseek`, `rand`, `strcpy`, `free`, `toupper`, `ultoa`, and `memset`. TLINK now
resolves those names directly from the historical library. No library module
selection or placement changed.

Recovered cross-owner calls now likewise use the actual owner publics
`_f01ce`, `_f5593`, `_f6c57`, `_faf45`, and `_fd825`. Numeric bindings that
land exactly on a reconstructed function entry are represented uniformly as
`owner` plus `public`, rather than as absolute code offsets. This canonicalizes
254 manifest and recipe declarations without supplying a final address to the
linker.

After these changes the structural linker report contains zero EXTDEF rename,
case-normalization, runtime-DATA alias, or caller-scoped alias transforms. Its
only remaining code-symbol transform class is internal-entry PUBDEF injection
for 27 genuine entries in `RUNTIME_BLOCK`. All reconstructed function-entry
aliases were then replaced by their selected owner publics, and the final 27
publics are now emitted by canonical inline assembly. See
[the natural-public checkpoint](natural-runtime-publics.md).

Both independent checks remain exact:

```text
fixed reconstruction:       byte identical
Turbo Link 2.0 structural:  byte identical
unresolved symbols:         0
first code divergence:      none
SHA-256: 1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10
```

`tools/promote_library_publics.py` performs the repeatable source and binding
renames. `tools/canonicalize_code_bindings.py` converts exact entry-offset
bindings to relocatable owner/public references.
