# SOUND.ASM state field map (DS:175E..1834, DS:1E84..1E96)

Sources cross-checked: `asm/SOUND.ASM` (equates, extern list, comments),
`src/*.C` + `include/*.H` (extern declarations and call sites),
`recipes/c/*.json` (DGROUP-offset bindings for the SOUND.ASM owners),
`docs/current/interface-conflicts.json` (the project's own global-symbol/
alias/storage-evidence registry, which already tracks most of these words),
and `layout/manifest.json` (DGROUP frame base and the full `DGROUP_offset`
binding set used across *every* code region in the binary, checked
exhaustively for a data public at each address in this report).

**2026-09-21 sound2 pass:** closed out the remaining LOW-confidence words
(17C4/17CC/17D4/17DC/17F4/1E8C), the 182C/1832 internal tables, and the
no-data-symbol equates (voice_pending_table, voice_retune_base_table,
voice_table_b).  Found and fixed one real bug in `asm/SOUND.ASM`:
`state_active equ 177ch` was an undocumented fourth alias for the same word
`_v_b` already names (confirmed via `layout/manifest.json`'s
`DGROUP_offset` bindings, which list only `_v_b` at DGROUP 6012/0x177C) --
the file's own comment had claimed it had no corresponding extern.  Fixed in
place; `_sound_control_block_advance` (F_C440) now reads `[si+_v_b]`
directly.  Also promoted two other raw hex displacements that already had a
data public elsewhere in the file to their extrn (`[si+17bch]` -> `_v_len`,
`[si+1794h]` -> `_voice_stream_base_table`).  `python
tools/probe_module.py M_C1A0_CB48` stayed EXACT throughout (2492/2492 bytes;
fixups grew 191 -> 194 as raw constant-offset equates were replaced by
extrn-backed references, which is expected). No new C-facing fields were
added to `include/SOUND.H`: none of the words in this pass's scope are
touched by any `src/*.C` translation unit. See
`build/probes/sound2/CANDIDATES.md` for the proposed new data publics this
pass identified (none applied here -- they need a supervisor's recipes/data
edit).

## Two clusters, two different consumers

- **DS:175E..1834** — the primary sound-effects driver's state (this is
  what `asm/SOUND.ASM` calls "voices", up to 4 of them: PC-speaker /
  Tandy-gate / queued-OPL-bank output selected by the backend-mode word at
  DS:1778). This is a *different* voice concept from the 9-voice OPL music
  bank in `src/OPLREG.C`/`OPLVOICE.C`; the two only meet when backend mode 2
  pushes a queued value into `voice_bank_retune_on`/`voice_bank_update`
  (bank 0..2, i.e. 3 real OPL voices per queued "bank").
- **DS:1E84..1E96** — the secondary "music stream" state consumed only by
  `_sound_tick_step`/`_sound_stream_command_step` (`F_C8E2`/`F_C914`) and
  its four helper ops (`F_C9A4`/`F_CA03`/`F_CA35`/`F_CA51`/`F_CA83`/`F_CA91`/
  `F_CA9B`). It is the single-stream twin of the 175E cluster: `_snd_delay`
  is the scalar analogue of `_v_ctr`, `_snd_one` of `_v_hold`, `_snd_len`
  (1E8E) of `_v_len` (17BC), `_mus_ptr`/`_mus_arg`/`_mus_flag` are the
  cursor/argument/continue-flag for that one stream.

## Structural finding: a 14-table struct-of-arrays block at 178C..17FB

The offsets between every named/equated field from `dv`(178C) through
`g17f4`(17F4) are exactly 8 bytes apart, and 17F4+8=0x17FC is exactly where
`_notetab` starts. That means this whole span is **14 parallel 4-word
tables** (one word per voice, voices 0..3 — matching the observed
`_snd_nvoices` ceiling of 4), stride 2, SI = voice*2:

```
178C dv/g178c        179C v_ctr/g179c     17AC v_a/g17ac      17BC v_len/g17bc
1794 sv               17A4 g17a4          17B4 v_hold/g17b4   17C4 state_cursor
17CC state_cursor_next 17D4 state_cursor_mode 17DC state_text 17E4 state_mode
17EC g17ec            17F4 g17f4
```

`v_b` (177C) is the same shape (4 words, 177C..1783) but sits just before
the scalars `snd_mode`(1784)/`snd_hi`(1786)/`g1788`/`g178a` that precede this
block.

## Field map

Confidence: HIGH = multiple independent, unambiguous sources agree.
MEDIUM-HIGH = one strong mechanical/behavioural chain, no contradicting
evidence. MEDIUM = plausible single-routine evidence, some uncertainty.
LOW = mechanism is clear, exact meaning is not — no name proposed.

| Addr | Current name(s) / aliases | Width | Readers / writers | What the code shows | Proposed name | Conf. |
|---|---|---|---|---|---|---|
| 175E | `_g175e` (extern, unsigned) | word | W: `player_record_load_publish` (PLRLDPUB.C, `g175e=gc5de`). R: `_stream_control_block_arm` (F_CAF1): `di=stream_id*2+g175e; ax=es:[di]; ax+=g175e` | Offset half of the loaded sound-resource far pointer (paired with `snd_seg`=1760, which is the segment half of the *same* `gc5de/gc5e0` far pointer from `resource_load_record_alloc`). Also doubles as the base that a table of relative-offset entries inside that resource block is added to. | `snd_base` | MEDIUM-HIGH |
| 1760 | `_g1760` / `snd_seg` (extern) | word | W: PLRLDPUB.C. R: `_sound_tick_step` (`es=_snd_seg`), `_stream_control_block_arm` (`es=_snd_seg`) | Segment of the loaded sound-resource block. Already well-named. | — (keep `snd_seg`) | HIGH |
| 1762 | `_g1762` (extern, unsigned, PLRLDPUB.C); **raw `[1762h]` in SOUND.ASM, no extern used there** | word | W: PLRLDPUB.C (`g1762=(unsigned)gc5da`). R: `_sound_voice_table_reload` (F_C7CB), raw displacement, twice | Offset of the 0x620-byte staging/scratch block (paired with `snd_seg2`=1764, same `gc5da` malloc'd pointer). Same "offset added to a per-voice table entry" pattern as 175E/snd_seg. | `snd_base2` | MEDIUM-HIGH |
| 1764 | `_g1764` / `snd_seg2` (extern) | word | W: PLRLDPUB.C. R: `_sound_tick_entry` (`es=_snd_seg2`), F_C7CB (`es=_snd_seg2`) | Segment of the staging block. Already well-named. | — (keep `snd_seg2`) | HIGH |
| 1766 | equate `voice_retune_base_table` (all sites now go through it; no raw displacement remains) | word ×4 (voice 0..3) | W: `_sound_pit_divisor_program` (F_C706, backend_mode==2 "store_queued" path, `[si+voice_retune_base_table]=ax`, the divisor that would otherwise go to PIT port 42h). R: `_voice_enable` (F_C678, backend_mode==2 "queue_value" path), pushed as the `base` argument to `voice_bank_retune_on(bank,base)` in OPLVOICE.C | Per-voice queued PIT-divisor/tone value awaiting delivery to the 9-voice OPL bank via `voice_bank_retune_on`. No prime/reload writer; holds whatever the last F_C706 call stored until the matching F_C678 call drains it. Confirmed no data public exists anywhere in `layout/manifest.json`'s `DGROUP_offset` bindings at 5990/0x1766 (exhaustive scan across every code region in the binary, not just this file). | `voice_retune_base_table` | MEDIUM-HIGH |
| 176E | `_f1` (SNDFXTGL.C) / `_sound_enabled` | word | W/R: OPTIONS.C, TIMER.C, PLAYERSL.C, SLOTMENU.C, SNDFXTGL.C | Master sound-effects on/off (options menu + timer gate). Already well-named; `f1` is a local alias in one TU only. | — (keep `sound_enabled`) | HIGH |
| 1770 | `_g1770` / `_snd_flag` (F_C1A0) / `_snd_on` (F_C8E2, F_CAF1, F_CB48) | word | R: `_sound_tick_entry` as `snd_flag`. R/W: `_sound_tick_step`, `_stream_control_block_arm`, `_sound_stop_reset` as `snd_on`. C: INTRO.C sets it; ROUNDEND.C busy-waits `while(g1770);` after `stream_control_block_arm(9)` | **Confirmed same word, three names**, all inside SOUND.ASM/its immediate C callers (`docs/current/interface-conflicts.json` groups them under one `storage_evidence`/alias set). Semantics: "a stream is currently armed/playing" — set to 2 by `stream_control_block_arm`, cleared by `sound_stop_reset`, gates whether `sound_tick_step` even runs. `roundend_wait`'s busy-wait proves this is a genuine playing-flag, not two coincidentally-overlapping concepts. | `snd_on` (retire `snd_flag`) | HIGH (conflict) / MEDIUM-HIGH (name) |
| 1772 | `_f2` (SNDFXTGL.C) / `_music_enabled`; **also raw `[1772h]` in F_C7CB, no extern used there** | word | W/R: OPTIONS.C, TIMER.C, PLAYERSL.C, SLOTMENU.C, SNDFXTGL.C. R (raw): `_sound_voice_table_reload` gates its whole body on `[1772h]!=0` | Master music on/off; F_C7CB's raw check ("only reload the voice table while music is enabled") is a second, unnamed consumer of the same flag. | — (keep `music_enabled`; promote the raw displacement to `_music_enabled`) | HIGH |
| 1774 | `_g1774` / `_mus_flag` | word | W: INTRO.C (brackets `stream_control_block_arm` calls). R/W: `_sound_stream_command_step` (F_C914, command 0xF: continue-chain vs `sound_stop_reset`) | "Keep auto-chaining the music stream" flag. Already well-named. | — (keep `mus_flag`) | HIGH |
| 1776 | `_g1776` / `_snd_flag2` | word | R: `_sound_voice_pump_loop` (gates the `sv`→`dv` freeze copy). Also read/written as plain `g1776` in GAME.C, INTRO.C, LEVEL.C, PLAYERSL.C in contexts that don't obviously involve sound | Confirmed alias by the project's own registry, but the non-sound C call sites are worth a second look before treating `snd_flag2` as the *only* correct reading — flagging for human review rather than asserting more than the evidence shows. | — (keep `snd_flag2`, but see caveat) | MEDIUM |
| 1778 | `_g1778` (RESCLOAD.C: "the mode"; STARTUP.C `-SI/-ST/-SA` command-line flags; `sound_backend_probe`) / `_snd_paused` (F_C1A0, F_C27D, F_C834) | word | See below | **Confirmed same word** — `docs/current/interface-conflicts.json` cites the *identical* `storage_objects/G_P212A8.phys` evidence id for both names across 7 independent callers. C-side evidence is unambiguous: `cmdline_parse_args` sets it from `-SI`(0)/`-ST`(1)/`-SA`(2); `sound_backend_probe` sets 0/1/2/3 from `display_mode`/`opl_detect`/a port probe; `_sound_backend_select_init`, `_sound_control_value_select`, `_voice_enable/_disable`, `_sound_pit_divisor_program`, `_sound_voice_table_reload`, `_sound_voices_disable_all` all branch on 0/1/2/3 as an output-backend selector (0=PC speaker, 1=Tandy/PCjr-style gate, 2=queued OPL bank, 3=a one-shot probe that immediately collapses back into 1). **Caveat:** the ASM's own label at `_sound_control_value_select`'s value==1 branch is named `C359_paused_mode`, and `F_C834`/`F_C877` both cap voice count to 4 by comparing this word to 2 — one routine spells it `_snd_paused`, the twin routine `_g1778`. This looks like the driver historically overloaded "which backend" with "reduced/paused service", so the *conflict* is HIGH confidence but the *final* name deserves a human look before commit. | `snd_backend_mode` (retire `snd_paused`) | HIGH (conflict) / MEDIUM-HIGH (name, with caveat) |
| 177A | `_g177a` / `_snd_nvoices` | word | W: `_sound_backend_select_init` (1 or 4). R: loop bounds in F_C232, F_C27D, F_C7CB, F_C877 | Active voice count for this cluster (1 or 4, never the 9 of the OPL bank). Already well-named. | — (keep `snd_nvoices`) | HIGH |
| 177C | `_v_b`; **was also a fifth alias `state_active` in F_C440, fixed this pass** | word ×4 | W: `_sound_tick_entry` (zeroes all 4 every tick; conditionally sets `[0]=1`). R: `_sound_control_value_select`(reset-voice path), `_voice_enable`, `_voice_disable`, `_sound_pit_divisor_program`, and now `_sound_control_block_advance` (F_C440, slot-active gate) (all: "if v_b!=0, skip — this call is a no-op") | Per-voice "output suppressed this tick" gate, checked before the backend-mode (1778) branch in all four PIT/gate routines, and (newly confirmed) before the pause-overlay renderer processes a slot at all. `layout/manifest.json` lists only `_v_b` at DGROUP 6012/0x177C -- the equate `state_active equ 177ch` in `asm/SOUND.ASM` duplicated this exact word and its own file comment wrongly claimed it had no extern; fixed in place (F_C440 now reads `[si+_v_b]`). Already named (not a `g`-number); no rename proposed. | — | (n/a, already named) |
| 1784 | `_g1784` / `_snd_mode` | word | W: F_C232 (init 2), F_C834 (reset 0). R: F_C1A0, F_C1F7 (0/2 branch) | Tick-service mode (0 = normal pump, 2 = "already primed, skip re-prime"). Distinct from 1778's backend selector. Already well-named. | — (keep `snd_mode`) | HIGH |
| 1786 | `_g1786` / `_snd_hi` | word | W: F_C834 (reset -1), F_C7CB (`=di`, the just-reloaded table id, i.e. `[bp+4]`) | Cache key for "which voice table is currently loaded" (parallels `music_track_handle` in RESCLOAD.C). Existing name plausible; not a bare `g`-number needing resolution. | — | MEDIUM |
| 1788 | `_g1788` | word (scalar, not per-voice) | W: `_sound_param_scale4` (secondary cmd 4, raw×4). R/W: `_voice_level_percent_scale` (adds into `g178a`), R: `_sound_command_value_derive` (`bx=g1788+g178a`, then nibble-shift/index logic feeding `command_value`=`v_ctr`) | Scratch "base" half of a base+percent pair whose result becomes the per-voice hold counter `v_ctr`. **Caution:** the existing function name `_voice_level_percent_scale` calls this "level", but the byte-identical sibling routine `F_C9A4`/`_sound_stream_delay_decode` runs the *same* algorithm to produce `_snd_delay` (a time value, not a volume). So "level" may be a misnomer for what looks structurally like a duration/delay base. Naming kept neutral pending that check. | `snd_value_base` | MEDIUM |
| 178A | `_g178a` | word (scalar) | W: `_voice_level_percent_scale`. R: `_sound_command_value_derive` | Scratch "percent-scaled delta" half of the same pair. Same "level vs delay" caveat as 1788. | `snd_value_pct` | MEDIUM |
| 178C | `_g178c` / `_dv`; equate `command_cursor` (F_C2EA, F_C440) | word ×4 | W: F_C232 (init/prime), F_C7CB (reload, same value as `sv`), F_C2EA (`+=2` per command, i.e. `command_cursor`). R: F_C232 (`es:[di]`), F_C1F7 (`_dv`, fixed offsets 0/2/4/6, refreshed from `_sv`) | **Confirmed same word, three names.** Live per-voice command-stream cursor: initialised from the reload table, advanced 2 bytes per dispatched command, and periodically reset back to its `sv` baseline in the pump loop when `snd_flag2`/`1776` is set (mode 0 only). `dv` (fixed-offset access) and `_g178c`/`command_cursor` (SI-indexed access) are the *same* 4-word table read two different ways. | `voice_stream_cursor_table` (retire `dv`, `g178c`, `command_cursor`) | HIGH (conflict) / MEDIUM-HIGH (name) |
| 1794 | `_sv` | word ×4 | W: F_C7CB (reload; written with the *same* value as `_g178c`/`dv` in the same instruction pair) | Baseline (reload-time) per-voice command-stream pointer, never advanced; the pump loop copies it back into `dv`/`g178c` to "rewind" a voice's stream. | `voice_stream_base_table` | MEDIUM-HIGH |
| 179C | `_v_ctr` / `_g179c`; equates `command_value` (F_C2EA/F_C3DB), `state_value` (F_C440) | word ×4 | W: `_sound_command_value_derive` (`command_value`, feeds from the 1788/178A pair). R/W: `_sound_voice_service_loop` (`v_ctr`, decremented every tick; note-off when it hits `v_len-1`). R: F_C440 (`state_value`, compared to `v_len`/`state_limit` for the pause-overlay renderer) | **Confirmed same word, three names** — the value the command dispatcher derives *is* the per-voice hold counter, which is then reused by the paused-mode renderer as a blink/limit comparator. Already has one good established extern (`v_ctr`); the two equates are redundant historical names from when this was compiled as separate fragments. | — (keep `v_ctr`; retire equates `command_value`/`state_value`) | HIGH |
| 17A4 | `_g17a4` | word ×4 | W: F_C232 (init 1), `_f_c5a8` (secondary cmd 3, direct AL override). R: `_voice_command_decode_apply` (copied into `v_ctr` after every note-decode: "C5D1_store_result") | Per-voice source value for `v_ctr` — the "duration" the next note-decode will latch in, overridable directly via secondary command 3. | `voice_dur_table` | MEDIUM |
| 17AC | `_v_a` / `_g17ac`; equates `state_ready` (F_C440), `command_state` (F_C2EA local re-declare, unused past that file) | word ×4 | W: F_C232 (0 or 2 depending on the per-voice entry-table marker byte). R: `_sound_voice_service_loop` (gate), F_C440 (`state_ready`) | Per-voice "this slot is wired to a live command table" flag (2=armed, 0=not). Already well-named. | — (keep `v_a`; retire equates) | HIGH |
| 17B4 | `_v_hold` / `_g17b4`; equate `state_allow` (F_C440) | word ×4 | W: `_sound_command_flags_update` (secondary cmd 0). R: `_sound_voice_service_loop` (skip off-check), F_C440 (`state_allow`) | Per-voice sustain/hold flag — twin of `snd_one` (1E94) in the stream cluster. Already well-named. | — (keep `v_hold`; retire equate) | HIGH |
| 17BC | `_v_len` / `_g17bc`; equate `state_limit` (F_C440) | word ×4 | W: `_sound_command_flags_update` (cleared alongside hold). R: `_sound_voice_service_loop` (off-check `ctr==len-1`), F_C440 (`state_limit`) | Per-voice note length — twin of `snd_len`(1E8E). Already well-named. | — (keep `v_len`; retire equate) | HIGH |
| 17C4 | equate `state_cursor` only | word ×4 | W: `_sound_table_word_select_store` (F_C5B3, secondary cmd 5, value from the DS:1832 table indexed by the command's own unmasked AL argument). R: F_C440 (copied into `state_cursor_next` only on the `C440_reset_cursor` path, i.e. when this slot's `voice_pending_table`==1 and `g17f4`<2) | Per-voice base pointer for the pause-overlay's byte-stream renderer (F_C440). The consumed bytes are written straight to the OPL data port through `_opl_port_write_byte`, with a per-slot register base (0x90/0xB0/0xD0/0xF0, +0x20 per slot) added to each byte read — so this is an in-segment offset to a stream of OPL register values, not literal text or frequencies, but *which* register block it targets is not established. Confirmed no data public exists at DGROUP 6084/0x17C4 anywhere in `layout/manifest.json`. | — (no proposal) | LOW |
| 17CC | equate `state_cursor_next` only | word ×4 | R/W: F_C440 (`C440_read_char`: live read cursor, `mov al,[di]` from DS directly (not ES-relative), advances by 1 byte, terminates the stream on 0xFF by forcing `state_cursor_mode`=1) | Live iterator over the byte stream `state_cursor`(17C4) points into; re-seeded from `state_cursor` every time this slot's mode resets to 0. Confirmed no data public at DGROUP 6092/0x17CC. | — (no proposal) | LOW |
| 17D4 | equate `state_cursor_mode` only | word ×4 | R/W: F_C440 only. Writers: `C440_reset_cursor` sets it to 0 (on a fresh low-`g17f4` decode); `C440_read_char` sets it to 1 the instant it reads a 0xFF terminator byte from the stream (and skips rendering entirely that tick). Reader: `C440_select_char`'s `cmp ...,0 / jz read_char; cmp ...,1 / jg render_blank` chain. | Per-voice progress state for the byte-stream renderer: **0 = actively reading** (advance `state_cursor_next` and render each byte); **1 = stream exhausted**, render the fixed override `state_text` every tick instead; **>1 = unreachable with the current writers** (only 0 and 1 are ever stored; the `render_blank` branch this value would select is dead code in the traced call graph). Note this is the *opposite* sense of an earlier draft of this table, which had 0/1 swapped. Confirmed no data public at DGROUP 6100/0x17D4. | — (no proposal) | LOW |
| 17DC | equate `state_text` only | word ×4 | W: `_f_c5c6` (F_C5C6, secondary cmd 6, direct zero-extended AL). R: F_C440 (`C440_select_char`, rendered every tick once `state_cursor_mode`==1, i.e. every tick *after* the byte stream has hit its terminator, until the next reset) | Per-voice "steady-state" glyph shown once the 17C4/17CC byte stream is exhausted — lets secondary command 6 inject one literal override byte into that same render path. Same "which register block" caveat as 17C4. Confirmed no data public at DGROUP 6108/0x17DC. | — (no proposal) | LOW |
| 17E4 | `voice_pending_table` (retired equates `state_mode`/`command_mode`; the two raw `[si+17e4h]` sites in F_C5D1 now go through the equate too) | word ×4 | W: `_sound_command_value_derive` (F_C3DB, always sets 1 after deriving a value), `_voice_command_decode_apply` (F_C5D1, sets 1 on both the mode-1 divisor path and the default/master-shift path). R: `_sound_control_block_advance` (F_C440, gates the `C440_reset_cursor` branch and is cleared back to 0 immediately after) | Per-voice "a value was just (re)decoded" flag — set by every note/command decode path, consumed once per tick by the pause-overlay renderer to decide whether to reseed the 17C4/17CC byte-stream cursor. Never touched by `sound_voice_table_prime`/`sound_voice_table_reload`; a pure per-tick decode-to-renderer handshake bit. Confirmed no data public exists anywhere in `layout/manifest.json` at DGROUP 6116/0x17E4. | `voice_pending_table` | MEDIUM |
| 17EC | `_g17ec`; equates `command_guard` (F_C2EA), `state_guard` (F_C440) | word ×4 | W: F_C232 (init 0), F_C2EA (0 on normal command / 1 on command "0"=note-off), `_voice_command_decode_apply` (1 on zero-value/note-off, 0 on full frequency decode). R: F_C440 (`state_guard==1` forces the blank render glyph) | Per-voice "last command was a rest/note-off" flag; read by the renderer to blank instead of drawing a note glyph. | `voice_rest_table` (retire equates) | MEDIUM-HIGH |
| 17F4 | `_g17f4`; equates `command_index` (F_C2EA/F_C3DB), `state_index` (F_C440) | word ×4 | W: F_C232 (init 0, prime only). `_sound_command_value_derive` (F_C3DB): when the command's AH bit 0x80 ("direct value") is clear and bit 0x10 is set, 0->1, 1->3 (2 is skipped), 3 stays 3; when bit 0x10 is clear, 0 stays 0, anything else increments by one with no upper clamp. R: F_C440 (`state_index`, compared against 1/2/3 to choose the pause-overlay's reset/select-char/render-blank branch — see `C440_slot_mode`/`C440_alternate_index`) | Per-voice 0..3-ish progress counter fully traced mechanically (exact bit tests and both progression rules known) but what it counts is not established; never touched by prime/reload beyond the initial zero. Its scalar twin at DS:1E8C runs the byte-identical state machine for the single music stream but is never read back by anything (see that row). | — (no proposal) | LOW |
| 17FC | `_notetab` | word[12] (one octave) | R: `_sound_control_value_select`, `_sound_note_dispatch`, `_stream_note_program` (all: `dec al; shl al,1; add bx,ax; shr ax,cl` — PIT-divisor-by-semitone lookup, then right-shift by octave) | Primary note→PIT-divisor table. Already well-named; span confirmed exactly 12 words (0x17FC..0x1813) by the gap to the next table. | — | HIGH |
| 1814 | equate `voice_table_b` only, comment "second note/frequency table" | word[12] | R: `_sound_control_value_select`, paused-mode (`snd_backend_mode==1`) branch only | Sibling table to `_notetab`, same 12-word shape and the identical `dec al;shl al,1;add bx,ax;mov ax,[bx];shr ax,cl` note/octave lookup, used only by the paused-backend note path. Span confirmed exactly 12 words: 0x1814+24=0x182C is exactly where the next table starts, the same tight packing `_notetab` shows. No C-facing extern exists (internal to SOUND.ASM); confirmed no data public at DGROUP 6164/0x1814. | `notetab_alt` (ASM-internal constant, optional) | MEDIUM |
| 182C | none — raw `[bx+182ch]` only, comment "unnamed word table" | word[4], only indices 0 and 3 ever read | R: `_voice_command_decode_apply` (F_C5D1), default path, indexed by `_snd_backend_mode*2` | Backend modes 1 and 2 both branch away before this code, so only mode 0 (index 0, address 0x182C) and mode 3 (index 3, address 0x182C+6=0x1832) are ever actually read; mode 3 is the transient probe value `sound_backend_select_init` immediately collapses back to mode 1, so only index 0 is live in steady state. The looked-up word is added to a negative, command-derived delta and right-shifted by an octave count before `_sound_pit_divisor_program` — the same shape as a `_notetab` lookup, i.e. a base PIT-divisor/f-number constant per backend. Index 3's address (0x1832) is the exact first word of the table below; index 2's address (0x1830) is `_opl_port` itself, but mode 2 never reaches this indexed read so that overlap is never exercised. Confirmed no data public at DGROUP 6188/0x182C. | — (no proposal) | LOW |
| 1830 | `_g1830` / `_opl_port` | word | W: `_sound_backend_select_init` (as `opl_port`: 0xC0/0x205). R: `_opl_register_write` (as `opl_port`, register-select port), `_opl_port_write_byte` (as `g1830`, same port), `opl_detect` in OPLREG.C (`inport(g1830)`) | **Confirmed same word** — `docs/current/interface-conflicts.json` lists `opl_port` with alias `g1830` at the same storage offset/evidence id, and the ASM extern list declares both `_opl_port` and `_g1830` as if they were separate words. One OPL/backend I/O port, reached by two names. | `opl_port` (retire `g1830`) | HIGH |
| 1832 | none — raw `[bx+1832h]` only, comment "unnamed word table" | word[N], N unknown | R: `_sound_table_word_select_store` (F_C5B3, secondary cmd 5), indexed by the command's own raw, unmasked AL argument byte (no upper bound enforced by this code) | Table selected by secondary command 5's argument byte; the looked-up word is stored straight into `state_cursor` (17C4) and later consumed by `_sound_control_block_advance` (F_C440) as an in-segment pointer to a stream of bytes written to the OPL data port — see 17C4's row. Size not established (extends past this report's DS:1834 boundary; would need the actual command-5 argument range used by the loaded sound data to measure). First word coincides with index 3 of the 182C table above — see that row for the same coincidence from the other side. Confirmed no data public at DGROUP 6194/0x1832. | — (no proposal) | LOW |

### DS:1E84..1E96 (secondary "music stream" cluster)

| Addr | Current name(s) / aliases | Width | Readers / writers | What the code shows | Proposed name | Conf. |
|---|---|---|---|---|---|---|
| 1E84 | `_g1e84` | word (scalar) | W: `_stream_level_base_set` (secondary ctlblock cmd 4, raw×4). R/W: `_stream_level_percent_scale` (adds into 1E86). R: `_sound_stream_delay_decode` (`bx=g1e84+g1e86`, feeds `_snd_delay`) | Scalar twin of 1788: base half of a base+percent pair feeding `_snd_delay`. Same "level-vs-delay naming" caveat as 1788/178A — the function is named `_stream_level_*` but the consumer is a delay value. | `stream_value_base` | MEDIUM |
| 1E86 | `_g1e86` | word (scalar) | W: `_stream_level_percent_scale`. R: `_sound_stream_delay_decode` | Percent-scaled delta half of the same pair. | `stream_value_pct` | MEDIUM |
| 1E88 | `_g1e88` / `_mus_ptr` | word | W: `_stream_control_block_arm` (far-pointer-table lookup result). R: `_sound_stream_command_step` (`di=_mus_ptr`, cursor into the command bytes) | Live cursor into the music-stream command bytes (ES:`_snd_seg`). Already well-named. | — (keep `mus_ptr`) | HIGH |
| 1E8A | `_g1e8a` / `_mus_arg` | word | R/W: `_stream_control_block_arm` (self-referential priority gate: only accepts a new index ≤ the currently-armed one, then stores it). R: `_sound_stream_command_step` (command 0xF, passed back in as the next arm argument) | Currently-armed stream/cue index; doubles as its own priority gate (lower index pre-empts higher). Already well-named. | — (keep `mus_arg`) | HIGH |
| 1E8C | `_g1e8c` | word (scalar) | W: `_sound_stream_delay_decode` (F_C9A4), the byte-identical AH-bit-driven 0..3 progression as `_g17f4` above. R: also `_stream_control_block_arm` (F_CAF1), zeroed when a new stream entry is armed | Scalar twin of `g17f4`; mechanism fully traced (identical bit tests and both progression rules) but, unlike `g17f4`, **nothing in this module or its extern list ever reads it back** — its only other touch is the arm-time zero. Either a genuinely dead leftover of the 178C-block algorithm reused verbatim for the single-stream case, or consumed by code outside this module's evidenced call graph. | — (no proposal) | LOW |
| 1E8E | `_g1e8e` / `_snd_len` | word (scalar) | W: `_sound_ctlblock_flags_latch` (secondary ctlblock cmd 0, cleared alongside `snd_one`), `_stream_control_block_arm` (init 6). R: `_sound_tick_step` (off-check `delay==len-1`) | Scalar twin of `v_len`. Already well-named. | — (keep `snd_len`) | HIGH |
| 1E90 | `_g1e90` / `_snd_delay` | word (scalar) | Pervasive: F_C8E2, F_C914, F_C9A4, F_CA9B, F_CAF1 | The music-stream's countdown timer — scalar twin of `v_ctr`. Already well-named. | — (keep `snd_delay`) | HIGH |
| 1E92 | `_g1e92` | word (scalar) | W: `_stream_note_delay_set` (secondary ctlblock cmd 3), `_stream_control_block_arm` (init 1). R: `_stream_note_program` (fallback: copied straight into `_snd_delay` on a repeat/rest note) | Default/fallback note delay, matches its own setter's name directly. | `stream_note_delay` | MEDIUM-HIGH |
| 1E94 | `_g1e94` / `_snd_one` | word (scalar) | W: `_sound_ctlblock_flags_latch` (secondary ctlblock cmd 0). R: `_sound_tick_step` (skips the off-check entirely) | Scalar twin of `v_hold`. Already well-named (if `snd_hold` would read more consistently with `v_hold`, that's a cosmetic-only suggestion, not included in the rename map). | — (keep `snd_one`) | HIGH |

## Alias conflicts (same physical DS word, two or more extern/equate names)

All of the following are corroborated either by `docs/current/interface-conflicts.json`'s
`storage_evidence`/`aliases` fields (which key on an evidence id, not just a
coincidental offset match) or by direct twin-routine comparison in
`asm/SOUND.ASM` itself:

1. **DS:1770** — `_g1770` / `_snd_flag` / `_snd_on` (three names, one word).
2. **DS:1778** — `_g1778` / `_snd_paused` (confirmed via identical
   `storage_objects/G_P212A8.phys` evidence id across 7 callers in both ASM
   and C). Semantically this is very likely "backend mode", with the
   "paused" name being a historical leftover from a separately-compiled
   fragment — flagged for a human check before the final call.
3. **DS:178C** — `_g178c` / `_dv` / equate `command_cursor` (three names,
   one 4-word table, read both by SI and by fixed offsets 0/2/4/6).
4. **DS:1830** — `_g1830` / `_opl_port` (confirmed via identical
   `storage_objects/G_P21360.phys`-style evidence in `interface-conflicts.json`,
   plus `opl_detect()` in OPLREG.C reading `inport(g1830)` as the same
   register-select port `_opl_register_write` addresses as `_opl_port`).
5. **DS:179C** — `_v_ctr` / `_g179c` / equates `command_value` and
   `state_value` (one word, three redundant historical names beyond the
   already-good `v_ctr`).
6. **DS:17AC** — `_v_a` / `_g17ac` / equate `state_ready` (+ a same-named
   local `command_state` re-declaration in F_C2EA's block, unused beyond
   that file).
7. **DS:17B4** — `_v_hold` / `_g17b4` / equate `state_allow`.
8. **DS:17BC** — `_v_len` / `_g17bc` / equate `state_limit`.
9. **DS:17E4** — equates `state_mode` / `command_mode`, folded into
   `voice_pending_table` (no data public exists at this address, confirmed
   by the sound2 pass; stays an equate, not an extern). The two bare
   `[si+17e4h]` sites in F_C5D1 that used to bypass even the equate now go
   through `voice_pending_table` too.
10. **DS:17EC** — `_g17ec` / equates `command_guard` / `state_guard`.
11. **DS:17F4** — `_g17f4` / equates `command_index` / `state_index`.
12. **DS:1772** — `_music_enabled` has a live extern, but F_C7CB reaches the
    same word through a bare `[1772h]` displacement instead of using it.
    (Fixed in an earlier pass — no raw `1772h` remains.)
13. **DS:177C** — `_v_b` / equate `state_active` (found in the sound2 pass,
    2026-09-21): confirmed via `layout/manifest.json`'s `DGROUP_offset`
    bindings, which list only `_v_b` at offset 6012 (0x177C). Fixed: the
    equate is removed and `_sound_control_block_advance` (F_C440) now reads
    `[si+_v_b]` directly like every other reader of this word.

## (a) Rename map — old = new (HIGH / MEDIUM-HIGH confidence only)

```
_g175e            = snd_base
_g1762            = snd_base2
_g1770 / _snd_flag = snd_on          ; retire _snd_flag, keep one name at DS:1770
_g1778 / _snd_paused = snd_backend_mode   ; HIGH-confidence conflict, MEDIUM-HIGH name (see caveat above)
_g178c / _dv      = voice_stream_cursor_table   ; also retire equate command_cursor
_sv               = voice_stream_base_table
_g17ec            = voice_rest_table  ; also retire equates command_guard/state_guard
_g1830            = opl_port          ; retire _g1830, keep _opl_port
_g1e92            = stream_note_delay
```

Equate-only retirements (fold into the extern already named for that
address; no new symbol needed):

```
command_value / state_value  -> _v_ctr        (DS:179C)
state_ready   / command_state -> _v_a          (DS:17AC)
state_allow                   -> _v_hold       (DS:17B4)
state_limit                   -> _v_len        (DS:17BC)
```

MEDIUM-confidence proposals (evidenced ASM-internal equate names; not
promoted to SOUND.H since no data public backs them and no C consumer
touches them): `voice_retune_base_table` (1766), `snd_value_base`/
`snd_value_pct` (1788/178A), `voice_dur_table` (17A4), `voice_pending_table`
(17E4), `stream_value_base`/`stream_value_pct` (1E84/1E86).

LOW-confidence fields, mechanism now fully traced (register flow, compare/
add targets, and hardware sink where one exists) but meaning still not
established, so no name is proposed: 17C4, 17CC, 17D4, 17DC, 17F4, 1E8C, and
the two internal tables at 182C/1832 — see their rows in the Field map above
for the per-word mechanism, and `build/probes/sound2/CANDIDATES.md` for
proposed data-public names a supervisor could still assign despite the
open meaning.

## (b) Raw displacements in asm/SOUND.ASM (resolved)

The `[1762h]` and `[1772h]` sites this section used to list were fixed in an
earlier pass (both now go through `_snd_base2`/`_music_enabled`; no raw
`1762h`/`1772h` remains in the file).

The sound2 pass (2026-09-21) re-checked the other two proposals against an
exhaustive scan of `layout/manifest.json`'s `DGROUP_offset` bindings across
every code region in the binary, not just this file, and found **no data
public at either address** — so, per the "only promote to `extrn` when a
data symbol exists" rule, neither became a real extern. Instead:

```
[si+1766h]  (F_C678, F_C706)  -> [si+voice_retune_base_table]  ; equate, not extrn -- no data public at 0x1766
[si+17e4h]  (F_C5D1, twice)   -> [si+voice_pending_table]      ; equate, not extrn -- no data public at 0x17E4
```

Both equates already carried the evidenced name; only the two call sites
that bypassed even the equate and hard-coded the hex literal were fixed.
Two more raw displacements found and fixed in the same pass, this time
promoted to a real extern because a data public *does* exist at their
address (confirmed the same way):

```
[si+17bch]  (F_C549)   -> [si+_v_len]                   ; extrn already declared, now used
[si+1794h]  (F_C7CB)   -> [si+_voice_stream_base_table] ; extrn already declared, now used
```

And one equate turned out to duplicate an existing extern outright:
`state_active equ 177ch` (F_C440) named the same word as `_v_b`; the equate
is removed and F_C440 now reads `[si+_v_b]` directly (see the Alias
conflicts list above).

All other DS:17xx/1E9x raw uses in SOUND.ASM go through an equate or extern;
`python tools/probe_module.py M_C1A0_CB48` stayed EXACT through every one of
these changes (2492/2492 bytes; fixups 191 -> 194, since an extern-backed
access carries a fixup a raw/equate constant offset never did). See
`build/probes/sound2/CANDIDATES.md` for the new-data-public proposals this
pass raises for 1766/17C4/17CC/17D4/17DC/17E4/1814/182C.

## (c) Proposed include/SOUND.H contents (historical; see include/SOUND.H for what actually shipped)

This was the original proposal from the pass that produced most of the
Field map above. The project's actual `include/SOUND.H` ended up narrower:
it kept `g1788`/`g178a`/`g17a4`/`g1e84`/`g1e86` under their `g`-numbers
rather than the `snd_value_base`/`snd_value_pct`/`voice_dur_table`/
`stream_value_base`/`stream_value_pct` names proposed here, and never added
`voice_pending_table`/`voice_retune_base_table`/`voice_rest_table`-style
entries for words no C translation unit touches (`voice_pending_table` and
`voice_retune_base_table` in particular stay ASM-internal equates — see
their Field-map rows). Treat the block below as this pass's reasoning, not
as the current header; only the fields with an existing extern (HIGH) or a
same-or-higher confidence proposed rename were included here, and
LOW-confidence fields were left as ASM-internal equates per the "no
mechanical names" rule.

```c
#ifndef EMPIRES_SOUND_H
#define EMPIRES_SOUND_H

/* Loaded sound-resource block (far pointer split across two DS words each;
   set once by src/PLRLDPUB.C). */
extern unsigned snd_seg;        /* DS:1760 -- segment */
extern unsigned snd_base;       /* DS:175E -- offset */
extern unsigned snd_seg2;       /* DS:1764 -- staging-block segment */
extern unsigned snd_base2;      /* DS:1762 -- staging-block offset */

/* Master enable flags (options menu + timer gate). */
extern int sound_enabled;       /* DS:176E */
extern int music_enabled;       /* DS:1772 */

/* Sound-effects engine state (up to 4 concurrent "voices"). */
extern int snd_on;              /* DS:1770 -- a stream is armed/playing (was snd_flag/snd_on) */
extern int mus_flag;            /* DS:1774 -- keep auto-chaining the stream */
extern int snd_flag2;           /* DS:1776 -- see caveat: non-sound call sites exist */
extern int snd_backend_mode;    /* DS:1778 -- 0=PC speaker,1=Tandy gate,2=queued OPL,3=probe-only (was g1778/snd_paused) */
extern int snd_nvoices;         /* DS:177A -- 1 or 4 */
extern int v_b[4];              /* DS:177C -- per-voice output-suppressed gate */
extern int snd_mode;            /* DS:1784 -- 0=normal pump, 2=already primed */
extern int snd_hi;              /* DS:1786 -- currently-loaded voice-table id */
extern int snd_value_base;      /* DS:1788 -- see 1788/178A caveat */
extern int snd_value_pct;       /* DS:178A */

extern int voice_stream_cursor_table[4]; /* DS:178C -- live per-voice command cursor (was dv/g178c) */
extern int voice_stream_base_table[4];   /* DS:1794 -- reload-time baseline for the cursor (was sv) */
extern int v_ctr[4];            /* DS:179C -- per-voice hold/duration counter */
extern int voice_dur_table[4];  /* DS:17A4 -- source value latched into v_ctr on decode */
extern int v_a[4];              /* DS:17AC -- per-voice armed flag */
extern int v_hold[4];           /* DS:17B4 -- per-voice sustain flag */
extern int v_len[4];            /* DS:17BC -- per-voice note length */
/* DS:17C4/17CC/17D4/17DC (pause-overlay renderer cursor/mode/override) and
   DS:17F4 (small decode index) are left as SOUND.ASM-internal state: their
   mechanism is understood but their exact purpose is not, and no C
   translation unit currently reaches them. */
extern int voice_pending_table[4]; /* DS:17E4 -- "value just decoded" flag */
extern int voice_rest_table[4]; /* DS:17EC -- per-voice last-command-was-rest flag */

extern int voice_retune_base_table[4]; /* DS:1766 -- queued value for voice_bank_retune_on */

extern unsigned notetab[12];    /* DS:17FC -- note -> PIT divisor, one octave */
extern unsigned opl_port;       /* DS:1830 -- OPL/backend I/O port (was g1830/opl_port, unified) */

/* Secondary "music stream" cluster (single stream, no per-voice indexing). */
extern int stream_value_base;   /* DS:1E84 -- see 1788/178A caveat */
extern int stream_value_pct;    /* DS:1E86 */
extern int mus_ptr;             /* DS:1E88 -- cursor into the stream's command bytes */
extern int mus_arg;             /* DS:1E8A -- armed cue index / priority key */
extern int snd_len;             /* DS:1E8E -- scalar twin of v_len */
extern int snd_delay;           /* DS:1E90 -- scalar twin of v_ctr */
extern int stream_note_delay;   /* DS:1E92 -- default/fallback note delay */
extern int snd_one;             /* DS:1E94 -- scalar twin of v_hold */

#endif
```

## Open items for a human before acting on any of this

1. **DS:1778** (`snd_paused`/`g1778`): the conflict is certain, the
   consolidated name is not — check whether the "paused" label at
   `_sound_control_value_select`'s value==1 branch reflects a genuine
   secondary meaning before deleting it.
2. **DS:1776** (`snd_flag2`/`g1776`): several C call sites (GAME.C, INTRO.C,
   LEVEL.C, PLAYERSL.C) touch this address in contexts that don't obviously
   involve sound; worth checking whether the offset registry has conflated
   two different physical words.
3. **DS:1788/178A and DS:1E84/1E86**: the existing function names
   (`_voice_level_percent_scale`, `_stream_level_percent_scale`,
   `_stream_level_base_set`) say "level", but the byte-identical algorithm
   feeds a duration/delay value (`v_ctr`/`snd_delay`), not a volume. Worth a
   second look at those *function* names (out of scope for this data-only
   report) before trusting "level" as this pair's semantics.
4. **DS:182C/0x1832**: index 3 of the 182C table (reachable only when
   `snd_backend_mode==3`, which is itself transient) lands on the same word
   as the start of the 1832 table — likely harmless (mode 3 never reaches
   that code path in practice) but not proven here. Re-confirmed by the
   sound2 pass from both tables' comments in `asm/SOUND.ASM`; still not
   resolved either way.
5. **DS:1832 table size** (sound2 pass): selected by secondary command 5's
   raw, unmasked argument byte with no bound enforced in this module — its
   real size can only be measured from the actual command-5 argument range
   the loaded sound/level data uses, which this report did not have access
   to. Do not assign it a data public with a guessed length (see
   `build/probes/sound2/CANDIDATES.md`, which deliberately proposes no name
   for it).
6. **DS:1E8C** (`g1e8c`, sound2 pass): runs the identical 0..3 progression
   as `g17f4`, but unlike `g17f4` nothing in this module's evidenced call
   graph ever reads it back after writing it (only zeroed at arm time).
   Worth checking whether some out-of-module consumer reads it, or whether
   it is genuinely dead code inherited from copying the 178C-block
   algorithm onto the single-stream cluster.
