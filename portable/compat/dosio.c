/* dosio.c -- save-slot file I/O and the BIOS tick clock, over stdio.
 * See dosio.h for the mapping rationale.
 */
#include "dosio.h"

#include <stdio.h>
#include <time.h>

#define DOSIO_MAX_HANDLES 32

static FILE *s_handles[DOSIO_MAX_HANDLES];

static int alloc_handle(FILE *f)
{
    int i;
    for (i = 0; i < DOSIO_MAX_HANDLES; i++) {
        if (s_handles[i] == NULL) {
            s_handles[i] = f;
            return i;
        }
    }
    fclose(f);
    return -1;
}

static FILE *handle_file(int handle)
{
    if (handle < 0 || handle >= DOSIO_MAX_HANDLES) {
        return NULL;
    }
    return s_handles[handle];
}

int dosio_open_read(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        return -1;
    }
    return alloc_handle(f);
}

int dosio_open_write(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        return -1;
    }
    return alloc_handle(f);
}

dos_long dosio_read(int handle, void *buf, dos_long n)
{
    FILE *f = handle_file(handle);
    size_t got;
    if (!f || n < 0) {
        return -1;
    }
    got = fread(buf, 1, (size_t)n, f);
    if (got < (size_t)n && ferror(f)) {
        return -1;
    }
    return (dos_long)got;
}

dos_long dosio_write(int handle, const void *buf, dos_long n)
{
    FILE *f = handle_file(handle);
    size_t put;
    if (!f || n < 0) {
        return -1;
    }
    put = fwrite(buf, 1, (size_t)n, f);
    if (put < (size_t)n) {
        return -1; /* write failures fail outright, per tu-porting-rules.md sec 4 */
    }
    return (dos_long)put;
}

dos_long dosio_seek(int handle, dos_long offset, int whence)
{
    FILE *f = handle_file(handle);
    int w;
    long pos;
    if (!f) {
        return -1;
    }
    switch (whence) {
        case DOSIO_SEEK_SET: w = SEEK_SET; break;
        case DOSIO_SEEK_CUR: w = SEEK_CUR; break;
        case DOSIO_SEEK_END: w = SEEK_END; break;
        default: return -1;
    }
    if (fseek(f, (long)offset, w) != 0) {
        return -1;
    }
    pos = ftell(f);
    if (pos < 0) {
        return -1;
    }
    return (dos_long)pos;
}

int dosio_close(int handle)
{
    FILE *f = handle_file(handle);
    int rc;
    if (!f) {
        return -1;
    }
    rc = fclose(f);
    s_handles[handle] = NULL;
    return rc == 0 ? 0 : -1;
}

dos_ulong dosio_bios_ticks(void)
{
    /* 18.2 Hz "BIOS tick" count since midnight -- the exact PIT-derived
     * ratio (1193182 / 65536 Hz, same divisor family as timer.h's
     * TIMER_TICK_HZ) applied to the host wall clock's seconds-since-
     * midnight.  This is used exactly once historically, to seed the RNG
     * (`srand(biostime(0,0L))`, src/GAME.C) -- an approximation of the
     * real DOS BIOS tick counter is all that call site needs. */
    time_t t = time(NULL);
    struct tm tmv;
    double secs_since_midnight;
    double ticks;

#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif

    secs_since_midnight = tmv.tm_hour * 3600.0 + tmv.tm_min * 60.0 + tmv.tm_sec;
    ticks = secs_since_midnight * (1193182.0 / 65536.0);
    return (dos_ulong)ticks;
}
