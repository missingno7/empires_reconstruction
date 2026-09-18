# Third local C wave and compiled initializer ownership

Twelve functions now reproduce 821 previously raw code bytes. Another 15 bytes
are generated from a C initializer, reducing total raw coverage by 836 bytes.
All three game files still match exactly, including the EXE relocation table.

| Owner | Code bytes | Source pattern |
|---|---:|---|
| F_75F3 | 131 | Local array initializer and ordered calls |
| F_7747 | 68 | Register locals and a do/while input loop |
| F_A19D | 67 | Three constant-argument calls |
| F_A1E0 | 67 | Related constant-argument calls |
| F_A24E | 63 | Loop assigning 27-byte structures |
| F_A40A | 50 | Parameter forwarding and constant arguments |
| F_B3D7 | 56 | Calls using a global far pointer |
| F_B55E | 53 | Four compact-model library free calls |
| F_CE68 | 54 | Shift, assignment expression and record-field OR |
| F_D089 | 72 | Register-held flags, stores and a counting loop |
| F_D0D1 | 70 | Two register-held table lookups and calls |
| F_D117 | 70 | Related table lookups and calls |

## Closing F_75F3

The previous held draft copied an external structure into a local structure.
That produces the wrong register choices and push order around `SCOPY@`.
A local `char cap[15]` initialized by a string literal produces the exact
original code, including that far helper call and its MZ relocation. The
compiler also emits the exact 15-byte `_DATA` initializer.

Both contributions now have ownership. `F_75F3` owns the 131 code bytes;
`C_DATA_75F3` owns the initialized data at DS:0B87. The C owner's `_DATA`
binding names the data owner, so the address is derived from placement. The
builder reads the whole initialized segment from the same fresh OBJ, checks
its size and absence of fixups, and compares every byte. It cannot silently
discard an initializer edit while retaining old raw data.

The regression suite demonstrates this distinction: changing a character in
the literal leaves the compiled code equal, but changes emitted `_DATA` and
fails the data comparison. Additional controls reject mismatched source
ownership, wrong segment ownership, length changes, and unexpected relocations.
The first implementation supports complete relocation-free `_DATA` segments;
it does not claim general linking or original module recovery.

F_D089 also required an exact source form: initializing the register index,
then assigning it to the global, emits a direct register store. A chained
assignment introduces an extra AX move and produces 73 bytes rather than 72.

## Proof and remaining candidates

`recipes/c/matching-wave3.json` declares code and initialized-data ownership.
`matching-wave3-evidence.json` records fresh results. Run:

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave3.json
python tools/reconstruct_game.py
```

The F_7932 (50 bytes, DS:0BB4) and F_B967 (56 bytes, DS:B6CF) probes have
since been promoted after independent storage and caller evidence closed their
bindings. F_9D8E remains a byte-equal probe at DS:125D. That pointer lands in
the final 20 bytes of an adjacent raw owner rather than an identified data
component, so it remains local research under `build/c-wave3/`; equality of a
probe is not a linkage grant.

All three historical byte-differing C drafts are now resolved across the
three local waves. The historical held snapshot still has 24 linkage refusals;
new research candidates are separate from that snapshot. Current matching C
coverage is 161 functions / 16,174 bytes. Raw coverage is 54,194 bytes, including
32,933 recorded unresolved machine bytes and 21,261 unclassified bytes.
