/* asm_rectq.h -- rect-invalidation queue record layout, asm/RECTQ.ASM
 * (`_rect_queue_flush`, F_1ECD).
 *
 * Each queued record is 4 bytes (asm/RECTQ.ASM's own header comment):
 *   byte0 = x/2   byte1 = y   byte2 = w/2   byte3 = h
 * (only x and w are halved; the consumer doubles them back before calling
 * gfx_box(x,y,w,h)).  Records are appended at rect_queue_write_ptr by the
 * gfx_* wipe/blit/copy primitives (portable/gfx/gfx_planar.c,
 * portable/gfx/gfx_vga.c) whenever gbc==1, and drained by
 * rect_queue_flush() (portable/game/asm_rectq.c) between the read cursor
 * ui_gfx_blob and the write cursor rect_queue_write_ptr.
 *
 * Storage: there is NO dedicated DGROUP buffer for this queue.
 * ui_gfx_blob (DS:C5CA, portable/resource/archive.c) is the resource
 * subsystem's decode-staging pointer; the rect queue reuses whatever
 * memory it points into (see the port report for the full trail through
 * src/GAME.C, src/BOARD.C, src/LEVEL.C, which reset
 * `rect_queue_write_ptr = ui_gfx_blob;` before every queueing round).
 */
#ifndef PORTABLE_ASM_RECTQ_H
#define PORTABLE_ASM_RECTQ_H

#include "dos_types.h"

#define RECT_QUEUE_RECORD_BYTES 4u

#endif /* PORTABLE_ASM_RECTQ_H */
