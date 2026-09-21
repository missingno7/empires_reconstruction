/* src/TIMER.C: Timer: INT 8 install/restore, handler and tick helpers.
   One translation unit; the sections below were the separate member
   sources of grouped module C_6B7A_6C87 and keep their original ids. */

/* ---- F_6B7A (original code at 0x6B7A) ---- */
/* F_6B7A -- install the timer interrupt and program the PIT divisor. */
extern void interrupt (*int8_saved_vector)(void);   /* saved INT 8 vector: offset at DS:0B7A, segment at DS:0B7C */
extern void interrupt timer_irq_handler(void);      /* F_6BCF below, this unit's INT 8 handler */
void timer_irq_install()
{
    asm push ax
    asm push dx
    asm push ds
    asm push es
    asm mov ax,3508h
    asm int 21h
    asm mov ax,es
    asm mov word ptr int8_saved_vector+2,ax
    asm mov word ptr int8_saved_vector,bx
    asm mov dx,offset timer_irq_handler
    asm push cs
    asm pop ds
    asm mov ax,2508h
    asm int 21h
    asm mov al,36h
    asm out 43h,al
    asm mov ax,13b1h
    asm out 40h,al
    asm mov al,ah
    asm out 40h,al
    asm mov al,0b6h
    asm out 43h,al
    asm pop es
    asm pop ds
    asm pop dx
    asm pop ax
}


/* ---- F_6BAC (original code at 0x6BAC) ---- */
/* F_6BAC -- restore the timer interrupt and reset the PIT divisor. */
extern void interrupt (*int8_saved_vector)(void);   /* saved INT 8 vector: offset at DS:0B7A, segment at DS:0B7C */
void timer_irq_restore()
{
    asm push ax
    asm push dx
    asm push ds
    asm push es
    asm mov dx,word ptr int8_saved_vector
    asm mov ax,word ptr int8_saved_vector+2
    asm mov ds,ax
    asm mov ax,2508h
    asm int 21h
    asm mov al,36h
    asm out 43h,al
    asm xor ax,ax
    asm out 40h,al
    asm mov al,ah
    asm out 40h,al
    asm pop es
    asm pop ds
    asm pop dx
    asm pop ax
}


/* ---- F_6BCF (original code at 0x6BCF) ---- */
/* Timer IRQ0 handler (original F_6BCF): Turbo C interrupt ABI.
   Every 13th tick calls the saved BIOS INT 8 vector; every tick advances the
   32-bit tick counter and services the sound engine unless a request is
   pending.  The bare PUSHF/POPF around the body is the one inline fragment:
   the historical object keeps the caller flags across the STI. */
extern int near timer_tick_phase,sound_request_count,sound_enabled,music_enabled;
extern unsigned long near timer_ticks;                    /* 32-bit tick counter */
extern void interrupt (* near int8_saved_vector)(void);         /* saved INT 8 vector */
extern void sound_tick_entry(void);                            /* sound engine tick */
void __sti__(void);
void __outportb__(int,unsigned char);
void interrupt timer_irq_handler(void)
{
 asm pushf;
 __sti__();
 ++timer_tick_phase; if(timer_tick_phase>=13) {timer_tick_phase=0;int8_saved_vector();}
 ++timer_ticks;
 if(!sound_request_count && (sound_enabled || music_enabled)) sound_tick_entry();
 __outportb__(0x20,0x20);
 asm popf;
}

extern unsigned long timer_ticks;              /* DS:0B76, high word at DS:0B78 */
extern unsigned long gc0d0;             /* DS:C0D0, high word at DS:C0D2 */

/* ---- F_6C26 (original code at 0x6C26) ---- */
/* F_6C26 -- spin until the 32-bit counter at DS:0B76 has advanced by n.
   The comparison is `jb` twice, so both sides are UNSIGNED long
   (tc20-codegen rule 5); the parameter is widened with `cwd`, so IT is a
   signed int.  The loop body is empty, which is why the jump to the test is
   the zero-displacement EB00. */
void timer_wait_ticks(n)
int n;
{
    unsigned long t;

    t = n + timer_ticks;
    while (timer_ticks < t) ;
}


/* ---- F_6C57 (original code at 0x6C57) ---- */
/* F_6C57 -- arm a tick deadline.  timer_ticks is the free-running tick (unsigned
   long), gc0d0 the deadline.  Plain C. */
void timer_deadline_arm(int n)
{
    gc0d0 = n + timer_ticks;
}


/* ---- F_6C6F (original code at 0x6C6F) ---- */
/* F_6C6F -- spin until the deadline armed by timer_deadline_arm has passed.  The EB 00 at
   the entry is the while-loop's jump-to-test with an empty body. */
void timer_deadline_wait(void)
{
    while (timer_ticks < gc0d0)
        ;
}


/* ---- F_6C87 (original code at 0x6C87) ---- */
/* F_6C87 -- has the 32-bit counter at DS:0B76 reached the deadline at
   DS:C0D0?  `ja`/`jb`/`jae` on the two halves makes both sides UNSIGNED long
   (rule 5), and one convention name reaches each high word as `_sym+2`
   (rule 18).  Both arms END in a jump to the epilogue -- the second one is
   the zero-displacement EB00 -- so both are `return` statements of an
   `if`; a single `return (a < b ? 0 : 1)` lets the second arm fall through
   and loses the EB00. */
int timer_deadline_reached()
{
    if (timer_ticks < gc0d0)
        return (0);
    return (1);
}
