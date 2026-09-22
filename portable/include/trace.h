/* trace.h -- opt-in bring-up tracing (EMPIRES_TRACE=1 in the environment). */
#ifndef PORTABLE_TRACE_H
#define PORTABLE_TRACE_H
#include <stdio.h>
#include <stdlib.h>
static inline int empires_trace_enabled(void)
{
    static int state = -1;
    if (state < 0)
        state = getenv("EMPIRES_TRACE") != NULL;
    return state;
}
#define EMPIRES_TRACE(...) do { if (empires_trace_enabled()) { fprintf(stderr, "[trace] " __VA_ARGS__); fputc('\n', stderr); } } while (0)
#endif
