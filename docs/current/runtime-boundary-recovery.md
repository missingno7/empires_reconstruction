# Runtime boundary recovery handover

Coordinates below are owner-relative and half-open; historical CS = offset + 039Ch.
The frozen byte hash remains authoritative. `tools/runtime_boundaries.py` derives
and validates the evidence stored in `recipes/runtime/oracle.json`; CFG generation
rejects disagreement between those two sources.

## Typed data

| Range | Structure | Consumers |
| --- | --- | --- |
| 003C–0046 | Five absolute CS mode dispatch words; zero slots unsupported | 019C |
| 0046–0050 | Five absolute CS blit dispatch words; zero slots unsupported | 02A2 |
| 0050–00F2 | 81 signed entry deltas, relative to 07C5; 15-byte stride | 02F8, 02FD, 0301, 0305, 07C2 |
| 00F2–0194 | 81 signed entry deltas, relative to 0F3F; 21-byte stride | 087B, 0880, 089A, 089D, 0F3C |
| 1570–1582 | Nine absolute CS compositor entries | 148F, 18B3 |
| 1582–1594 | Nine absolute CS compositor entries | 1503 |
| 15FC–16FC | 256 byte transparency masks | BX=1998h and CS XLAT at 17AB, 1844 |

These are addresses, relative entry/branch displacements, and byte masks, not
unexplained dimensions or strides. The strides describe repeated instruction
bodies. Mask[v] preserves the high nibble when v's high nibble is zero and the
low nibble when v's low nibble is zero. Every possible XLAT index is checked.
The ASM uses named DW tables and explicit historical CS constants, preserving
its zero-fixup contract. Raw code was only reflowed at proven instruction boundaries;
no grinder transcription was performed.

## Former 0307–0F3F blob

| Range | Ownership and behavior |
| --- | --- |
| 0307–07B7 | 80 unrolled 15-byte transfers; table-derived entries; fallthrough |
| 07B7–07C2 | Row strides, loop to 07C2, register restore and return |
| 07C2–07C5 | Mutable near jump; operand written at 02FD |
| 07C5–083B | Dispatch slot 3: ordinary copy routine, loops, return |
| 083B–089F | Dispatch slot 4: setup, table read and computed JMP AX |
| 089F–0F2F | 80 unrolled 21-byte transforms; table-derived entries; fallthrough |
| 0F2F–0F3C | Row strides, loop to 0F3C, register restore and return |
| 0F3C–0F3F | Mutable near jump; operand written at 0880 |

The old 15F0 boundary split the instruction beginning at 15EF. Code continues
through RET at 15FB. The lookup table begins at 15FC, and the next public routine
begins at 16FC. No padding is inferred from plausible instruction decoding.
Detailed roots, edges, exits, returns, dependencies and evidence are recorded in
`runtime-cfg.json` under `typed_tables` and `boundary_regions`.

## Deliberately unresolved

The display selector may be 0..5, but F_490D calls F_48BE before F_0281:
selectors 2 and 5 replace the built-in runtime at CS:039C with AE000 resources
3 and 2. Built-in dispatch normally sees selectors 1, 3, and 4. Its zero and
out-of-table slots are therefore not evidence that the normal program jumps
into data. Arbitrary direct invocation still has no local guard, and these
sites retain incomplete status with explicit execution-context evidence.

The two computed-entry width calculations still lack a locally proven 0..80
bound. Their stored targets and patched loop-back targets remain recorded
without assuming every possible argument is safe. Exact source recovery of
reachable code does not claim behavioral correctness for arbitrary inputs.

All three compositor dispatchers now have complete locally derived index sets.
At 141E/1421, CL is loaded from a byte and CH is cleared; 145E/1460 produces an
even BX in 0..510. Subtracting 16 for full blocks leaves an even remainder in
0..14 at 148F/1503. The shared path from 18B3 uses a byte multiplied by four,
so it preserves that domain too. Tests enumerate every possible byte input.

The copied table at 191C�192E is now typed data: all nine entries equal the
1570 table plus 0424h, matching the duplicated code displacement. It has no
proven consumer. The string at 19A3�19AB is typed as EGA.DRV plus NUL.
Only the C3 byte at 19A2 remains SUPERVISOR; no entry root is invented.

## Grinder workflow

The real checker is `tools/check_candidate.py`; refresh creates the referenced
`docs/current/candidates/*.json` and verifies those paths exist. Select work with
`python tools/reconstruction_factory.py next`, transcribe only the selected
card's source lines, then run its FAST and promotion commands. An unchanged card
correctly fails the quality-improvement gate. Promotion requires a fresh full
acceptance build. Preserve all untracked factory tooling and cards when handing
this working tree to the next worker; they already existed in this checkout.

## Final handover audit

Run `python tools/audit_grinder_queue.py` before assigning work. It reassembles
runtime bytes, regenerates CFG and candidates, verifies current acceptance and
fingerprints, compares every card and queue entry, and requires the cards to
partition all raw bytes exactly. Consumable cards must contain whole decoded
instructions and no incomplete indirect site.

Dispatcher 18B3 now has a complete local target set: 1889 zeroes BX, 188D loads
one byte, and 1899/189B multiply by four. The count is 0..1020 without wrapping.
The block loop subtracts 16, so the BX<16 dispatch path can index only bytes
0, 4, 8, and 12 of the existing even-pixel table. All 256 input byte values are
checked. The 148F/1503 sites are also bounded by the byte-count proof above; the computed-entry width sites retain their unresolved assumptions.

The unused copied table and driver string are recovered data. The intervening
C3 at 19A2 is an unreferenced RET candidate, not a proven routine or padding.
A scan of every text byte offset found no direct branch/call encoding targeting
CS:1D3E; the complete original EXE contains no little-endian 1D3E word either.
Computed references remain possible. See asm-capsule-audit.json for the audit.
