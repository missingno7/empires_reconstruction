/* Timer IRQ0 handler (original F_6BCF): Turbo C interrupt ABI.
   Every 13th tick calls the saved BIOS INT 8 vector; every tick advances the
   32-bit tick counter and services the sound engine unless a request is
   pending.  The bare PUSHF/POPF around the body is the one inline fragment:
   the historical object keeps the caller flags across the STI. */
extern int near timer_tick_phase,g237c,sound_enabled,music_enabled;
extern unsigned long near timer_ticks;                    /* 32-bit tick counter */
extern void interrupt (* near int8_saved_vector)(void);         /* saved INT 8 vector */
extern void fc1a0(void);                            /* sound engine tick */
void __sti__(void);
void __outportb__(int,unsigned char);
void interrupt timer_irq_handler(void)
{
 asm pushf;
 __sti__();
 ++timer_tick_phase; if(timer_tick_phase>=13) {timer_tick_phase=0;int8_saved_vector();}
 ++timer_ticks;
 if(!g237c && (sound_enabled || music_enabled)) fc1a0();
 __outportb__(0x20,0x20);
 asm popf;
}
