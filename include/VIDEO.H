/* Runtime graphics/blit primitives implemented in asm/RUNTIME_BLOCK.ASM.
 * Declarations are intentionally unprototyped (old-style K&R), matching the
 * majority of the hundreds of local `extern void gfx_x();` declarations this
 * header replaces. An explicit typed prototype changes argument conversion
 * at call sites (char/int promotion, near/far pointers, long), which would
 * risk altering generated code away from byte-exact; a few caller modules
 * kept a typed local prototype instead of this header (see exceptions below)
 * because probing showed their reconstructed bytes depend on it.
 *
 * Exceptions (local typed declaration kept, NOT replaced by this header):
 *   src/HUD.C       - gfx_clear_rect(int,int,int,int); gfx_bar(int,int,int);
 *                      gfx_blit_bitmap(char near *,int,char far *)
 *   src/BOARDDRW.C  - gfx_copy_rect(unsigned,unsigned,void far *,int)
 *   src/INTRO.C     - gfx_copy_rect(int,int,void far *,int) (local variant)
 *   src/SLOTMENU.C  - gfx_bar(int,int,int)
 *   src/HUD.C       - gfx_vline(int,int,int)
 *   src/VIDEO.C     - gfx_vline(int,int,int)
 *   src/ROUNDEND.C  - gfx_blit_image(int,int,char far *)
 *   src/HITTEST.C   - gfx_get_pixel() kept locally as `extern int` (return
 *                      value is used); harmless duplicate of this header's
 *                      own `extern int` declaration.
 */
#ifndef VIDEO_H
#define VIDEO_H

/* F_039F: nested-rect corner/edge outline primitive ("box"). */
extern void gfx_box();

/* F_03A2: single-row planar span writer, nibble-clipped at both ends
 * ("bar" alias, 98+ call sites; used for 1-pixel-tall horizontal bars). */
extern void gfx_bar();

/* F_03A8: planar rectangle clear (nibble-clipped columns + full-byte rows,
 * AND/OR merge against a caller color). */
extern void gfx_clear_rect();

/* F_03AB: planar rectangle XOR fill (inverts pixels under the rect). */
extern void gfx_fill_rect();

/* F_03B4: planar rectangle copy, word-aligned interior columns
 * (rep movsw + trailing movsb); "wipe" alias, ~98 call sites. */
extern void gfx_wipe_rect();

/* F_03CC: clipped source-rectangle copy from a 32-byte bitmap header,
 * clamped against the board viewport at ds:[94h]/ds:[96h] ("copy" alias). */
extern void gfx_copy_rect();

/* F_03C9: planar bitmap blit from a 32-byte source header, word-aligned
 * interior columns, identical copy loop shape to gfx_wipe_rect ("blit"
 * alias). */
extern void gfx_blit_bitmap();

/* F_03A5: single-column planar span writer, nibble-clipped like gfx_bar but
 * vertical ("vline"; x,y,height). Called by DIALOG.C/HUD.C/MENULIST.C/
 * PUZZLE.C/VIDEO.C for dialog borders and HUD frame edges. */
extern void gfx_vline();

/* F_03AE: copies a screen rectangle out of VRAM into a caller buffer,
 * storing the buffer's (rows,cols) header first (x,y,w,h,buf,rows).
 * Used by DIALOG.C to save the pixels a dialog is about to cover. */
extern void gfx_save_rect();

/* F_03B1: inverse of gfx_save_rect -- copies a previously captured buffer
 * (with its own (rows,cols) header) back onto the screen at (x,y). Used by
 * DIALOG.C to restore the pixels behind a dismissed dialog. */
extern void gfx_restore_rect();

/* F_03B7: screen-to-screen rectangle copy with the destination rows written
 * bottom-to-top (vertical flip), byte order otherwise unchanged. Paired
 * with gfx_copy_rect_flip_h in MRKCELL.C/PUZZLE.C/SCOREPNL.C tile draws. */
extern void gfx_copy_rect_flip_v();

/* F_03BA: screen-to-screen rectangle copy with each source byte's nibbles
 * swapped and destination columns written right-to-left (horizontal
 * mirror), rows top-to-bottom. Used by LEVEL.C/MRKCELL.C/PUZZLE.C/
 * SCOREPNL.C/SLOTMENU.C for mirrored tile variants. */
extern void gfx_copy_rect_flip_h();

/* F_03BD: gfx_copy_rect_flip_h combined with gfx_copy_rect_flip_v -- nibble
 * swap plus both row and column order reversed (180-degree mirror). Used
 * by PUZZLE.C for the remaining rotated tile variant. */
extern void gfx_copy_rect_flip_hv();

/* F_03C0: splits each source byte's two nibbles into two separate output
 * rows (0A0h stride apart), i.e. a planar de-interleave/reflect copy;
 * rows written top-down. Used by SCORE.C for one score-digit transition. */
extern void gfx_copy_rect_split();

/* F_03C3: gfx_copy_rect_split with destination rows written bottom-to-top
 * and columns right-to-left (mirrored). Used by SCORE.C for the opposite
 * score-digit transition. */
extern void gfx_copy_rect_split_flip_v();

/* F_03C6: draws one glyph from the current font (metrics/data tables at
 * ds:[0C0DEh..0C0E8h]) at (x,y) and returns its pixel advance width.
 * Used by TXTWRAP.C to lay out a wrapped string one character at a time. */
extern int gfx_draw_char();

/* F_03CF: draws a packed/RLE image (source pointer with an embedded run
 * table, same per-pixel dispatch machinery as gfx_draw_char) at (x,y).
 * Used by ROUNDEND.C to paint the round-end score panel artwork. */
extern void gfx_blit_image();

/* F_03D2: writes the current color (ds:[40C8h]) into a single planar pixel
 * at (x,y), merging into whichever nibble the x parity selects. Used by
 * HITTEST.C to mark a hit-tested board cell. */
extern void gfx_set_pixel();

/* F_03D5: reads a single planar pixel at (x,y) back out as a normalized
 * nibble value. Used by HITTEST.C's raycast hit test to classify what was
 * drawn under the cursor. */
extern int gfx_get_pixel();

#endif
