# Owned library public references

72 bindings across 49 callers now reference OMF publics in 26 already owned
Borland library modules. Eleven target nonzero offsets within their modules.
For example, `_farmalloc` is at offset 507 in `LIB_FMALLOC`, and `_memmove`
is at offset 84 in `LIB_MOVMEM`.

`tools/reconstruct.py` reads the pinned library, checks each module digest,
and resolves a reference by its owner and exact public name. Relative offsets
come from the OMF public table on every build; they are not copied into binding
declarations. The library modules still own their complete code contributions,
including private helpers before and between publics. Both near and far
references use this resolution path. Missing, duplicate, and out-of-range
publics fail rather than falling back to the old absolute address.

`python tools/recover_library_bindings.py` performs the migration. A declaration
is eligible only when its old code offset agrees with exactly one matching
owned public. The command freshly builds and compares the entire EXE before
publishing a changed manifest. `library-binding-evidence.json` records the
old addresses and observed OMF-relative offsets for auditing; the build does
not use this receipt to resolve references. Repeating the command is a no-op.

Together with 454 C/ASM entry references and eight structured-data references,
534 of 1,665 owner-symbol declarations now use component ownership. This is partial
symbol resolution within the fixed placement scaffold. It does not establish
historical library selection order, source module boundaries, or real linker
layout. Raw coverage is 32,641 EXE bytes, and all three
reconstructed game files remain byte-identical.
