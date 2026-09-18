# Sixteenth local C wave

F_1D47 (350 bytes) and F_B4FB (99 bytes) reproduce their complete function
extents from freshly compiled C. Every emitted byte and declared OMF fixup
matches; neither contribution has initialized data or MZ relocation sites.
All external addresses agree with exact declarations in the pinned upstream
profile. F_684A and F_656C resolve through already owned component publics.

F_1D47 constructs a far-pointer table from three loaded offset tables, retaining
the original SI/DI loop allocation, postincrement order and two-byte displacement.
It also reproduces the early-return branch and subsequent copy-call sequence.
F_B4FB expresses sixteen byte stores as explicit displacements from DS:40D4,
whose base is already declared. The stores occupy offsets 72..79 and 416..423.
No separate storage objects or unexplained input bytes are invented for these
interior addresses. Its initial call retains the observed 672-byte argument.
The upstream region extent is shorter than that argument; this match establishes
the original call and accesses, not a newly proven complete storage allocation.

Fresh compiler controls reject a loop-bound change and a one-byte displacement
change. Both complete routines pass full binding and byte comparison. This
adds 449 C bytes, bringing coverage to 203 functions / 21,853 bytes. Raw EXE
coverage is 48,430 bytes, including 27,282 classified machine bytes and 21,148
unknown bytes. EXE and both DAT files remain exact.

The matching-C goal remains open. Known routine boundaries, missing storage
observations and compiler instruction selection remain productive avenues.
Fixed placement and declared addresses remain scaffolding; these matches do
not establish historical module grouping, BSS layout or recovered linking.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave16.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave16-evidence.json.
Recipe: recipes/c/matching-wave16.json.
