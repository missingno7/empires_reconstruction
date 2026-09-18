# Eighteenth local C wave

F_929E (268 bytes) and F_950C (306 bytes) reproduce complete functions using
the pinned Turbo C compiler. All 21 emitted fixups and complete code bytes
match. Neither function has initialized data or MZ relocation obligations.
Every external reference has an exact declaration in the pinned profile;
three references resolve through existing component publics.

F_929E accepts a two-byte structure by value, preserving separate signed-byte
accesses at BP+4 and BP+5. Its complete four-entry switch table is generated
by the compiler. Stack-local declaration order places the remainder-derived
coordinate at BP-4 and the quotient-derived coordinate at BP-2. Reversing the
declarations preserves code length but changes bytes; a fresh compiler control
must reject it.

F_950C preserves seven stack locals, SI/DI allocation and the predecrement
expressions in its call arguments. A fresh control changes predecrements to
postdecrements and must fail full comparison. Neither match substitutes raw
instructions for C expressions or copies a jump table from the original.

## Held byte-equal probe

F_4517 reproduces its complete 279-byte code extent using provisional numeric
bindings. Its storage bases DS:79BF, DS:74A2 and DS:8C12 have no exact entries
in the pinned profile, so it is not promoted. The first two bases participate
in loops with strides 793 and 187; the third is passed with length 2770.
Independent storage/call evidence is still needed. Byte equality with guessed
bindings alone is not credited as matching-C ownership.

Coverage is now 206 C functions / 22,719 bytes. Raw EXE coverage is 47,564
bytes across 64 regions, including 26,416 classified machine bytes and 21,148
unknown bytes. The complete EXE and both DAT archives remain exact. The goal
remains active; these fixed-placement components do not establish historical
modules, complete storage allocation or recovered linking.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave18.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave18-evidence.json.
Recipe: recipes/c/matching-wave18.json.
