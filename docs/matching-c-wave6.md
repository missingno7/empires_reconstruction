# Sixth local C wave

Six complete C routines reproduce 670 previously raw executable bytes.

| Owner | Bytes | Reconstructed source form |
|---|---:|---|
| F_6771 | 107 | Traverse variable-sized records and dispatch through two calls |
| F_67DC | 110 | Traverse word offsets through far pointers |
| F_7202 | 104 | Signed division, register loop and indexed resource calls |
| F_7298 | 123 | Indexed far-pointer calls and conditional update |
| F_A036 | 103 | Variable-argument pointer loop and pointer difference |
| F_D15D | 123 | Switch, embedded table and indexed selection loop |

All code, including F_D15D's compiler-generated switch table, is emitted by
fresh Turbo C 2.0 compilation. No inline assembly or byte injection is used.
The OMF binder handles the switch's internal code references and checks the
complete contribution. Local declaration order determines the stack slots in
F_67DC and F_A036. Moving the constant displacement into the base expression
in F_7298 gives the original instructions. Flat word-array indexing in F_D15D
avoids the extra far-pointer calculations generated for a multidimensional
array, giving the exact original table, branches and function length.

Regression controls change local declaration order and a switch case value;
both changed objects fail exact comparison. The complete unchanged candidates
are compiled and compared in every wave-six test. Six external references
resolve through existing owned C/ASM entry points, including F_8BA5,
F_A004, F_D0D1 and F_D117. Other bindings retain corroborated exact upstream
function or storage-object bases.

The ignored local F_B593 probe matches 124 code bytes but remains unpromoted:
its nine far-pointer globals at DS:C580/C584/C58A/C58E/C59A/C59E/C5A4/C5A8/C5AC
lack exact storage-object corroboration in the pinned profile. This remains a
binding-evidence task. F_9EC3 contains direct segment and string operations;
F_A09D depends on the previously held DS:C360 base. Neither is counted here.

Coverage is now 179 matching C functions / 17,767 bytes. Raw EXE coverage is
52,573 bytes, including 31,340 classified unresolved machine bytes. All three
game files still rebuild exactly. Fixed placement and incomplete module/linker
recovery remain explicit limitations; the matching-C goal is still active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave6.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

The recipe is `recipes/c/matching-wave6.json`; metadata-only fresh proof is
`matching-wave6-evidence.json`.
