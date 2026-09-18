# First local matching C wave

Ten functions now compile to 291 previously raw EXE bytes with Turbo C 2.0.
Every emitted byte, external fixup and relocation obligation is compared;
there are no inline assembly bodies, binary patches or excluded bytes in
these C owners.

| Owner | Bytes | Source construction |
|---|---:|---|
| F_1EA5 | 15 | Signed division of a masked global |
| F_49E3 | 13 | Four ordered calls |
| F_68CF | 143 | Corrected held C draft and explicit symbol bindings |
| F_697D | 19 | Library call with saved far pointer |
| F_7343 | 11 | Byte parameter stored to a global |
| F_7DFC | 11 | Call with a DGROUP array address |
| F_9D79 | 21 | Calls followed by a polling loop |
| F_AD0E | 23 | Byte field in a 27-byte record array |
| F_CB48 | 20 | Two stores and a call, with `-k` stack frame |
| F_D60C | 15 | Conditional call |

The seven-binding `F_68CF` draft had misleading aliases. `_f2` is a data
reference to DS:1772, not function offset 2. Data addresses are declared against
the upstream storage-object model and corroborated by already matching owners:
F_6BCF for 176E/1772 and F_3986 for 13ED/C470. Call targets are recorded function
entries; newly owned callees use component bindings. The two chained assignments
must be `f2 = f1 = ...` to emit stores to 176E followed by 1772. The reverse
order computes the same final values but does not match the original bytes.

The other nine sources were reconstructed directly from the observed instruction
sequences. Their global declarations use exact storage-object bases in the pinned
upstream model; function targets use recorded entries. `setvect` resolves through
the owned GETVECT library module's OMF public. The recipe records those binding
sources and upstream metadata hashes; none are inferred from expected operands
during a build.

The upstream F_D60C extent is 16 bytes. Fresh C emits 15 bytes through RET;
the following zero remains a separate raw byte. This avoids attributing an
unexplained byte to compiler output. F_CB48 requires `-k`; without it the emitted
routine is only 16 bytes instead of 20.

`recipes/c/matching-wave1.json` contains the explicit owners and build rules.
`python tools/promote_c_candidates.py` checks fresh compiler output for every
new owner before changing the canonical partition. It leaves upstream files
untouched and refuses conflicting existing ownership. The metadata-only receipt
is `c-matching-evidence.json`. Normal builds compile all sources afresh.

Regression tests compile all ten and two negative variants: reversed flag-store
order must differ, and omitting the required frame must fail extent length.
They also check that the trailing zero at D61B remains raw.

Current coverage is 136 matching C functions (14,433 bytes), 20 matching ASM
owners (2,456 bytes), and 55,950 raw bytes. Of that raw coverage, 34,674 bytes
are recorded unresolved machine extents and 21,276 remain unclassified.
The remaining held snapshot contains 26 candidates, including two byte-differing
C drafts. Historical module ownership and real linking remain unproven.
