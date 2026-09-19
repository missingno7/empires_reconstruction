# First relocatable TLINK experiment

The fixed-placement reconstruction remains the executable oracle. The new
`tools/probe_tlink_layout.py` path compiles the 339 proven post-startup C
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

The ordinary-owner run produced a detailed map and placed the startup plus the
first 339 reconstructed code owners at the expected load offsets without
explicit per-function addresses. Its first divergence was concrete:

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

The probe now also has an explicit `--promote-toupper` experiment. It removes
the ordinary `F_F9BE` object, renames the caller's temporary `_ff9be` EXTDEF to
the verified library public `_toupper`, and lets TLINK select `TOUPPER` from
`CC.LIB`. The promoted module is selected with the correct 49-byte extent, but
the current link places it at `0xF63E` while the oracle expects `0xF9BE`. The
code-owner prefix then remains exact; the earliest remaining mismatch is the
set of library modules that must be pulled before `TOUPPER` (the current
synthetic DATA/DGROUP and unresolved startup bindings do not yet provide all
historical references).

Adding the temporary historical-public demand object, together with the
verified `_delay` → `_f6c57` caller normalization, closes that library-order
experiment. TLINK then reports `_TEXT` length `0xFA23`, `_DATA` beginning at
`0xFA30`, and `TOUPPER` at `0xF9BE`; every supplied C-owner row remains at its
oracle offset. This is the first proof that the complete code/library prefix
can emerge from TLINK segment placement and library selection rather than
per-function address forcing.

The demand object is intentionally temporary. It asks for the publics already
evidenced by the fixed library manifest so linker placement can be tested
before the historical data/startup translation units are recovered. It does
not claim those publics are the final reconstructed source bindings.

The `--scaffold-dgroup` experiment now adds a temporary grouped OMF DATA/BSS
contribution. Its initialized tail is copied from the fixed oracle only to
hold the space not yet decoded into source; the linker still computes the
segment bases and stack placement. With the historical demand, recovered
symbol aliases, case normalization, and internal-label exposure enabled,
TLINK produces:

```text
_TEXT   0x00000 .. 0x0FA22   (0xFA23 bytes)
_DATA   0x0FA30 .. 0x1332B   (0x38FC bytes)
_BSS    0x13332 .. 0x1C4FB   (0x91CA bytes)
_STACK  0x1C500 .. 0x1C5E5   (0x00E6 bytes)
unresolved symbols: 0
first code divergence: none
```

The three numeric calls into library interiors are resolved to the exact
`HARDERR` and `OPEN` publics at those offsets, and C0C's `_main` entry is
exposed as an alias on the recovered `F_4A93` object. These are verified
linker adapters, not claims about historical translation-unit boundaries.
The initialized DATA tail and BSS tail remain explicitly temporary and are
the next ownership frontier. The machine-readable report is generated at
`build/tlink-structural-report.json` (ignored by Git), and the fixed
reconstruction path is unchanged.
