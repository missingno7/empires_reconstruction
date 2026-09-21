# ASM Provenance Audit

Machine-readable form: `docs/current/asm-provenance.json`. Audit tool: `tools/audit_asm_provenance.py` (report mode by default; `--strict` for a failing exit code).

<!-- GENERATED: the tables below are regenerated from asm-provenance.json by tools/audit_asm_provenance.py. Do not hand-edit them; edit the JSON and re-run the audit instead. Narrative sections after the marker below are preserved by hand. -->

## Summary

Active production ASM modules: **11**, **12336** bytes.

| Classification | Modules |
|---|---|
| PROBABLE_ORIGINAL_ASM | 10 |
| HISTORICAL_LIBRARY_OR_RUNTIME | 1 |

Promoted from ASM history (no longer active production ASM): **4** -- F_6BCF, M_50D2_53BF, M_6B1A_6B4A, M_DDD9_DF98.

**UNKNOWN/PENDING_PROBE: 0 modules, 0 members.**

| Member classification | Count |
|---|---|
| PROBABLE_ORIGINAL_ASM | 61 |
| HISTORICAL_LIBRARY_OR_RUNTIME | 1 |
| PADDING | 1 |

## Modules

| Module | Source | Members | Bytes | Classification | Confidence | Production |
|---|---|---|---|---|---|---|
| RUNTIME_BLOCK | asm/RUNTIME_BLOCK.ASM | RUNTIME_BLOCK | 6571 | HISTORICAL_LIBRARY_OR_RUNTIME | HIGH | True |
| F_1ECD | asm/F_1ECD.ASM | F_1ECD | 74 | PROBABLE_ORIGINAL_ASM | HIGH | True |
| F_1F91 | asm/F_1F91.ASM | F_1F91 | 126 | PROBABLE_ORIGINAL_ASM | HIGH | True |
| M_4AA8_4EEB | asm/SPRITES.ASM | F_4AA8, F_4B0C, F_4E9F, F_4EEB | 1211 | PROBABLE_ORIGINAL_ASM | HIGH | True |
| M_50D2_53BF | src/STARTUP.C | F_50D2, F_53BF | 312 | PROBABLE_MIXED_C_ASM | HIGH | False |
| M_6036_6181 | asm/SPRDRAW.ASM | F_6036, F_60A9, F_6181 | 502 | PROBABLE_ORIGINAL_ASM | HIGH | True |
| M_6B1A_6B4A | recovery/asm/M_6B1A_6B4A.ASM | F_6B1A, F_6B4A | 76 | RECONSTRUCTION_ONLY_ASM | HIGH | False |
| F_6BCF | src/TIMERIRQ.C | F_6BCF | 87 | PROBABLE_MIXED_C_ASM | HIGH | False |
| M_6D86_6F4B | asm/DECODE.ASM | F_6D86, PAD_006FC5, F_6DCC, F_6EFF, F_6F4B | 573 | PROBABLE_ORIGINAL_ASM | HIGH | True |
| F_9EC3 | asm/F_9EC3.ASM | F_9EC3 | 125 | PROBABLE_ORIGINAL_ASM | HIGH | True |
| M_D386_D3CF | asm/M_D386_D3CF.ASM | F_D386, F_D3CF | 84 | PROBABLE_ORIGINAL_ASM | MEDIUM | True |
| M_D61C_D79C | asm/M_D61C_D79C.ASM | F_D61C, F_D79C | 507 | PROBABLE_ORIGINAL_ASM | MEDIUM | True |
| M_D818_D825 | asm/M_D818_D825.ASM | F_D818, F_D825 | 71 | PROBABLE_ORIGINAL_ASM | HIGH | True |
| M_DDD9_DF98 | recovery/asm/MUSIC.ASM | F_DDD9, F_DE7E, F_DEFA, F_DF98 | 700 | RECONSTRUCTION_ONLY_ASM | HIGH | False |
| M_C1A0_CB48 | asm/SOUND.ASM | F_C1A0, F_C1F7, F_C232, F_C27D, F_C2EA, F_C359, F_C3DB, F_C440, F_C501, F_C549, F_C567, F_C59A, F_C5A8, F_C5B3, F_C5C6, F_C5D1, F_C678, F_C6B9, F_C706, F_C755, F_C77A, F_C7CB, F_C834, F_C877, F_C898, F_C8D4, F_C8E2, F_C914, F_C988, F_C9A4, F_CA03, F_CA35, F_CA51, F_CA83, F_CA91, F_CA9B, F_CAD0, F_CADB, F_CAE6, F_CAF1, F_CB48 | 2492 | PROBABLE_ORIGINAL_ASM | HIGH | True |

## Member classifications

| Module | Member | Classification | Confidence |
|---|---|---|---|
| RUNTIME_BLOCK | RUNTIME_BLOCK | HISTORICAL_LIBRARY_OR_RUNTIME | HIGH |
| F_1ECD | F_1ECD | PROBABLE_ORIGINAL_ASM | HIGH |
| F_1F91 | F_1F91 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_4AA8_4EEB | F_4AA8 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_4AA8_4EEB | F_4B0C | PROBABLE_ORIGINAL_ASM | HIGH |
| M_4AA8_4EEB | F_4E9F | PROBABLE_ORIGINAL_ASM | HIGH |
| M_4AA8_4EEB | F_4EEB | PROBABLE_ORIGINAL_ASM | HIGH |
| M_50D2_53BF | F_50D2 | MATCHING_C | HIGH |
| M_50D2_53BF | F_53BF | MATCHING_C | HIGH |
| M_6036_6181 | F_6036 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_6036_6181 | F_60A9 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_6036_6181 | F_6181 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_6B1A_6B4A | F_6B1A | MATCHING_C | HIGH |
| M_6B1A_6B4A | F_6B4A | MATCHING_C | HIGH |
| F_6BCF | F_6BCF | MATCHING_C | HIGH |
| M_6D86_6F4B | F_6D86 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_6D86_6F4B | PAD_006FC5 | PADDING | HIGH |
| M_6D86_6F4B | F_6DCC | PROBABLE_ORIGINAL_ASM | HIGH |
| M_6D86_6F4B | F_6EFF | PROBABLE_ORIGINAL_ASM | HIGH |
| M_6D86_6F4B | F_6F4B | PROBABLE_ORIGINAL_ASM | HIGH |
| F_9EC3 | F_9EC3 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_D386_D3CF | F_D386 | PROBABLE_ORIGINAL_ASM | MEDIUM |
| M_D386_D3CF | F_D3CF | PROBABLE_ORIGINAL_ASM | MEDIUM |
| M_D61C_D79C | F_D61C | PROBABLE_ORIGINAL_ASM | MEDIUM |
| M_D61C_D79C | F_D79C | PROBABLE_ORIGINAL_ASM | MEDIUM |
| M_D818_D825 | F_D818 | PROBABLE_ORIGINAL_ASM | MEDIUM |
| M_D818_D825 | F_D825 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_DDD9_DF98 | F_DDD9 | MATCHING_C | HIGH |
| M_DDD9_DF98 | F_DE7E | MATCHING_C | HIGH |
| M_DDD9_DF98 | F_DEFA | MATCHING_C | HIGH |
| M_DDD9_DF98 | F_DF98 | MATCHING_C | HIGH |
| M_C1A0_CB48 | F_C1A0 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C1F7 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C232 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C27D | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C2EA | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C359 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C3DB | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C440 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C501 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C549 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C567 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C59A | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C5A8 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C5B3 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C5C6 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C5D1 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C678 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C6B9 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C706 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C755 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C77A | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C7CB | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C834 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C877 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C898 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C8D4 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C8E2 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C914 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C988 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_C9A4 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CA03 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CA35 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CA51 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CA83 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CA91 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CA9B | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CAD0 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CADB | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CAE6 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CAF1 | PROBABLE_ORIGINAL_ASM | HIGH |
| M_C1A0_CB48 | F_CB48 | PROBABLE_ORIGINAL_ASM | HIGH |


## Notable evidence highlights

Current as of 2026-09-21 (historical-source closure, TU wave).  Rules and the
translation-unit evidence live in `docs/current/tu-structure.md`; the remaining
work in `docs/current/closure-frontier.md`.

- **asm/SOUND.ASM (M_C1A0_CB48)** is one hand-written module: its framed
  routines use SI/DI without saving them (Turbo C always emits `push si`/`push di`
  around inline asm naming them -- probed: the C-unit hypothesis is 6 bytes
  longer), register-ABI helpers save AX/CX/DX/BX inside frames, the OPL write
  uses a REPT macro, and all in-cluster calls are fixup-free relative calls.  The
  seven former `-k`/dummy-parameter C wrappers (SNDSCL4, SNDTICK, SNDNOTE, SPKON,
  SPKOFF, PITDIV2, SNDSTOP) were reconstruction artifacts; OPTIONS.C grows under
  `-k`, so no `-k` unit existed.
- **Promoted to C this wave:** F_50D2/F_53BF (video adapter and sound-hardware
  probes, `src/STARTUP.C`) with pseudo-registers and `__int__`/`__inportb__`/
  `__outportb__`; the NOP-padded byte stores there are TASM sizing a forward EXTRN
  in a TCC-generated unit, which is positive evidence of inline asm inside a .C
  file and explains why CMDLINE and VIDMODE needed `-B`.
- **Hand-written graphics modules** (F_4AA8..F_4EEB sprite driver, F_6036..F_6181
  sprite/tile draws, M_6D86_6DCC..F_6F4B decoders, F_1ECD/F_1F91/F_9EC3 blit
  helpers, M_D386_D3CF, M_D61C_D79C, M_D818_D825 draw queue): LODS/STOS/XLAT/LOOP
  bodies, BP repurposed (`push bp` after the frame, `mov bp,sp` re-basing to patch
  pushed arguments), `push di; push si` hand order, word-aligned segments with
  TLINK pads.  Every call target is symbolic; `call $+delta` survives nowhere
  outside the runtime block (which now has labels for all 80 relative branches).
- **RUNTIME_BLOCK** remains the EGA/library runtime (6571 bytes, assembler
  measured: 0 raw unresolved bytes).

## Capsules (opaque instruction bytes)

None in production (`tools/report_source_quality.py`: ASM_DB_CAPSULE 0 bytes).
The historical capsule references (recovery/src/F_4B0C.C, recovery/src/DOS_STUB.C)
are inactive.  The single `db 0c3h` before `EGA.DRV` in asm/RUNTIME_BLOCK.ASM is
an unreferenced byte with no fixup or public; it is recorded as data.

## Ranked next actions

1. Name the sound driver's remaining LOW-confidence state words (docs/current/sound-state.md) and the 15 address-named functions that only have mechanical descriptions.
2. Unify the last data aliases (g1760/g1764 vs snd_seg/snd_seg2, g1774/g1776 vs mus_flag/snd_flag2) and extend include/SOUND.H.
3. Keep `tools/audit_tu_flags.py --strict` at 0 and re-run `tools/audit_relocation_topology.py` after any module change.
