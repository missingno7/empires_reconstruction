/* src/GAME.C: Program lifecycle: main, game driver and shutdown.
   One translation unit; the sections below were the separate member
   sources of grouped module C_49E3_4A93 and keep their original ids. */

/* ---- F_49E3 (original code at 0x49E3) ---- */
extern void sound_stop_reset(), fc834(), f697d(), timer_irq_restore();
void game_shutdown(void) { sound_stop_reset(); fc834(); f697d(); timer_irq_restore(); }


/* ---- F_49F0 (original code at 0x49F0) ---- */
/* F_49F0 -- "THE GAME": the top-level sequencer that calls the intro driver
   intro_run_chapter (157C6) and then runs the per-level record sequence.  Entry 14AF0,
   163 bytes, 16 activations per cold boot.

   Disassembly (assets/AEPROG.EXE, ndisasm -b16 -o 0x49f0):
       55 8BEC 83EC02 56 57   push bp; mov bp,sp; sub sp,2; push si; push di
       E8 CB0C                call intro_run_chapter
       3D FFFF                cmp ax,-1        (accumulator short form)
       75 03 / E9 8A00        jnz +3 / jmp 4A8D  -- "if (...) return;"
       E8 46D8                call hud_arena_init        (0 args, result unused)
       1E B8FE8B 50           push ds; mov ax,0x8BFE; push ax  -- (char far *)g8bfe
       E8 45AF                call setjmp
       59 59                  pop cx; pop cx    (cdecl cleanup, 4 bytes = 2 pops)
       8BF8                   mov di,ax         -- d = setjmp(g8bfe)
       BEFFFF                 mov si,0xFFFF     -- s = -1
       E8 E327 / E8 988B / E8 3B48   ui_overlay_reset(); sound_request_count_clear(); puzzle_free_resources();
       83FF03 7502 EB68       cmp di,3; jnz +2; jmp 4A8D   -- if (d == 3) return;
       83FF02 7D4D            cmp di,2; jnl 4A77           -- if (d < 2) { ... }
       E8 3961 3DFFFF 7502 EB59   if (fab66() == -1) return;
       A1ED13 BA1B00 F7E2     mov ax,[13ED]; mov dx,0x1B; mul dx
       8BD8 81C370C4          mov bx,ax; add bx,0xC470
       1E 07                  push ds; pop es          -- far-pointer form:
                                                          (char far *)slot_table + i*0x1B
       268A4715 98 50         mov al,[es:bx+0x15]; cbw; push ax
       E8 F628 59             call energy_set; pop cx
       ... the same address recomputed, field +0x0C this time ...
       268A470C 98 8946FE     mov al,[es:bx+0xC]; cbw; mov [bp-2],ax
       0BC0 740D              or ax,ax; jz 4A77        -- if (n = ...) {
       FF4EFE                 dec word [bp-2]          --   n--;
       8B76FE 8BC6 50         mov si,[bp-2]; mov ax,si; push ax
                                                       --   f4943(s = n)
                                  (the assignment's VALUE is what is pushed:
                                   "s = n" loads si straight from memory, then
                                   the expression result goes through ax.  Two
                                   separate statements would have pushed si --
                                   see the loop body below, which does.)
       E8 CDFE 59             call f4943; pop cx
       EB0C                   jmp short 4A85           -- while (s != 4) {
       56 E8 EF87 59          push si; call player_select_run; pop cx
       8BF0                   mov si,ax                --   s = player_select_run(s);
       56 E8 BFFE 59          push si; call f4943; pop cx   -- f4943(s);
       83FE04 75EF            cmp si,4; jnz 4A79       -- }
       E8 9862                call fad25
       5F 5E 8BE5 5D C3       pop di; pop si; mov sp,bp; pop bp; ret

   The 27-byte stride at 0xC470 and the byte fields at +0x15 and +0x0C are
   the per-level record indexed by current_slot; the multiply is `mul` (unsigned),
   which is what TC 2.0 emits for `int * const` -- the same shape the already
   matched F_5AC3 carries at 5B35 (`mov dx,0x18; mul dx; mov bx,ax;
   add bx,0x900; push ds; pop es`). */

extern int  intro_run_chapter(), setjmp(), fab66(), player_select_run();
extern void hud_arena_init(), ui_overlay_reset(), sound_request_count_clear(), f4943(), fad25();
extern void puzzle_free_resources(void);
extern void energy_set(int state);

#include "C470.H"

extern char g8bfe[];                    /* DS:8BFE */

void game_run()
{
    int n;                              /* bp-2 */
    register int s, d;                  /* si, di */

    if (intro_run_chapter() == -1) return;
    hud_arena_init();
    d = setjmp(g8bfe);
    s = -1;
    ui_overlay_reset();
    sound_request_count_clear();
    puzzle_free_resources();
    if (d == 3) return;
    if (d < 2) {
        if (fab66() == -1) return;
        energy_set(slot_table[current_slot].state);
        if (n = slot_table[current_slot].byte12) {
            n--;
            f4943(s = n);
        }
    }
    while (s != 4) {
        s = player_select_run(s);
        f4943(s);
    }
    fad25();
}


/* ---- F_4A93 (original code at 0x4A93) ---- */
/* F_4A93 -- main().  Entry 14B93, 20 bytes.  A straight-line sequence of
   five calls: video_mode_select() gates whether the rest ever runs (init failure ->
   immediate return with no side effects on the four other calls), then
   boot_init_seed_rand (boot init), game_run (the game), game_shutdown (shutdown), video_set_text_mode (exit).
   Disassembly (assets/AEPROG.EXE, physical 14B93):
       E8 74 07        call video_mode_select          (1530A)
       0B C0            or  ax,ax
       74 0C            jz  14BA6           (the ret at the end)
       E8 70 FE        call boot_init_seed_rand          (14A0D)
       E8 50 FF        call game_run          (14AF0)
       E8 40 FF        call game_shutdown          (14AE3)
       E8 A9 B8        call video_set_text_mode          (1044F)
       C3               ret
   No MOV sets ax before the single ret, so no "return <value>" appears in
   the source.  An "if (x == 0) return;" early-return shape does NOT
   reproduce these bytes: TC 2.0's unoptimized codegen for that shape is the
   "jnz +2 / jmp +N" idiom (two branches, 4 bytes) rather than a single
   inverted jcc, which overflows this 20-byte extent by 2 bytes (confirmed
   by a first compile attempt, T00.OBJ, kept as the negative test). The
   ORIGINAL shape is a single guarded block with no early exit: */
extern int  video_mode_select();
extern void boot_init_seed_rand();
extern void game_run();
extern void game_shutdown();
extern void video_set_text_mode();

main()
{
    if (video_mode_select()) {
        boot_init_seed_rand();
        game_run();
        game_shutdown();
        video_set_text_mode();
    }
}
