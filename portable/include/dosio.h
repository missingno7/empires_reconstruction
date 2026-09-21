/* dosio.h -- save-slot file I/O and the BIOS tick clock, over stdio.
 *
 * tu-porting-rules.md sec 4: "open/read/write/close/lseek on the resource
 * files are already inside portable/resource.  Save-slot file I/O ->
 * dosio.h (dosio_open_read, dosio_read, dosio_write, dosio_seek,
 * dosio_close) with explicit error returns; disk_reset_retry, harderr,
 * hardresume, hardretn, dos_critical_error_* -> delete the call and route
 * the read-vs-write policy through dosio.h instead"; sec 4 also maps
 * `biostime(0,0)` (only in `srand(biostime(...))`, src/GAME.C) to
 * `dosio_bios_ticks()`.
 *
 * Every function returns -1 on failure, mirroring the historical DOS
 * wrappers' convention (src/RESOURCE.C's open/read/write/close call sites
 * all branch on a negative return).
 */
#ifndef PORTABLE_DOSIO_H
#define PORTABLE_DOSIO_H

#include "dos_types.h"

#define DOSIO_SEEK_SET 0
#define DOSIO_SEEK_CUR 1
#define DOSIO_SEEK_END 2

/* Open `path` for reading / for writing (create/truncate).  Returns a
 * small non-negative handle, or -1 on failure. */
int dosio_open_read(const char *path);
int dosio_open_write(const char *path);

/* Read/write up to `n` bytes.  Returns the byte count actually
 * transferred, or -1 on failure (never a short count on success, matching
 * the historical wrappers' blocking-stdio semantics). */
dos_long dosio_read(int handle, void *buf, dos_long n);
dos_long dosio_write(int handle, const void *buf, dos_long n);

/* fseek-style positioning (DOSIO_SEEK_*, matching SEEK_SET/CUR/END).
 * Returns the new absolute offset, or -1 on failure. */
dos_long dosio_seek(int handle, dos_long offset, int whence);

/* Returns 0 on success, -1 on failure (e.g. already closed / bad handle). */
int dosio_close(int handle);

/* 18.2 Hz ("BIOS tick") count since midnight, from the host clock -- the
 * portable replacement for `biostime(0,0)` in `srand(biostime(0,0L))`
 * (src/GAME.C).  1573040 = 24 * 60 * 60 * 1000 / 54.9254... -- computed
 * here as (seconds-since-midnight * 1000) / 54925 4ths (the historical
 * ratio, 1193182 / 65536 ~= 18.20648 Hz -> the DOS BIOS tick period is
 * 65536 / 1193182 s = 0.0549254... s = 54.9254... ms per tick). */
dos_ulong dosio_bios_ticks(void);

#endif /* PORTABLE_DOSIO_H */
