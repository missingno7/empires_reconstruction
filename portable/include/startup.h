/* startup.h -- prototypes for portable/game/startup.c (src/STARTUP.C /
 * src/VIDEO.C's video_set_text_mode), the supervisor-written replacement
 * for the historical BIOS/DOS boot probes -- see tu-porting-rules.md sec 5:
 * "video_set_text_mode, bios_equipment_probe, video_adapter_detect,
 * sound_backend_probe, cmdline_parse_args, video_mode_select | replaced by
 * portable/game/startup.c (supervisor-written)".
 *
 * Not every historical function in that row is exposed here: only the ones
 * called from outside startup.c itself (game.c's game_main/cmdline
 * wiring).  bios_equipment_probe/video_adapter_detect/sound_backend_probe
 * stay startup.c-internal, called only from video_mode_select().
 */
#ifndef PORTABLE_STARTUP_H
#define PORTABLE_STARTUP_H

#include "dos_types.h"

dos_int video_mode_select(void);
void    video_set_text_mode(void);
void    startup_set_args(int argc, char **argv);
void    cmdline_parse_args(void);

#endif /* PORTABLE_STARTUP_H */
