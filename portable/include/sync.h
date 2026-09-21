/* sync.h -- minimal threading primitives for the portable tree.
 *
 * The game thread, the 236.7 Hz timer thread and the SDL main thread need a
 * mutex, a condition variable and a thread handle.  Implemented over Win32 in
 * portable/compat/sync_win32.c (other hosts: pthreads, later).  No SDL here:
 * portable/game must stay SDL-free.
 */
#ifndef PORTABLE_SYNC_H
#define PORTABLE_SYNC_H

#include <stdbool.h>
#include <stdint.h>

typedef struct sync_mutex { void *impl; } sync_mutex;
typedef struct sync_cond  { void *impl; } sync_cond;
typedef struct sync_thread { void *impl; } sync_thread;

void sync_mutex_init(sync_mutex *m);
void sync_mutex_destroy(sync_mutex *m);
void sync_mutex_lock(sync_mutex *m);
void sync_mutex_unlock(sync_mutex *m);

void sync_cond_init(sync_cond *c);
void sync_cond_destroy(sync_cond *c);
void sync_cond_wait(sync_cond *c, sync_mutex *m);              /* m must be held */
bool sync_cond_wait_ms(sync_cond *c, sync_mutex *m, uint32_t ms); /* false on timeout */
void sync_cond_signal(sync_cond *c);
void sync_cond_broadcast(sync_cond *c);

typedef void (*sync_thread_fn)(void *arg);
bool sync_thread_start(sync_thread *t, sync_thread_fn fn, void *arg);
void sync_thread_join(sync_thread *t);

/* Monotonic wall clock in nanoseconds and a sleep helper for the tick thread. */
uint64_t sync_now_ns(void);
void     sync_sleep_ns(uint64_t ns);

#endif
