/* sound.h -- SOUND.ASM/OPL C-facing entry points the game calls.
 *
 * Interim surface for Wave 4 (tu-porting-rules.md sec 5: "sound_*, voice_*
 * ASM publics -> sound.h (portable driver, Wave 4; stubs until then)").
 * This header only DECLARES the functions asm/SOUND.ASM exposed to C and
 * the backend primitives that replace OPLREG.C's direct port I/O; it does
 * not implement anything (stubs live in portable/audio/sound_stub.c) and it
 * does not declare any DGROUP object -- every SOUND.ASM/SOUND.H state
 * object is either generator-owned (game_state.h for BSS, game_data.h for
 * initialized DATA) or, per the gap this header documents below, missing
 * entirely from both.  #include "game_state.h" for the objects that ARE
 * there; do not add `extern` redeclarations of generated state here.
 *
 * ---------------------------------------------------------------------
 * GAP (task item 3 "check the names exist in game_state.h and report any
 * missing" -- reported, NOT fixed here; portable/generated is owned by a
 * different agent):
 *
 * None of include/SOUND.H's individually-named DGROUP objects
 * (sound_enabled, music_enabled, snd_on, mus_flag, snd_flag2,
 * snd_backend_mode, snd_nvoices, v_b/voice_stream_cursor_table/
 * voice_stream_base_table/v_ctr/v_a/v_hold/v_len, notetab, opl_port,
 * snd_seg/snd_seg2/snd_base/snd_base2, mus_ptr, mus_arg, snd_len,
 * snd_delay, stream_note_delay, snd_one) exist as individually-addressable
 * symbols in game_state.h.  `docs/portable/state-map.md` line ~112 shows
 * the whole 1832-byte span DS:176E..1E96 was captured by
 * tools/portable/datagen.py as ONE opaque initialized-DATA blob,
 * `portable/generated/game_data.h`'s `struct DATA_01139E_SOUND_s`
 * (`state_words[71]`, `note_divisors[24]`, assorted `lookup_*` byte
 * arrays), with only `#define sound_enabled DATA_01139E_SOUND` (the whole
 * STRUCT INSTANCE, not an int field) surviving as an alias.  That means:
 *   - `sound_enabled` / `music_enabled` are currently macros for an
 *     aggregate; `if (sound_enabled)` (as src/TIMER.C, src/OPTIONS.C,
 *     src/SNDFXTGL.C all write it) will not compile once a ported .c file
 *     includes both this header's umbrella (game.h) and touches either
 *     name -- this is a REAL, load-bearing gap, not a style nit.
 *   - `snd_on` (read directly by src/ROUNDEND.C's `roundend_wait`:
 *     `while (snd_on) ;`) has no symbol at all, not even inside the blob
 *     alias list.
 *   - `sound_request_count` (DS:237C, src/TIMER.C/src/SNDREQ.C) is a
 *     SEPARATE address from the 175E..1E96 span entirely and also has no
 *     generated symbol anywhere; today it only exists as the ad hoc
 *     `dos_int sound_request_count;` portable/audio/sound_stub.c defines
 *     for timer.c's sake (timer.c declares it itself via a local `extern`,
 *     not through any header).
 * Fixing this needs either per-field decode of DATA_01139E_SOUND (turning
 * the blob into a real `struct` with named fields, still in game_data.h)
 * or moving the scalar control fields into game_state.h as BSS + adding
 * `sound_request_count`'s DS:237C object -- both are portable/generated
 * changes, out of this header's ownership.  Flagged for the generator
 * owner / Wave 4; portable/audio/sound_stub.c's ad hoc globals remain the
 * only working definitions of sound_request_count/sound_enabled/
 * music_enabled until that lands, and this header deliberately does NOT
 * redeclare them (that would conflict with game_data.h's macro once both
 * headers are included together).
 * ---------------------------------------------------------------------
 */
#ifndef PORTABLE_SOUND_H
#define PORTABLE_SOUND_H

#include "dos_types.h"
#include "game_state.h"

/* ---- asm/SOUND.ASM's 8 C-facing entry points (docs/portable/
 * asm-module-inventory.md sec 9 "C-facing entry points"; the other 33
 * PUBLIC routines in that module are ASM-to-ASM-only register-ABI helpers
 * with no C caller -- see game_funcs.h's asm/SOUND.ASM section) ---- */

/* F_C1A0 -- per-tick service entry, called from timer_service_tick() under
 * the historical gate (`!sound_request_count && (sound_enabled ||
 * music_enabled)`, src/TIMER.C).  Stub: portable/audio/sound_stub.c. */
void sound_tick_entry(void);

/* F_C77A -- select and initialise a sound backend (0=PC speaker, 1=Tandy
 * gate, 2=queued OPL, 3=probe-only).  Caller: src/PLRLDPUB.C. */
void sound_backend_select_init(void);

/* F_C7CB -- reload per-voice values from the current backend table (gated
 * on music_enabled).  Caller: src/RESCACHE.C. */
void sound_voice_table_reload(dos_int v);

/* F_C834 -- reset each configured voice, retain voice count.  Callers:
 * src/GAME.C, src/LEVEL.C, src/OPTIONS.C, src/PLAYERSL.C, src/SLOTMENU.C,
 * src/SNDFXTGL.C. */
void sound_voices_reset(void);

/* F_C877 -- submit a disabled-voice update for every voice.  Caller:
 * src/SNDREQ.C (sound_start). */
void sound_voices_disable_all(void);

/* F_C898 -- write one OPL register+data pair with the required settling
 * delay (register-select write, 6x settling read, data write, 35-iteration
 * settling loop).  Callers: src/OPLINIT.C, src/OPLREG.C, src/VOXCHAN.C.
 * NOT the same thing as this header's own opl_write() below -- this is
 * the historical *timed* register write asm/SOUND.ASM performed against
 * real OPL hardware; opl_write() is the new backend primitive the Wave 4
 * driver uses instead (Nuked-OPL3 or similar has no port-I/O settling
 * requirement). */
void opl_register_write(dos_int reg, dos_int val);

/* F_CAF1 -- arm the single music-stream cursor for a new cue index
 * (self-referential priority gate: only a <=-current index pre-empts).
 * Callers: src/BOARD.C, src/GAME.C, src/HITTEST.C, src/INTRO.C,
 * src/LEVEL.C, src/PUZZLE.C, src/ROUNDEND.C; also asm/SPRITES.ASM
 * (ASM-to-ASM, the bytecode interpreter). */
void stream_control_block_arm(dos_int n);

/* F_CB48 -- clear the "stream armed" flag and invalidate the cached stream
 * index.  Callers: src/BOARD.C, src/GAME.C, src/INTRO.C, src/LEVEL.C,
 * src/OPTIONS.C, src/PLAYERSL.C, src/PUZZLE.C, src/ROUNDEND.C,
 * src/SLOTMENU.C, src/SNDFXTGL.C, src/SNDREQ.C. */
void sound_stop_reset(void);

/* ---- backend API (tu-porting-rules.md sec 5: "opl_register_write,
 * opl_detect, port I/O -> sound.h (opl_write(reg,val), opl_detect()
 * returns 1)").  These replace src/OPLREG.C's own port-I/O-based
 * opl_register_write/opl_detect outright (game_funcs.h's "provided by
 * services" list drops src/OPLREG.C's opl_detect() for exactly this
 * reason); the Wave 4 driver implements them against Nuked-OPL3 or a real
 * device.  Stubs: portable/audio/sound_stub.c. ---- */
void opl_write(dos_uint reg, dos_uint val);
dos_int opl_detect(void);

#endif /* PORTABLE_SOUND_H */
