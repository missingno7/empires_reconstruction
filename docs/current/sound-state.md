# SOUND.ASM state field map (DS:175E..1834, DS:1E84..1E96)

Research artifact only. No tracked file was edited. Sources cross-checked:
`asm/SOUND.ASM` (equates, extern list, comments), `src/*.C` + `include/*.H`
(extern declarations and call sites), `recipes/c/*.json` (DGROUP-offset
bindings for the SOUND.ASM owners), `docs/current/interface-conflicts.json`
(the project's own global-symbol/alias/storage-evidence registry, which
already tracks most of these words), and `layout/manifest.json` (DGROUP
frame base, used only to confirm the offset convention).

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
| 1766 | none — **raw `[si+1766h]` only**, comment "per-voice queue slot" | word ×4 (voice 0..3) | W: `_sound_pit_divisor_program` (queued-backend path, `[si+1766h]=ax`). R: `_voice_enable` (queued-gate path, pushes `[si+1766h]` as the `base` argument to `voice_bank_retune_on(bank,base)` in OPLVOICE.C) | Per-voice queued PIT-divisor/tone value awaiting delivery to the 9-voice OPL bank via `voice_bank_retune_on`. Never has a C-side name; only ever touched by SOUND.ASM raw displacements. | `voice_retune_base_table` | MEDIUM-HIGH |
| 176E | `_f1` (SNDFXTGL.C) / `_sound_enabled` | word | W/R: OPTIONS.C, TIMER.C, PLAYERSL.C, SLOTMENU.C, SNDFXTGL.C | Master sound-effects on/off (options menu + timer gate). Already well-named; `f1` is a local alias in one TU only. | — (keep `sound_enabled`) | HIGH |
| 1770 | `_g1770` / `_snd_flag` (F_C1A0) / `_snd_on` (F_C8E2, F_CAF1, F_CB48) | word | R: `_sound_tick_entry` as `snd_flag`. R/W: `_sound_tick_step`, `_stream_control_block_arm`, `_sound_stop_reset` as `snd_on`. C: INTRO.C sets it; ROUNDEND.C busy-waits `while(g1770);` after `stream_control_block_arm(9)` | **Confirmed same word, three names**, all inside SOUND.ASM/its immediate C callers (`docs/current/interface-conflicts.json` groups them under one `storage_evidence`/alias set). Semantics: "a stream is currently armed/playing" — set to 2 by `stream_control_block_arm`, cleared by `sound_stop_reset`, gates whether `sound_tick_step` even runs. `roundend_wait`'s busy-wait proves this is a genuine playing-flag, not two coincidentally-overlapping concepts. | `snd_on` (retire `snd_flag`) | HIGH (conflict) / MEDIUM-HIGH (name) |
| 1772 | `_f2` (SNDFXTGL.C) / `_music_enabled`; **also raw `[1772h]` in F_C7CB, no extern used there** | word | W/R: OPTIONS.C, TIMER.C, PLAYERSL.C, SLOTMENU.C, SNDFXTGL.C. R (raw): `_sound_voice_table_reload` gates its whole body on `[1772h]!=0` | Master music on/off; F_C7CB's raw check ("only reload the voice table while music is enabled") is a second, unnamed consumer of the same flag. | — (keep `music_enabled`; promote the raw displacement to `_music_enabled`) | HIGH |
| 1774 | `_g1774` / `_mus_flag` | word | W: INTRO.C (brackets `stream_control_block_arm` calls). R/W: `_sound_stream_command_step` (F_C914, command 0xF: continue-chain vs `sound_stop_reset`) | "Keep auto-chaining the music stream" flag. Already well-named. | — (keep `mus_flag`) | HIGH |
| 1776 | `_g1776` / `_snd_flag2` | word | R: `_sound_voice_pump_loop` (gates the `sv`→`dv` freeze copy). Also read/written as plain `g1776` in GAME.C, INTRO.C, LEVEL.C, PLAYERSL.C in contexts that don't obviously involve sound | Confirmed alias by the project's own registry, but the non-sound C call sites are worth a second look before treating `snd_flag2` as the *only* correct reading — flagging for human review rather than asserting more than the evidence shows. | — (keep `snd_flag2`, but see caveat) | MEDIUM |
| 1778 | `_g1778` (RESCLOAD.C: "the mode"; STARTUP.C `-SI/-ST/-SA` command-line flags; `sound_backend_probe`) / `_snd_paused` (F_C1A0, F_C27D, F_C834) | word | See below | **Confirmed same word** — `docs/current/interface-conflicts.json` cites the *identical* `storage_objects/G_P212A8.phys` evidence id for both names across 7 independent callers. C-side evidence is unambiguous: `cmdline_parse_args` sets it from `-SI`(0)/`-ST`(1)/`-SA`(2); `sound_backend_probe` sets 0/1/2/3 from `display_mode`/`opl_detect`/a port probe; `_sound_backend_select_init`, `_sound_control_value_select`, `_voice_enable/_disable`, `_sound_pit_divisor_program`, `_sound_voice_table_reload`, `_sound_voices_disable_all` all branch on 0/1/2/3 as an output-backend selector (0=PC speaker, 1=Tandy/PCjr-style gate, 2=queued OPL bank, 3=a one-shot probe that immediately collapses back into 1). **Caveat:** the ASM's own label at `_sound_control_value_select`'s value==1 branch is named `C359_paused_mode`, and `F_C834`/`F_C877` both cap voice count to 4 by comparing this word to 2 — one routine spells it `_snd_paused`, the twin routine `_g1778`. This looks like the driver historically overloaded "which backend" with "reduced/paused service", so the *conflict* is HIGH confidence but the *final* name deserves a human look before commit. | `snd_backend_mode` (retire `snd_paused`) | HIGH (conflict) / MEDIUM-HIGH (name, with caveat) |
| 177A | `_g177a` / `_snd_nvoices` | word | W: `_sound_backend_select_init` (1 or 4). R: loop bounds in F_C232, F_C27D, F_C7CB, F_C877 | Active voice count for this cluster (1 or 4, never the 9 of the OPL bank). Already well-named. | — (keep `snd_nvoices`) | HIGH |
| 177C | `_v_b` | word ×4 | W: `_sound_tick_entry` (zeroes all 4 every tick; conditionally sets `[0]=1`). R: `_sound_control_value_select`(reset-voice path), `_voice_enable`, `_voice_disable`, `_sound_pit_divisor_program` (all: "if v_b!=0, skip — this call is a no-op") | Per-voice "output suppressed this tick" gate, checked before the backend-mode (1778) branch in all four PIT/gate routines. Already named (not a `g`-number); no rename proposed. | — | (n/a, already named) |
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
| 17C4 | equate `state_cursor` only | word ×4 | W: `_sound_table_word_select_store` (secondary cmd 5, value from the DS:1832 table by AL). R: F_C440 (`state_cursor` copied into `state_cursor_next` on reset) | Per-voice base pointer for the pause-overlay's byte-stream renderer (F_C440). Mechanism is clear; the actual visual/protocol purpose of what F_C440 emits through `_opl_port_write_byte` (0x90/0xB0/0xD0/0xF0 + a byte read from this cursor) is not established well enough to name confidently — it may not be "text" at all. | — (no proposal) | LOW |
| 17CC | equate `state_cursor_next` only | word ×4 | R/W: F_C440 (live read cursor, advances by 1 byte; terminates on 0xFF) | Live iterator over the table `state_cursor`(17C4) points into. | — (no proposal) | LOW |
| 17D4 | equate `state_cursor_mode` only | word ×4 | R/W: F_C440 (0=reset, 1=reading, >1=exhausted/blank) | Per-voice small state machine driving F_C440's render choice. | — (no proposal) | LOW |
| 17DC | equate `state_text` only | word ×4 | W: `_f_c5c6` (secondary cmd 6, direct AL). R: F_C440 (used verbatim when `state_cursor_mode==1`) | Per-voice direct override value for F_C440's renderer. Same caveat as 17C4. | — (no proposal) | LOW |
| 17E4 | equates `state_mode` (F_C440) / `command_mode` (F_C3DB); **raw `[si+17e4h]` in F_C5D1, twice, no equate used there** | word ×4 | W: `_sound_command_value_derive` (always sets 1 after deriving), `_voice_command_decode_apply` (sets 1 on both the mode-1 and default/master decode paths, raw). R: F_C440 (`state_mode`, selects the "just decoded" render branch) | Per-voice "a value was just (re)decoded" flag — set by every note/command decode path, read once by the pause-overlay renderer. Mechanically clear. | `voice_pending_table` | MEDIUM |
| 17EC | `_g17ec`; equates `command_guard` (F_C2EA), `state_guard` (F_C440) | word ×4 | W: F_C232 (init 0), F_C2EA (0 on normal command / 1 on command "0"=note-off), `_voice_command_decode_apply` (1 on zero-value/note-off, 0 on full frequency decode). R: F_C440 (`state_guard==1` forces the blank render glyph) | Per-voice "last command was a rest/note-off" flag; read by the renderer to blank instead of drawing a note glyph. | `voice_rest_table` (retire equates) | MEDIUM-HIGH |
| 17F4 | `_g17f4`; equates `command_index` (F_C2EA/F_C3DB), `state_index` (F_C440) | word ×4 | W: F_C232 (init 0), `_sound_command_value_derive` (0..3 state machine on decode-flag bits). R: F_C440 (`state_index`, several branch comparisons) | Small per-voice index (0..3) driven by the command-value decoder and read by the renderer; the decode rules are understood mechanically but not what the index *represents*. | — (no proposal) | LOW |
| 17FC | `_notetab` | word[12] (one octave) | R: `_sound_control_value_select`, `_sound_note_dispatch`, `_stream_note_program` (all: `dec al; shl al,1; add bx,ax; shr ax,cl` — PIT-divisor-by-semitone lookup, then right-shift by octave) | Primary note→PIT-divisor table. Already well-named; span confirmed exactly 12 words (0x17FC..0x1813) by the gap to the next table. | — | HIGH |
| 1814 | equate `voice_table_b` only, comment "second note/frequency table" | word[12] | R: `_sound_control_value_select`, paused-mode (`g1778==1`) branch only | Sibling table to `_notetab`, same 12-word shape, used only by the paused/alternate-backend note path. No C-facing extern exists (internal to SOUND.ASM). | `notetab_alt` (ASM-internal constant, optional) | MEDIUM |
| 182C | none — raw `[bx+182ch]` only, comment "unnamed word table" | word[≤4] | R: `_voice_command_decode_apply`, default/master path, indexed by `g1778*2` | Small table indexed by backend mode (0 or 3 in practice, since modes 1/2 branch away earlier). Its high end (index 3 → offset 1832) coincides with the start of the next table below — worth a human check for an accidental overlap, not asserted here. | — (no proposal) | LOW |
| 1830 | `_g1830` / `_opl_port` | word | W: `_sound_backend_select_init` (as `opl_port`: 0xC0/0x205). R: `_opl_register_write` (as `opl_port`, register-select port), `_opl_port_write_byte` (as `g1830`, same port), `opl_detect` in OPLREG.C (`inport(g1830)`) | **Confirmed same word** — `docs/current/interface-conflicts.json` lists `opl_port` with alias `g1830` at the same storage offset/evidence id, and the ASM extern list declares both `_opl_port` and `_g1830` as if they were separate words. One OPL/backend I/O port, reached by two names. | `opl_port` (retire `g1830`) | HIGH |
| 1832 | none — raw `[bx+1832h]` only, comment "unnamed word table" | word[N], N unknown | R: `_sound_table_word_select_store` (secondary cmd 5), indexed by AL*2 | Table selected by secondary command 5's argument byte; size not established (extends past this report's DS:1834 boundary). | — (no proposal) | LOW |

### DS:1E84..1E96 (secondary "music stream" cluster)

| Addr | Current name(s) / aliases | Width | Readers / writers | What the code shows | Proposed name | Conf. |
|---|---|---|---|---|---|---|
| 1E84 | `_g1e84` | word (scalar) | W: `_stream_level_base_set` (secondary ctlblock cmd 4, raw×4). R/W: `_stream_level_percent_scale` (adds into 1E86). R: `_sound_stream_delay_decode` (`bx=g1e84+g1e86`, feeds `_snd_delay`) | Scalar twin of 1788: base half of a base+percent pair feeding `_snd_delay`. Same "level-vs-delay naming" caveat as 1788/178A — the function is named `_stream_level_*` but the consumer is a delay value. | `stream_value_base` | MEDIUM |
| 1E86 | `_g1e86` | word (scalar) | W: `_stream_level_percent_scale`. R: `_sound_stream_delay_decode` | Percent-scaled delta half of the same pair. | `stream_value_pct` | MEDIUM |
| 1E88 | `_g1e88` / `_mus_ptr` | word | W: `_stream_control_block_arm` (far-pointer-table lookup result). R: `_sound_stream_command_step` (`di=_mus_ptr`, cursor into the command bytes) | Live cursor into the music-stream command bytes (ES:`_snd_seg`). Already well-named. | — (keep `mus_ptr`) | HIGH |
| 1E8A | `_g1e8a` / `_mus_arg` | word | R/W: `_stream_control_block_arm` (self-referential priority gate: only accepts a new index ≤ the currently-armed one, then stores it). R: `_sound_stream_command_step` (command 0xF, passed back in as the next arm argument) | Currently-armed stream/cue index; doubles as its own priority gate (lower index pre-empts higher). Already well-named. | — (keep `mus_arg`) | HIGH |
| 1E8C | `_g1e8c` | word (scalar) | W/R: `_sound_stream_delay_decode` (0..3 state machine, same shape as 17F4) | Scalar twin of `g17f4`; mechanism clear, meaning not established. | — (no proposal) | LOW |
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
9. **DS:17E4** — equates `state_mode` / `command_mode` (no extern at all;
   also touched via bare `[si+17e4h]` in F_C5D1 with no equate).
10. **DS:17EC** — `_g17ec` / equates `command_guard` / `state_guard`.
11. **DS:17F4** — `_g17f4` / equates `command_index` / `state_index`.
12. **DS:1772** — `_music_enabled` has a live extern, but F_C7CB reaches the
    same word through a bare `[1772h]` displacement instead of using it.

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

MEDIUM-confidence proposals (worth adding to SOUND.H, but not asserted as
certain): `voice_retune_base_table` (1766), `snd_value_base`/`snd_value_pct`
(1788/178A), `voice_dur_table` (17A4), `voice_pending_table` (17E4),
`stream_value_base`/`stream_value_pct` (1E84/1E86).

LOW-confidence fields intentionally left unnamed (mechanism understood,
meaning is not): 17C4, 17CC, 17D4, 17DC, 17F4, 1E8C, and the two internal
tables at 182C/1832.

## (b) Raw displacements in asm/SOUND.ASM that could become named externs

```
[1762h]            (F_C7CB, twice)         -> extrn _snd_base2:word      ; [_snd_base2]
[1772h]            (F_C7CB)                -> ds:_music_enabled          ; already has an extern elsewhere
[si+1766h]         (F_C678, F_C706)        -> extrn _voice_retune_base_table:word ; [si+_voice_retune_base_table]
[si+17e4h]         (F_C5D1, twice)         -> extrn _voice_pending_table:word     ; [si+_voice_pending_table]
```

All other DS:17xx/1E9x raw uses in SOUND.ASM already go through an equate;
the four above are the only ones that bypass even an equate and hard-code
the hex literal.

## (c) Proposed include/SOUND.H contents

Only the fields with an existing extern (HIGH) or a same-or-higher
confidence proposed rename are included; LOW-confidence fields are left as
ASM-internal equates (not promoted to C-visible names) per the "no
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
   that code path in practice) but not proven here.
