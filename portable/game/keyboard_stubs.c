/* keyboard_stubs.c -- stand-ins for not-yet-ported subsystems that
 * portable/game/keyboard.c calls into.
 *
 * menu_list_active (src/MENULACT.C) and menu_loop_run (src/MENULOOP.C)
 * used to be stubbed here; portable/game/menulact.c and
 * portable/game/menuloop.c now supply the real definitions, so both were
 * removed from this file to avoid a duplicate-definition link error (same
 * pattern as sound_effects_toggle's removal, noted below).
 *
 * sound_effects_toggle (src/SNDFXTGL.C's port) used to be stubbed here;
 * portable/game/sndfxtgl.c now supplies the real definition, so it was
 * removed from this file to avoid a duplicate-definition link error.
 *
 * Nothing left to stub right now; the typedef below only keeps this
 * translation unit non-empty (MSVC C4206) until the next stub lands.
 */
typedef int keyboard_stubs_translation_unit_not_empty;
