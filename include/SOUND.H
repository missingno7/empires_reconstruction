#ifndef EMPIRES_SOUND_H
#define EMPIRES_SOUND_H

/* asm/SOUND.ASM's sound-effects/music-stream state block, DS:175E..1E94.
   See docs/current/sound-state.md for the full field map and the
   evidence behind each name.  Only fields whose declared type (and, for the
   data that a C caller already touches, whose spelling) is consistent across
   every current consumer are declared here; the rest stay local/ASM-internal
   -- see the comments below for why each one was left out. */

/* Loaded sound-resource block: two far pointers split across DS words, set
   once by src/PLRLDPUB.C (resource block and 0x620-byte staging block). */
extern unsigned snd_seg;        /* DS:1760 -- resource block segment */
extern unsigned snd_seg2;       /* DS:1764 -- staging block segment */
extern unsigned snd_base;       /* DS:175E -- offset half of the resource pointer */
extern unsigned snd_base2;      /* DS:1762 -- staging-block offset */

/* Master enable flags (options menu + timer gate). */
extern int sound_enabled;       /* DS:176E */
extern int music_enabled;       /* DS:1772 */

/* Sound-effects engine state (up to 4 concurrent "voices"). */
extern int snd_on;              /* DS:1770 -- a stream is armed/playing */
extern int mus_flag;            /* DS:1774 -- keep auto-chaining the music stream (INTRO.C/LEVEL.C set it around a stream start) */
extern int snd_flag2;           /* DS:1776 -- gates the pump loop's cursor reset to the stream base; the UI code
                                    sets it to 1 while a screen is up and clears it in the chapter loop */
extern int snd_backend_mode;    /* DS:1778 -- 0=PC speaker,1=Tandy gate,2=queued OPL,3=probe-only */
extern int snd_nvoices;         /* DS:177A -- 1 or 4 */
extern int v_b[4];              /* DS:177C -- per-voice output-suppressed gate */
extern int snd_mode;            /* DS:1784 -- 0=normal pump, 2=already primed */
extern int snd_hi;              /* DS:1786 -- currently-loaded voice-table id */
extern int g1788;               /* DS:1788 -- scratch base half of a base+percent pair that
                                    feeds v_ctr; naming kept neutral (see SNDSTATE.md's
                                    "level vs delay" caveat) */
extern int g178a;               /* DS:178A -- scratch percent-scaled delta half of the pair */

extern int voice_stream_cursor_table[4]; /* DS:178C -- live per-voice command cursor (was dv/g178c) */
extern int voice_stream_base_table[4];   /* DS:1794 -- reload-time baseline for the cursor (was sv) */
extern int v_ctr[4];            /* DS:179C -- per-voice hold/duration counter */
extern int g17a4[4];            /* DS:17A4 -- source value latched into v_ctr on decode */
extern int v_a[4];              /* DS:17AC -- per-voice armed flag */
extern int v_hold[4];           /* DS:17B4 -- per-voice sustain flag */
extern int v_len[4];            /* DS:17BC -- per-voice note length */
/* DS:17C4/17CC/17D4/17DC (pause-overlay renderer cursor/mode/override) and
   DS:17F4 (small decode index) are left as SOUND.ASM-internal state: their
   mechanism is understood but their exact purpose is not, and no C
   translation unit reaches them.
   DS:17E4 (voice_pending_table) and DS:1766 (voice_retune_base_table) have
   no data symbol at all -- they stay plain equates in asm/SOUND.ASM. */
extern int voice_rest_table[4]; /* DS:17EC -- per-voice last-command-was-rest flag */

extern unsigned notetab[12];    /* DS:17FC -- note -> PIT divisor, one octave */
extern unsigned opl_port;       /* DS:1830 -- OPL/backend I/O port */

/* Secondary "music stream" cluster (single stream, no per-voice indexing). */
extern int g1e84;               /* DS:1E84 -- scalar twin of g1788 */
extern int g1e86;               /* DS:1E86 -- scalar twin of g178a */
extern int mus_ptr;             /* DS:1E88 -- cursor into the stream's command bytes */
extern int mus_arg;             /* DS:1E8A -- armed cue index / priority key */
extern int snd_len;             /* DS:1E8E -- scalar twin of v_len */
extern int snd_delay;           /* DS:1E90 -- scalar twin of v_ctr */
extern int stream_note_delay;   /* DS:1E92 -- default/fallback note delay */
extern int snd_one;             /* DS:1E94 -- scalar twin of v_hold */

#endif
