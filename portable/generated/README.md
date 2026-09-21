# portable/generated

`game_data.[ch]` and `game_state.[ch]` are **generated**. Do not hand-edit
them; edit `tools/portable/datagen.py` (or its historical-tree inputs) and
regenerate:

```
python tools/portable/datagen.py
```

This also refreshes `portable/generated/symbols.json` and
`docs/portable/state-map.md`. The generator never writes outside
`portable/generated/`, `docs/portable/state-map.md` and (on first run only,
via a one-time manual copy) `portable/data/`; it never touches the historical
tree (`src/`, `asm/`, `include/`, `recipes/`, `layout/`, `tools/*.py` other
than `tools/portable/`).

## What each file is

- **`game_data.[ch]`** -- one C definition per initialized DGROUP DATA object
  (`recipes/data/game-initialized.json`) that is not owned by a hand-written
  subsystem (`tools/portable/state_ownership.json`) and not part of a
  `compiled-data` component (those are reproduced by the matching ported
  `portable/game/*.c` file's own static initializer instead -- see
  `docs/portable/state-map.md`'s "Ported-C-owned DATA" table). Pointer32/
  offset16 fields become real C pointers to the target object, resolved by
  name through the merged symbol table (`&target` / `target` array decay /
  `&target.field` / `&target.field[i]`, whichever the target's own shape
  requires).
- **`game_state.[ch]`** -- one zero-initialized C definition per BSS object
  (`src/data/GAME_BSS.json` public, sized/typed from
  `layout/production-plan.json`'s `bss[*].typed_reserves` or from a
  `/* DS:XXXX */` extern comment in `src/*.C`/`include/*.H` when
  `typed_reserves` has nothing) that is not subsystem-owned. Nothing here
  needs an initializer: C zero-initializes static storage.
- **`symbols.json`** -- the full merged DGROUP symbol table (name ->
  offset/section/size/type/aliases/definition site), machine-readable.
- **`../../docs/portable/state-map.md`** -- the human-readable version, plus
  the list of symbols whose historical type could not be determined (they
  fall back to `uint8_t name[N]`) and the list of DATA bytes present in
  `assets/AEPROG.EXE` that are not modeled at all (Turbo C startup/runtime-
  library data, or pure alignment padding -- see "Toolchain/alignment bytes
  not modeled" in that file).

## Regenerating

`tools/portable/datagen.py` reuses the historical DATA encoders
(`tools/exe_data.py`, `tools/typed_data.py`, `tools/pointer_records.py`,
`tools/sound_data.py`, `tools/sound_instruments.py`) as pure decoders --
it never compiles anything and never invokes the pinned DOS toolchain, so it
runs from a plain Python 3 checkout. It needs `assets/AEPROG.EXE` (the
historical-tree ground truth for DATA bytes; not committed, see
`docs/portable/architecture.md`) and, for the two hardware palettes, either
`portable/data/PALETTE_DAC6_0{1,4}1E.json` (committed) or
`raw/exe-data/PALETTE_DAC6_0{1,4}1E.json` (gitignored historical-recovery
cache) as a fallback.

Run the verification suite after regenerating:

```
python -m unittest tools.portable.test_datagen -v
```
