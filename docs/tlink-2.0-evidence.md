# Turbo Link 2.0 evidence

Current full-scaffold checkpoint: Turbo Link 2.0 links without the historical
demand object, with zero errors/unresolved symbols and exact segment bases.
The linked bytes still differ (56/106 relocations; first load difference 0xC8).
See [the checkpoint](tlink20-full-scaffold.md) and [generated metrics](structural-status.json).


The pinned Turbo C 2.0 distribution contains `disk2/TLINK.EXE` in the local
`turboc20.zip` archive. Its DOS banner is:

```text
Turbo Link  Version 2.0  Copyright (c) 1987, 1988 Borland International
```

The extracted local binary hashes to
`997fcac6089fa88f3d868bdaf8bd65bd44c5aa83885a1d61e73f77808bd4f8f7`. It is
recorded in `layout/toolchain.json` and `docs/tlink-candidate.json`; the binary
remains local and ignored by Git.

With the existing recovered C objects, source-owned `STRLEN`/`RAND`, symbol
normalization, and no synthetic DGROUP object, Turbo Link 2.0 produces:

```text
_TEXT   00000..0FA22   0xFA23 bytes
_DATA   0FA30..0FCCF   0x0290 bytes before unresolved data is supplied
_BSS    0FCC6..0FD09
_STACK  0FD10..0FDF5
code rows: 342
first code-placement divergence: none
unresolved symbols: 424 (all remaining data/startup ownership)
```

The linker-selected module sequence is identical with and without the
historical-library demand object: 391 map rows and the same library order. This
removes `LIBDEMAND.OBJ` from the current code-prefix experiment while keeping
the demand mode available as a diagnostic comparison.

The linked partial executable is compared independently against the oracle in
the machine-readable structural report: MZ fields, relocation ordering, load
image, `_TEXT`, initialized DATA, first differing byte, and whole-file SHA-256.
It is expected to differ until reconstructed DATA/BSS sources replace the
temporary scaffold.
