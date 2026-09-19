# First relocatable TLINK experiment

The fixed-placement reconstruction remains the executable oracle. The new
`tools/probe_tlink_layout.py` path compiles the 340 proven post-startup C
owners with the pinned Turbo C toolchain, trims each OMF contribution to its
owned `_TEXT` extent, inserts classified code-gap padding as temporary
relocatable OMF contributions, and invokes a local Borland TLINK candidate.
No load address is written into these objects.

The local `C0C.OBJ` startup object is pinned in `layout/toolchain.json`. The
startup audit establishes that its complete 444-byte `_TEXT` contribution
matches the first 444 load-image bytes outside its own fixup locations. The
linker candidate is recorded separately in `docs/tlink-candidate.json` because
the available binary is Borland C++ TLINK 5.1 from a different local toolchain;
it is evidence for linker behavior, not yet a verified Turbo C 2.0 input.

The latest run produced a detailed map and placed the startup plus the first
339 reconstructed code owners at the expected load offsets without explicit
per-function addresses. The first divergence is now concrete:

```text
owner       expected      TLINK         length
F_F9BE      0xF9BE        0xE5EB        49 bytes
```

`F_F9BE` is the reconstructed `TOUPPER` body. The original places those bytes
inside the late library region, while supplying the recovered C body as an
ordinary object places it immediately before TLINK's selected runtime library
modules. This identifies library extraction/order as the next structural
problem. Before this point, the only divergence was the classified one-byte
`PAD_004CA7` gap; representing code-gap pads as relocatable contributions
removed that mismatch without hard-coding its address.

The map still has unresolved symbols because DGROUP, BSS and the historical
startup bindings have not yet been represented by synthetic relocatable data
objects. That is expected at this milestone. The machine-readable report is
generated at `build/tlink-structural-report.json` (ignored by Git), and the
fixed reconstruction path is unchanged.
