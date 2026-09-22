/* port_options.h -- portable extension of the historical F3 Options menu.
 *
 * The generated DATA menu remains the recoverable historical source.  The
 * portable build installs this small runtime overlay into its mutable record
 * objects instead of changing generated DATA or regenerating historical
 * assets for a host-only UI addition.
 */
#ifndef PORTABLE_PORT_OPTIONS_H
#define PORTABLE_PORT_OPTIONS_H

#include "dos_types.h"

#define PORT_OPTIONS_ROW_COUNT 6

extern dos_char port_options_menu_text[];
extern void (*port_options_menu_callbacks[PORT_OPTIONS_ROW_COUNT])(void);

void port_options_install(void);

#endif /* PORTABLE_PORT_OPTIONS_H */
