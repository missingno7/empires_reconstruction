/* Placeholder translation unit so empires_core is never empty. */
#include "dos_types.h"
int dos_compat_link_anchor(void) { return 0; }

/* trace.h markers and printer. */
#include "trace.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

volatile unsigned empires_trace_seq;
const char *volatile empires_trace_last;

void empires_trace(const char *fmt, ...)
{
    static int enabled = -1;
    empires_trace_last = fmt;
    empires_trace_seq++;
    if (enabled < 0)
        enabled = getenv("EMPIRES_TRACE") != NULL;
    if (enabled) {
        va_list ap;
        va_start(ap, fmt);
        fputs("[trace] ", stderr);
        vfprintf(stderr, fmt, ap);
        fputc('\n', stderr);
        va_end(ap);
    }
}
