/* sound.h -- SOUND.ASM/OPL C-facing entry points the game calls.
 *
 * Phase 12 part 1: portable/audio/sound_driver.c now implements every
 * entry point this header declares (asm/SOUND.ASM's 41-routine game-owned
 * sound state machine, reimplemented as ordinary typed C -- no register
 * emulation).  This header only DECLARES the functions asm/SOUND.ASM
 * exposed to C, the backend event API hardware output now routes through,
 * and the two synthetic-memory arenas the driver addresses its command
 * streams through; it does not declare any DGROUP object -- every
 * SOUND.ASM/SOUND.H state object is generator-owned (game_data.h for the
 * initialized DATA span DS:175E..1E96, all of it now individually
 * addressable -- see below).  #include "game_state.h"/"game_data.h" (via
 * game.h) for the objects themselves; do not add `extern` redeclarations
 * of generated state here.
 *
 * ---------------------------------------------------------------------
 * Superseded GAP note: an earlier draft of this header reported that none
 * of include/SOUND.H's fields existed as individually-addressable symbols
 * (the whole DS:176E..1E96 span captured as one opaque blob).  That has
 * since been fixed by the generator: portable/generated/game_data.h now
 * declares sound_enabled, music_enabled, snd_on, mus_flag, snd_flag2,
 * snd_backend_mode, snd_nvoices, v_b/voice_stream_cursor_table/
 * voice_stream_base_table/v_ctr/v_a/v_hold/v_len/voice_rest_table[4],
 * g17f4[8], sound_region_17C4[40] (the LOW-confidence pause-overlay
 * renderer fields packed 5-tables-of-8-bytes; see
 * portable/audio/sound_driver_internal.h for the accessor layer),
 * notetab[12]/note_divisors_octave[24], opl_port, sound_dispatch_182C[2]/
 * sound_dispatch_1832[36] (both real C pointer arrays into constant note/
 * OPL-byte-stream tables), snd_base/snd_seg/snd_base2/snd_seg2, mus_ptr,
 * mus_arg, snd_len, snd_delay, stream_note_delay, snd_one, and
 * sound_request_count (DS:237C) -- all as individually addressable
 * symbols, plus sound_instrument_region/DATA_012C03_SOUND_INSTRUMENTS
 * (the OPL instrument-bank data OPLVOICE.C/VOXSLOAD.C already consume).
 * portable/audio/sound_stub.c's ad hoc globals are gone; game_data.c is
 * the sole definition of every one of these objects now.
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
 * reason).  Implementation: portable/audio/sound_driver.c. ---- */
void opl_write(dos_uint reg, dos_uint val);
dos_int opl_detect(void);

/* ---------------------------------------------------------------------
 * Backend event interface (architecture.md "Audio model": the driver
 * emits timestamped events instead of touching hardware directly).  Every
 * historical hardware write asm/SOUND.ASM performed becomes exactly one
 * call to one of the four hooks below:
 *
 *   sound_backend_opl_write(reg,val)     -- the OPL bus protocol
 *     (opl_write()/opl_register_write()'s register+data pair).
 *   sound_backend_pit_divisor(divisor)   -- PIT channel-2 divisor, port
 *     0x42 (the driver combines the historical lo-then-hi byte writes
 *     into one 16-bit event; _pit_channel2_set_divisor/the backend-mode-0
 *     path of _sound_pit_divisor_program).
 *   sound_backend_speaker_gate(enabled,tandy_mode) -- port 0x61 gate bits
 *     (tandy_mode==0: `or al,3`/`and al,0FCh`, the plain PC-speaker gate
 *     _speaker_gate_on/_off always use and backend-mode-0 voice_enable/
 *     disable use; tandy_mode==1: `or al,60h`, backend-mode-1
 *     voice_enable's Tandy/PCjr gate -- backend-mode-1 voice_disable does
 *     NOT use this port at all, see sound_backend_nibble_port_write).
 *   sound_backend_nibble_port_write(value) -- one raw byte to opl_port
 *     with no register/data split (_opl_port_write_byte: backend-mode-1's
 *     packed-nibble PIT-divisor emission, its voice reset byte, the
 *     paused-mode single-register writes in _sound_control_value_select,
 *     and the pause-overlay renderer _sound_control_block_advance).
 *
 * Default implementations (portable/audio/sound_driver.c) append to an
 * in-memory ring log of {tick, kind, a, b} the sound_event_log_* API
 * below reads back; real backends (Nuked-OPL3, a speaker synth) replace
 * these bodies in a later phase without changing this header's shape. */
enum sound_event_kind {
    SOUND_EVENT_OPL_WRITE = 0,
    SOUND_EVENT_PIT_DIVISOR,
    SOUND_EVENT_SPEAKER_GATE,
    SOUND_EVENT_NIBBLE_WRITE
};

struct sound_event {
    uint32_t tick; /* count of sound_tick_entry() calls at event time, 0-based */
    int kind;      /* enum sound_event_kind */
    uint16_t a;
    uint16_t b;
};

void sound_backend_opl_write(uint8_t reg, uint8_t val);
void sound_backend_pit_divisor(uint16_t divisor);
void sound_backend_speaker_gate(int enabled, int tandy_mode);
void sound_backend_nibble_port_write(uint8_t value);

void sound_event_log_clear(void);
size_t sound_event_log_count(void);
const struct sound_event *sound_event_log_get(size_t index);

/* ---------------------------------------------------------------------
 * Command-stream memory: two real pointers to the two historical far
 * -pointer targets, owned by this driver.
 *
 *   sound_resource_block -- what snd_seg:snd_base pointed at: the loaded
 *     "record 0x41" sound-resource block (portable/game/plrldpub.c's
 *     resource_ptr).  Addresses the single "music stream" cluster
 *     (mus_ptr, stream_control_block_arm's far-pointer-table lookup).
 *   sound_voice_block -- what snd_seg2:snd_base2 pointed at: gc5da, the
 *     0x620-byte staging block portable/game/rescache.c fills (via
 *     resource_load_record_into) before every sound_voice_table_reload()
 *     call.  Addresses the up-to-4-voice cluster
 *     (voice_stream_cursor_table/voice_stream_base_table).
 *
 * Every driver cursor into these blocks (mus_ptr, voice_stream_cursor_
 * table, ...) stays a plain 16-bit offset relative to the block's own
 * base, exactly like the historical ES:DI far-pointer addressing did
 * within one DOS segment -- snd_seg/snd_seg2 are never read for
 * addressing (portable/game/plrldpub.c leaves them at their generated
 * zero default, unwritten, same as snd_base/snd_base2, which have no
 * portable meaning once the far pointer is a real pointer).
 *
 * sound_set_resource_blocks() is how plrldpub.c publishes the two blocks
 * right after allocating/loading them; call it again whenever a loader
 * hands the driver a new resource_ptr/gc5da pair.  Both blocks start
 * NULL (no resource loaded yet -- the state a fresh boot, or any tick
 * serviced before player_record_load_publish() has run, is in): every
 * ES:DI-style byte read in sound_driver.c goes through a small accessor
 * that reads a NULL block as 0xFF, the SAME sentinel byte
 * sound_voice_table_prime()'s "is this the end marker" check and every
 * command-stream dispatcher's "0xF = terminator" opcode already use, so
 * ticking with nothing loaded is inert (no crash, no spin) instead of
 * undefined -- see portable/tests/test_sound.c's
 * "unarmed_tick_returns_promptly" test. */
extern uint8_t *sound_resource_block;
extern uint8_t *sound_voice_block;
void sound_set_resource_blocks(uint8_t *resource, uint8_t *staging);

/* ---------------------------------------------------------------------
 * Differential-oracle snapshot/restore (tools/portable/sound_oracle/,
 * added alongside the Unicorn-based asm/SOUND.ASM oracle; coordinate any
 * offset change with docs/portable/state-map.md and this header's own
 * SOUND_FIELD_TABLE twin in tools/portable/sound_oracle/emu.py).
 *
 * Copies every named sound-related DGROUP object (docs/current/
 * sound-state.md's field map, DS:175E..1E96 plus the scalar at DS:237C)
 * to/from a caller-owned SOUND_DRIVER_SNAPSHOT_SIZE-byte buffer laid out
 * in HISTORICAL byte order: buffer offset 0 == DS:175E, ...,
 * buffer offset 0x737 == DS:1E95 (span 0x738 bytes), buffer offset 0x738
 * == DS:237C (2 bytes, sound_request_count) -- total 0x73A (1850) bytes.
 *
 * Two historical spans inside that range are intentionally left alone by
 * both functions: DS:182C (sound_dispatch_182C, 4 bytes) and DS:1832
 * (sound_dispatch_1832, 72 bytes) are compile-time-constant POINTER
 * arrays here (real C pointers into the decoded lookup_XXXX blobs), not
 * byte-comparable with the historical raw DS-offset words asm/SOUND.ASM
 * itself stored there -- see sound_driver_internal.h's header comment.
 * sound_driver_snapshot() writes zero for those bytes (and for the
 * unrelated DS:187A..1E84 gap between the two sound clusters);
 * sound_driver_restore() ignores whatever the buffer holds there.
 *
 * These exist ONLY for the parity test (portable/tests/
 * test_sound_parity.c): normal driver operation never calls them. */
#define SOUND_DRIVER_SNAPSHOT_SIZE 1850
void sound_driver_snapshot(uint8_t *dgroup_image);
void sound_driver_restore(const uint8_t *dgroup_image);

#endif /* PORTABLE_SOUND_H */
