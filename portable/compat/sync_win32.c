/* sync_win32.c -- sync.h over Win32 SRW locks / condition variables. */
#include "sync.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

void sync_mutex_init(sync_mutex *m)
{
    SRWLOCK *l = (SRWLOCK *)malloc(sizeof *l);
    InitializeSRWLock(l);
    m->impl = l;
}
void sync_mutex_destroy(sync_mutex *m) { free(m->impl); m->impl = NULL; }
void sync_mutex_lock(sync_mutex *m)    { AcquireSRWLockExclusive((SRWLOCK *)m->impl); }
void sync_mutex_unlock(sync_mutex *m)  { ReleaseSRWLockExclusive((SRWLOCK *)m->impl); }

void sync_cond_init(sync_cond *c)
{
    CONDITION_VARIABLE *cv = (CONDITION_VARIABLE *)malloc(sizeof *cv);
    InitializeConditionVariable(cv);
    c->impl = cv;
}
void sync_cond_destroy(sync_cond *c) { free(c->impl); c->impl = NULL; }
void sync_cond_wait(sync_cond *c, sync_mutex *m)
{
    SleepConditionVariableSRW((CONDITION_VARIABLE *)c->impl, (SRWLOCK *)m->impl, INFINITE, 0);
}
bool sync_cond_wait_ms(sync_cond *c, sync_mutex *m, uint32_t ms)
{
    return SleepConditionVariableSRW((CONDITION_VARIABLE *)c->impl, (SRWLOCK *)m->impl, ms, 0) != 0;
}
void sync_cond_signal(sync_cond *c)    { WakeConditionVariable((CONDITION_VARIABLE *)c->impl); }
void sync_cond_broadcast(sync_cond *c) { WakeAllConditionVariable((CONDITION_VARIABLE *)c->impl); }

struct thread_start { sync_thread_fn fn; void *arg; };

static DWORD WINAPI thread_trampoline(LPVOID p)
{
    struct thread_start s = *(struct thread_start *)p;
    free(p);
    s.fn(s.arg);
    return 0;
}

bool sync_thread_start(sync_thread *t, sync_thread_fn fn, void *arg)
{
    struct thread_start *s = (struct thread_start *)malloc(sizeof *s);
    s->fn = fn;
    s->arg = arg;
    HANDLE h = CreateThread(NULL, 0, thread_trampoline, s, 0, NULL);
    if (!h) {
        free(s);
        t->impl = NULL;
        return false;
    }
    t->impl = h;
    return true;
}

void sync_thread_join(sync_thread *t)
{
    if (t->impl) {
        WaitForSingleObject((HANDLE)t->impl, INFINITE);
        CloseHandle((HANDLE)t->impl);
        t->impl = NULL;
    }
}

uint64_t sync_now_ns(void)
{
    static LARGE_INTEGER freq;
    LARGE_INTEGER now;
    if (freq.QuadPart == 0)
        QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    /* split to avoid overflow: seconds * 1e9 + remainder * 1e9 / freq */
    uint64_t sec = (uint64_t)now.QuadPart / (uint64_t)freq.QuadPart;
    uint64_t rem = (uint64_t)now.QuadPart % (uint64_t)freq.QuadPart;
    return sec * 1000000000ull + rem * 1000000000ull / (uint64_t)freq.QuadPart;
}

#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

void sync_sleep_ns(uint64_t ns)
{
    /* Sleep() granularity is the scheduler quantum (up to ~15 ms), far too
     * coarse for a 4.2 ms tick.  Use a high-resolution waitable timer
     * (Windows 10 1803+); fall back to Sleep() when it is unavailable.  The
     * tick thread still spins the last fraction of a millisecond itself. */
    static __declspec(thread) HANDLE timer;
    static __declspec(thread) int timer_tried;
    if (ns == 0) {
        SwitchToThread();
        return;
    }
    if (!timer_tried) {
        timer_tried = 1;
        timer = CreateWaitableTimerExW(NULL, NULL, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
    }
    if (timer) {
        LARGE_INTEGER due;
        due.QuadPart = -(LONGLONG)((ns + 99) / 100);   /* relative, 100 ns units */
        if (SetWaitableTimer(timer, &due, 0, NULL, NULL, FALSE)) {
            WaitForSingleObject(timer, INFINITE);
            return;
        }
    }
    DWORD ms = (DWORD)(ns / 1000000ull);
    if (ms == 0)
        SwitchToThread();
    else
        Sleep(ms);
}
#endif /* _WIN32 */
