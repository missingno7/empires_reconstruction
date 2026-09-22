/* port_debug.h -- opt-in portable developer/debug state. */
#ifndef PORTABLE_PORT_DEBUG_H
#define PORTABLE_PORT_DEBUG_H

#include "dos_types.h"

#include <stdbool.h>

void port_debug_init(bool enabled);
bool port_debug_enabled(void);

bool port_debug_unlimited_energy(void);
void port_debug_set_unlimited_energy(bool enabled);
dos_int port_debug_energy_delta(dos_int delta);

void port_debug_request_complete_chamber(void);
bool port_debug_take_complete_chamber_request(void);

void port_debug_install_menu(void);

extern dos_char port_debug_menu_text[];
extern void (*port_debug_menu_callbacks[2])(void);

#endif /* PORTABLE_PORT_DEBUG_H */
