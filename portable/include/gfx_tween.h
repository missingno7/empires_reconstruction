/* gfx_tween.h -- frame interpolation ("tweening") for the VGA presenter.
 *
 * The game renders one frame per timer_deadline_arm(24)..timer_deadline_wait()
 * window (~9.86 Hz at the historical 236.7 Hz tick) and never knows about
 * host refresh rates.  This module lets the presenter show the window at the
 * host's frame rate with the moving sprites drawn at positions interpolated
 * between the two most recent game frames, while the game logic, its tick
 * ratio, and every pixel it produces stay exactly as they are.
 *
 * How it works
 *   Capture (game thread, inside the VGA driver):  a blit made while
 *   gfx_tween_tag is nonzero -- game code sets the tag around the player and
 *   actor-record draws -- is recorded with its arguments, the clip words in
 *   force, a copy of the bitmap, and the pixels it is about to cover.  The
 *   driver draws exactly as before; the record is an observer.
 *   Publish (game thread, at timer_deadline_wait):  the presented VRAM, the
 *   op list and the frame's deadline are snapshotted as "frame n".
 *   Compose (presenter thread, every host frame):  start from frame n's
 *   VRAM, put back the covered pixels of every recorded op (reverse order),
 *   then redraw each op with the same driver code at
 *   lerp(position in frame n-1, position in frame n, alpha), where alpha is
 *   the host time's progress from frame n's publish to its deadline.  Ops
 *   are matched between frames by tag and order; a per-axis jump over
 *   GFX_TWEEN_SNAP_DISTANCE is a teleport and is not smoothed.
 *
 * Only the VGA (display selector 5) driver captures.  Disabled (the default
 * until the front end enables it), nothing here runs beyond a flag test.
 * Deterministic replays keep it disabled.
 */
#ifndef PORTABLE_GFX_TWEEN_H
#define PORTABLE_GFX_TWEEN_H

#include "dos_types.h"

/* ---- tagging (game code) ------------------------------------------------ */
extern int gfx_tween_tag;                       /* 0 = untagged; set around a tweenable draw */
#define GFX_TWEEN_TAG_PLAYER      1
#define GFX_TWEEN_TAG_INTRO       2             /* the intro's event-driven sprite */
#define GFX_TWEEN_TAG_ACTOR(i)    (0x100 + (int)(i))
/* Flashlight beam trail pixel by age (0 = newest of the 24, 8 new per
 * frame): the presenter grows the head and shortens the tail progressively. */
#define GFX_TWEEN_TAG_BEAM(age)   (0x200 + (int)(age))

#define GFX_TWEEN_SNAP_DISTANCE   48            /* per-axis pixels; larger = teleport */

/* ---- enable / configure (front end) ------------------------------------ */
void gfx_tween_set_enabled(bool enabled);       /* also resets all state */
bool gfx_tween_enabled(void);

/* Called by the presenter when interpolation presentation is resumed after
 * showing live VRAM.  This invalidates only presenter-side base/timing state;
 * it does not touch the game-thread capture engine or allocate/reset it. */
void gfx_tween_presenter_resume(void);

/* ---- driver hooks (gfx_vga.c) ----------------------------------------- */
struct vga_copy_clip;
void gfx_tween_capture_copy_rect(int tag, dos_int x, dos_int y, const uint8_t *bitmap,
                                 dos_int flip, const struct vga_copy_clip *clip);
void gfx_tween_capture_vline(int tag, dos_int x, dos_int y, dos_int n, uint8_t color);
void gfx_tween_capture_pixel(int tag, dos_int x, dos_int y, uint8_t color);

/* ---- frame boundary (game thread) ------------------------------------- */
/* Snapshot the presented VRAM plus the ops captured since the previous
 * publish as the newest frame.  `now_ms` is the host clock at publish,
 * `deadline_ms` when the game will draw the next one (the armed deadline,
 * or an event-driven animation's next event time), `vram_generation` the
 * gfx_vram_generation value at publish.  The front end calls this from the
 * timer frame observer (timer_set_frame_observer). */
void gfx_tween_frame_publish(double now_ms, double deadline_ms, uint32_t vram_generation);

/* ---- presenter (host thread) ------------------------------------------ */
/* Compose the interpolated 320x200 8bpp frame for host time `now_ms` into
 * `out`.  Returns false when the presenter should show the live VRAM
 * instead: nothing published yet, or the game has presented something
 * newer than the published frame and no new publish followed within the
 * hold window (a non-frame context such as a menu or dialog). */
bool gfx_tween_compose(uint8_t *out, double now_ms, uint32_t live_generation);

/* Counters for diagnostics: frames published, compositions returned,
 * compositions that actually interpolated (0 < alpha < 1 with a matching
 * previous frame), and live fallbacks. */
void gfx_tween_stats(unsigned *published, unsigned *composed, unsigned *interpolated, unsigned *live);

#endif /* PORTABLE_GFX_TWEEN_H */
