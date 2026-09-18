# Fifth local C wave

Ten complete routines compile to 542 exact code bytes with Turbo C 2.0.
No inline assembly, code-byte injection, excluded instructions or raw fallback
is used inside these C contributions.

| Owner | Bytes | Recovered C form |
|---|---:|---|
| F_020F | 6 | Return a global word |
| F_6FC3 | 7 | Store zero |
| F_7856 | 109 | Far pointer assignment, conditional calls |
| F_791E | 7 | Store one |
| F_7D91 | 96 | Construct a packed local structure, pass its address |
| F_8BA5 | 6 | Return a global word |
| F_93AA | 88 | Far-pointer indexed offsets and shifted arguments |
| F_9440 | 38 | Select a 240-byte record and call F_778B |
| F_963E | 95 | Two arithmetic branches using register locals |
| F_9908 | 90 | Three calls, including a far-pointer argument |

F_7D91 requires two separate zero stores in the recovered source. Combining
those stores into a chained assignment changes the compiler's register usage,
ordering and length (94 bytes instead of 96). Regression controls reject that
rewrite and a changed branch threshold in F_963E. All ten unchanged functions
are freshly compiled and compared across their complete declared extents.

`recipes/c/matching-wave5.json` records bindings and provenance;
`matching-wave5-evidence.json` records byte counts and fresh hashes. External
bindings use existing declarations or exact bases in the pinned upstream
profile. F_9440's target resolves through the newly owned F_778B public.

## Remaining leads from this pass

Two further ignored local probes match their code but are not promoted:
F_21A9 (50 bytes) references DS:9CF2 and DS:A6B6; F_A13F (31 bytes)
references DS:C360. Those data bases lack the exact corroborating storage
objects used for this batch. This is a binding-evidence gap, not proof that
the routines cannot be C-matched.

The short recorded extents F_D45C and F_CDDD end at calls without their
following cleanup/return paths. They require boundary recovery before a
complete-function claim. F_034F, F_C8D4, F_6B7A, F_6BAC, F_4F63,
F_652A and F_6EFF contain direct interrupt, port, segment-register or string
instruction sequences. F_C5A8, F_C5B3, F_C5C6, F_CA35, F_CA91 and F_C549
consume live register inputs. These observations guide further investigation;
they do not establish that every possible historical C source form is ruled out.

Current coverage: 173 matching C functions / 17,097 bytes; 53,243 raw bytes
remain, including 32,010 classified unresolved machine bytes. The goal remains
active: many larger routines, held bindings and code boundaries still need work.
The build continues to use fixed placement; no original module or real-linker
recovery is claimed by these component matches.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave5.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```
