/* port_settings.h -- portable runtime/configuration bridge for host options.
 *
 * Historical game state (music_enabled/sound_enabled) deliberately does not
 * live here.  This service owns only the portable output gains and the
 * presentation preference exposed by the portable Options overlay.
 */
#ifndef PORTABLE_PORT_SETTINGS_H
#define PORTABLE_PORT_SETTINGS_H

#include <stdbool.h>

/* Resolve the effective startup values without writing them back. */
bool port_settings_init(int music_volume, int sound_volume,
                        bool interpolation, const char *config_path);

int  port_settings_music_volume(void);
int  port_settings_sound_volume(void);
bool port_settings_interpolation(void);

/* Preview setters update the running process only. */
void port_settings_preview_music_volume(int percent);
void port_settings_preview_sound_volume(int percent);

/* Commit a menu choice to runtime state and the existing config store.  A
 * failed config update/save is reported to stderr, but the runtime choice is
 * retained and the functions still return to the menu. */
bool port_settings_commit_music_volume(int percent);
bool port_settings_commit_sound_volume(int percent);
bool port_settings_commit_interpolation(bool enabled);

#endif /* PORTABLE_PORT_SETTINGS_H */
