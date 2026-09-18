# Twelfth local C wave

F_250C (113 bytes) and F_5F3C (229 bytes) add 342 complete matching C bytes.

F_250C recovers assignment expressions, masks and conditional packed-byte
updates through a far pointer. F_5F3C reproduces three switches, signed local
byte shifts, coordinate adjustment and an indexed byte load. Its index uses
an explicit shift and subtraction, (y << 4) - y. Multiplication by 15 emits a
multiply instruction and changes the function length from 229 to 228 bytes.
The source preserves the observed expression rather than simplifying it.

Both routines use only ordinary C. Every emitted byte and OMF fixup is checked
across the complete function extent. Tests compile the unchanged sources and
reject both the multiplication rewrite and a changed low-bit mask. Bindings
retain established exact function or storage-object bases.

The inspected F_200F and F_21DB routines are further C-shaped candidates, but
reference several resource buffer bases that need explicit data/BSS evidence.
F_2269 shares the DS:96EE buffer reference. No C-impossibility claim is made;
these remain candidates for the data-ownership investigation.

Coverage is 195 matching C functions / 20,456 bytes. Raw EXE coverage is
49,827 bytes, including 28,654 classified unresolved machine bytes. All three
game files remain byte-identical. The matching-C goal remains active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave12.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Recipe: recipes/c/matching-wave12.json. Fresh metadata-only proof:
matching-wave12-evidence.json.
