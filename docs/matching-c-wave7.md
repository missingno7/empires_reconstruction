# Seventh local C wave

Five routines reproduce 756 previously raw code bytes. F_A28D's two static
C strings reproduce another 16 bytes, removing 772 bytes of raw fallback.

| Owner | Code bytes | Source form |
|---|---:|---|
| F_7162 | 153 | Decrement, conditional return and embedded switch table |
| F_738A | 141 | Signed byte conversion, register arithmetic and calls |
| F_A28D | 178 | Packed record access, string selection and conversion |
| F_B09A | 136 | Ordered calls and two global stores |
| F_D1D8 | 148 | Descending loop with arithmetic call arguments |

F_A28D initially lacked declared bases for DS:1650 and DS:1659. The bytes are
two null-terminated strings. Declaring them as static initialized C arrays
emits the entire original 16-byte _DATA contribution. C_DATA_A28D owns that
complete compiler segment at DS:1650, and the code references it through the
existing component-based module-segment binding. No raw string bytes are used
by the build. This establishes a valid reconstructed component arrangement,
not the historical source file boundary.

Explicit pointer casts on each conditional-expression branch reproduce the
original segment-register loads. The uncast expression shares a segment load
and produces different instructions. Local declaration order also matters for
the stack slots. The integer conversion call resolves to the verified _ultoa
public at offset 176 in LIB_LTOA; it is not treated as an unknown numeric call.

The initialized-data tests now include all five routines and the new data
owner. A same-length static-string mutation leaves code equal but fails the
initialized-data comparison, proving that edits cannot hide behind old bytes.
Fresh complete-function comparisons include the switch table and all fixups.

Matching C now covers 184 functions / 18,523 bytes, with 59 bytes of
compiler-generated data. Raw EXE coverage is 51,801 bytes: 30,584 classified
unresolved machine bytes and 21,217 unclassified bytes. All three game files
remain byte-identical. The broader matching-C and real-linking goals remain
unfinished.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave7.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

The recipe and metadata-only results are `recipes/c/matching-wave7.json` and
`matching-wave7-evidence.json`.
