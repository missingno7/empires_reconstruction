/* trace.h -- opt-in bring-up tracing (EMPIRES_TRACE=1 in the environment).
 *
 * Every EMPIRES_TRACE() site also publishes itself as a "marker" (the format
 * string) so scripted-input drivers can synchronise on the game's high-level
 * flow (--script "@slot_menu_run,...").
 */
#ifndef PORTABLE_TRACE_H
#define PORTABLE_TRACE_H

extern volatile unsigned empires_trace_seq;
extern const char *volatile empires_trace_last;

/* Records the marker, then prints when EMPIRES_TRACE is set. */
void empires_trace(const char *fmt, ...);

#define EMPIRES_TRACE(...) empires_trace(__VA_ARGS__)

#endif
