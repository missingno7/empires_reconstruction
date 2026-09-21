# ASM Provenance Audit

Scope: the 32 `TASM.EXE` production modules in `layout/production-plan.json`, expanded per member region for multi-function modules. Data drawn from `docs/current/asm-origin-review.json`, module header comments (written this session, re-checked against the ASM text), `docs/current/timer-c-probes.json`, `docs/current/c-path-probes.json`, and `docs/current/exact-c-recovery.md`. Machine-readable form: `docs/current/asm-provenance.json`.

## Summary

| Classification | Count |
|---|---|
| PROBABLE_ORIGINAL_ASM | 18 |
| PROBABLE_MIXED_C_ASM | 9 |
| PROBABLE_ORIGINAL_C | 1 |
| RECONSTRUCTION_ONLY_ASM | 3 |
| HISTORICAL_LIBRARY_OR_RUNTIME | 1 |
| UNKNOWN | 0 |
| **Total modules** | **32** |

Member regions across all modules: 65.

**UNKNOWN modules: 0.** All six previously-UNKNOWN modules (`M_50D2_53BF`, `F_6181`, `M_6B1A_6B4A`, `F_9EC3`, `F_CAF1`, `M_D818_D825`) were resolved this session via probe evidence in `build/probes/rec2/notes.md`, `build/probes/rec3/notes.md`, `build/probes/rec4/notes.md`, and the relocation-topology mismatch audit (`docs/current/relocation-topology.json`, see `tools/audit_relocation_topology.py`).

**Remaining in-progress re-C:** only `M_DDD9_DF98` (`MUSIC.ASM`), and only its `F_DF98` member — `F_DDD9`/`F_DE7E`/`F_DEFA` are otherwise reconstructable and the module's native-Turbo-C origin is now confirmed via the relocation-topology mismatch (descending EXE relocations under a TASM production route).

## Modules

| Module | Source | Members | Bytes | Classification | Confidence | Re-C | Result |
|---|---|---|---|---|---|---|---|
| RUNTIME_BLOCK | asm/RUNTIME_BLOCK.ASM | entry-family (summarized by group) | 6571 | HISTORICAL_LIBRARY_OR_RUNTIME | HIGH | not attempted (out of scope) | Keep ASM |
| F_1ECD | asm/F_1ECD.ASM | F_1ECD | 74 | PROBABLE_ORIGINAL_ASM | HIGH | not attempted | Keep ASM |
| F_1F91 | asm/F_1F91.ASM | F_1F91 | 126 | PROBABLE_ORIGINAL_ASM | HIGH | not attempted | Keep ASM |
| F_4AA8 | asm/F_4AA8.ASM | F_4AA8 | 100 | PROBABLE_ORIGINAL_ASM | HIGH | not attempted | Keep ASM |
| F_4B0C | asm/F_4B0C.ASM | F_4B0C | 915 | RECONSTRUCTION_ONLY_ASM | MEDIUM | capsule symbolized only | Keep ASM (symbolic) |
| F_4E9F | asm/F_4E9F.ASM | F_4E9F | 76 | PROBABLE_ORIGINAL_ASM | HIGH | not attempted | Keep ASM |
| F_4EEB | src/F_4EEB.ASM | F_4EEB | 120 | PROBABLE_MIXED_C_ASM | MEDIUM | done historically (inline-asm C) | Keep ASM |
| M_50D2_53BF | asm/M_50D2_53BF.ASM | F_50D2, F_53BF | 312 | PROBABLE_MIXED_C_ASM | LOW | F_50D2 attempted (220/237 bytes); F_53BF not attempted | Keep ASM |
| F_6036 | asm/F_6036.ASM | F_6036 | 115 | PROBABLE_MIXED_C_ASM | MEDIUM | done historically (inline-asm C) | Keep ASM |
| F_60A9 | asm/F_60A9.ASM | F_60A9 | 216 | PROBABLE_ORIGINAL_ASM | HIGH | not attempted | Keep ASM |
| F_6181 | asm/F_6181.ASM | F_6181 | 171 | PROBABLE_ORIGINAL_ASM | HIGH | investigated, conclusively blocked | Keep ASM |
| M_6B1A_6B4A | asm/M_6B1A_6B4A.ASM | F_6B1A, F_6B4A | 76 | RECONSTRUCTION_ONLY_ASM | MEDIUM | both members already solved as inline-asm C; merge+promote pending | Keep ASM (merge candidate) |
| F_6BCF | asm/F_6BCF.ASM | F_6BCF | 87 | PROBABLE_ORIGINAL_C | HIGH | attempted, near-exact (2-byte gap) | Keep ASM |
| M_6D86_6DCC | asm/M_6D86_6DCC.ASM | F_6D86, PAD_006FC5 (CS state, not pad), F_6DCC | 377 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted | Keep ASM |
| F_6EFF | asm/F_6EFF.ASM | F_6EFF | 76 | PROBABLE_MIXED_C_ASM | MEDIUM | not attempted | Keep ASM |
| F_6F4B | asm/F_6F4B.ASM | F_6F4B | 120 | PROBABLE_MIXED_C_ASM | MEDIUM | not attempted | Keep ASM |
| F_9EC3 | asm/F_9EC3.ASM | F_9EC3 | 125 | PROBABLE_ORIGINAL_ASM | HIGH | attempted, 164 vs 125 bytes, conclusively blocked | Keep ASM |
| M_C1A0_C232 | asm/M_C1A0_C232.ASM | F_C1A0, F_C1F7, F_C232 | 221 | PROBABLE_MIXED_C_ASM | MEDIUM | F_C1F7 confirmed PROBABLE_ORIGINAL_ASM/HIGH; F_C1A0/F_C232 not attempted | Keep ASM |
| M_C27D_C567 | asm/M_C27D_C567.ASM | F_C27D, F_C2EA, F_C359, F_C3DB, F_C440, F_C501, F_C549, F_C567 | 797 | PROBABLE_MIXED_C_ASM | LOW | not attempted per-function | Keep ASM |
| M_C5A8_C5C6 | asm/M_C5A8_C5C6.ASM | F_C5A8, F_C5B3, F_C5C6 | 41 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted | Keep ASM |
| M_C5D1_C706 | asm/M_C5D1_C706.ASM | F_C5D1, F_C678, F_C6B9, F_C706 | 388 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted | Keep ASM |
| F_C755 | src/F_C755.ASM | F_C755 | 37 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted | Keep ASM |
| M_C77A_C898 | asm/M_C77A_C898.ASM | F_C77A, F_C7CB, F_C834, F_C877, F_C898 | 346 | PROBABLE_MIXED_C_ASM | MEDIUM | F_C77A/F_C7CB confirmed PROBABLE_ORIGINAL_ASM/HIGH; F_C898 known mixed (src/F_C898.C); F_C834/F_C877 not attempted | Keep ASM |
| F_C8D4 | src/F_C8D4.ASM | F_C8D4 | 14 | PROBABLE_ORIGINAL_ASM | HIGH | not attempted | Keep ASM |
| F_C914 | asm/F_C914.ASM | F_C914 | 116 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted | Keep ASM |
| M_C9A4_CA91 | asm/M_C9A4_CA91.ASM | F_C9A4, F_CA03, F_CA35, F_CA51, F_CA83, F_CA91 | 247 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted per-function | Keep ASM |
| F_CA9B | asm/F_CA9B.ASM | F_CA9B | 53 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted | Keep ASM |
| F_CAF1 | asm/F_CAF1.ASM | F_CAF1 | 87 | PROBABLE_ORIGINAL_ASM | HIGH | attempted (4 iterations), conclusively blocked | Keep ASM |
| M_D386_D3CF | asm/M_D386_D3CF.ASM | F_D386, F_D3CF | 84 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted | Keep ASM |
| M_D61C_D79C | asm/M_D61C_D79C.ASM | F_D61C, F_D79C | 507 | PROBABLE_ORIGINAL_ASM | MEDIUM | not attempted | Keep ASM |
| M_D818_D825 | asm/M_D818_D825.ASM | F_D818, F_D825 | 71 | PROBABLE_MIXED_C_ASM | MEDIUM | F_D818 13/13 bytes (register-choice gap only); F_D825 not attempted | Keep ASM |
| M_DDD9_DF98 (MUSIC.ASM) | asm/MUSIC.ASM | F_DDD9, F_DE7E, F_DEFA, F_DF98 | 700 | RECONSTRUCTION_ONLY_ASM | HIGH | native TC origin confirmed (relocation mismatch); blocked only on F_DF98 without inline asm | Keep ASM pending F_DF98 |

Full evidence bullets, reasons, C candidates, and next actions for every module are in `docs/current/asm-provenance.json`.

### Notable evidence highlights

- **F_6BCF** (timer interrupt) is the strongest `PROBABLE_ORIGINAL_C` case: `docs/current/timer-c-probes.json` shows an ordinary Turbo C interrupt function matches all 87 bytes except a 2-byte PUSHF/POPF discrepancy across two probe variants. A third probe forcing a bare `pushf`/`popf` pair is the recommended next experiment.
- **F_1ECD, F_1F91, F_4AA8, F_4E9F** carry documented negative compiler evidence in their own header comments (register-ABI shapes TC 2.0 does not emit: simultaneously-live AX/BX/CX/DX or AX/BX/CX/DI, BP repurposed as a data register, `loop`-driven counters preserved across calls with explicit push/pop). These stay `PROBABLE_ORIGINAL_ASM` at HIGH confidence.
- **F_4B0C** is `RECONSTRUCTION_ONLY_ASM`: production already replaced the raw `asm db` capsule with symbolic TASM (labels, branches, named calls); `src/F_4B0C.C` is now an inactive capsule-only reference (see capsules table).
- **F_60A9** contains positive object evidence that it calls into C: the fixup at file offset 6122h is a proven SEGMENT fixup loading DGROUP (`mov ax,DGROUP`), the DS-restore idiom an ASM caller performs before invoking a C function.
- **Sound cluster** (`M_C1A0_C232`, `M_C27D_C567`, `M_C5D1_C706`, `M_C77A_C898`, `M_C9A4_CA91`, plus `F_C755`, `F_C8D4`, `F_C914`, `F_CA9B`, `F_CAF1`) shares SI-relative and ES:DI register-ABI addressing into common DGROUP control-block offsets (e.g. `1764h..17F4h`, `1E84h..1E94h`, `17A4h`, `17FCh`). `F_C898` is confirmed mixed C+inline-asm via `src/F_C898.C`. `F_C1F7`, `F_C77A`, `F_C7CB`, and `F_CAF1` are now individually confirmed `PROBABLE_ORIGINAL_ASM`/HIGH this session (see `member_classifications` in the JSON for `M_C1A0_C232`/`M_C77A_C898`). Per-function follow-up is still owed for `M_C27D_C567`'s 8 entries, `M_C9A4_CA91`'s 5 operation handlers, and `F_C1A0`/`F_C232`/`F_C834`/`F_C877` individually — flagged as next actions rather than asserted individually.
- **RUNTIME_BLOCK** is summarized by entry-family group (startup/dispatch head with self-modifying CS patch; video-adapter probing; EGA/VGA graphics-mask data tables; unreferenced stub + `EGA.DRV` name literal) rather than per byte, per task instruction.
- **Relocation-topology audit** (`tools/audit_relocation_topology.py`, `docs/current/relocation-topology.json`): the EXE's 106 relocations are written by TLINK in FIXUPP order, object by object; Turbo C emits FIXUPP descending, TASM/`-B`/inline-asm units emit ascending. The one confirmed mismatch is `M_DDD9_DF98` (MUSIC.ASM): a TASM production route predicts ascending, but the EXE shows descending relocations — direct object-topology proof the original was a native Turbo C unit, independent of the register-ABI arguments above. All other checked modules (including `F_9EC3`, `C_75F3_7856`, `RELOC_F_AD25_F_ADCF`) agree with their route's prediction; a fresh compile session (`build/production-w3786tlv/compile/WORK`) independently confirms raw FIXUPP order for 33 objects with 0 disagreements.
- **F_6181** and **F_9EC3** are now `PROBABLE_ORIGINAL_ASM`/HIGH: F_6181's mid-function `push dx; mov bp,sp` re-basing (patching a pushed argument for reuse across two calls) and F_9EC3's `XLAT`-twice-plus-manual-DS-reload sequence are both categorically outside Turbo C -mc's code generation, confirmed by fresh probes this session (`build/probes/rec3`, `build/probes/rec4`).
- **M_6B1A_6B4A** moves to `RECONSTRUCTION_ONLY_ASM`: both members already have byte-exact inline-asm C reconstructions (`src/F_6B4A.C` in production; `F_6B1A`'s historical reconstruction recorded in `layout/manifest.json`), so what remains is packaging (merge into one C unit and promote), not origin research.

## Capsules (opaque instruction bytes)

| File | Lines | Bytes | Classification | Reason |
|---|---|---|---|---|
| src/DOS_STUB.C | 4-22 | 404 | LEGITIMATE_DATA | Literal MS-DOS EXE header stub (loader preamble), vendor/linker boilerplate that must be byte-exact by nature. Outside `production-plan.json` (lives in `manifest.json` as `startup-stub`); included for completeness. |
| src/F_4B0C.C | 4-15+ | 915 | INSTRUCTION_CAPSULE | Whole-function `asm db` capsule for the F_4B0C opcode interpreter. Already resolved in production: `asm/F_4B0C.ASM` replaced it with symbolic TASM; `src/F_4B0C.C` is now an inactive reference, excluded from production scope but recorded here as the capsule type the task asks to flag. |
| asm/RUNTIME_BLOCK.ASM | 5 | 1 | LEGITIMATE_DATA | `db 0e9h` — first byte of a near-jmp whose displacement is computed from instruction labels; single opcode byte, not an opaque sequence. |
| asm/RUNTIME_BLOCK.ASM | 10 | 1 | LEGITIMATE_DATA | `db 0e8h` — first byte of a near-call at the dispatch head, same rationale as above. |
| asm/RUNTIME_BLOCK.ASM | 3128-3143 | 256 | LEGITIMATE_DATA | 16x16-byte EGA/VGA plane-mask ramp table (`0FFh` row then descending `0Fh`-prefixed rows) — genuine binary graphics-mask data, no opcode structure. |
| asm/RUNTIME_BLOCK.ASM | 3502 | 1 | **UNKNOWN** | `db 0c3h` (RET opcode) immediately before `runtime_driver_name` / `db 'EGA.DRV',0`. Header calls it an "unreferenced stub" byte; no fixup, cross-reference, or public symbol targets it. Could be a leftover RET from a trimmed routine or a coincidental padding byte — not established either way. This is the task-specified 0x19A2 byte. |

**Capsule totals:** 6 items — 1 `INSTRUCTION_CAPSULE` (already resolved for F_4B0C in production), 4 `LEGITIMATE_DATA`, 1 `UNKNOWN` (the RUNTIME_BLOCK 0xC3 byte).

## Ranked next actions

1. **RUNTIME_BLOCK 0xC3 byte at line 3502** — determine whether it's a real leftover RET or coincidental padding; check surrounding object/relocation evidence more closely (currently the only `UNKNOWN` capsule finding).
2. **Close `M_DDD9_DF98` (MUSIC.ASM)**: express `F_DF98` without inline asm to finish the module; native TC origin is already confirmed (relocation-topology mismatch), so this is the last blocker for the whole module.
3. **F_6BCF third probe**: try an explicit bare `pushf`/`popf` wrapper around the native Turbo C interrupt body to close the last 2-byte gap and promote to production C.
4. **M_6B1A_6B4A merge-and-promote**: combine F_6B1A's historical inline-asm C reconstruction (`layout/manifest.json` id `F_6B1A`) with `src/F_6B4A.C` into one compiled unit matching the module's object layout.
5. **Remaining per-function sound-cluster follow-up**: `M_C27D_C567` (8 entries), `M_C9A4_CA91` (5 operation handlers), and the still-open `F_C1A0`/`F_C232` (in `M_C1A0_C232`) and `F_C834`/`F_C877` (in `M_C77A_C898`) — give each entry the same individual scrutiny already applied to `F_C1F7`/`F_C77A`/`F_C7CB`/`F_CAF1` this session.
6. **F_50D2/F_53BF** (`M_50D2_53BF`): F_50D2 is blocked on a single SI-addressed ROM-probe fragment (try wrapping just that fragment in inline asm); F_53BF still needs an actual byte-diff probe to confirm its `ds:`-override tell.
7. **F_D818/F_D825** (`M_D818_D825`): continue the F_D818 SI/DI register-allocation search (scoped/looped declaration variants not yet tried) and run F_D825 through `probe_module.py` using the same near-pointer idiom.
8. **M_6D86_6DCC**: F_6D86 alone has a clean C-calling-convention (BP-frame) shape; consider a standalone probe once F_6DCC's CS-addressed shared state is confirmed to force the pair to stay ASM regardless.
9. **Relocation-topology audit maintenance**: re-run `tools/audit_relocation_topology.py` whenever `layout/production-plan.json` or a module's production route changes, to catch new mismatches early.
