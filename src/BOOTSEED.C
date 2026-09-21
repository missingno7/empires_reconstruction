/* F_490D -- boot init.  Entry 14A0D, 54 bytes.  A straight-line sequence
   of twelve calls, no branches.  Disassembly (assets/AEPROG.EXE):
       33C0 50 50 33C0 50   xor ax,ax; push ax; push ax; xor ax,ax; push ax
       E8 F8B0              call biostime          (1FB0F)   -- biostime(0, 0L):
                                                  the long/far arg is rightmost
                                                  (pushed first, one xor then
                                                  two pushes of the same zero
                                                  for its two halves), the int
                                                  arg is leftmost (pushed
                                                  second, its own fresh xor)
       83C406               add sp,6            (cdecl cleanup, 3 words)
       50                   push ax             (biostime's own return value)
       E8 E0AF              call srand          (1F9FE)   -- 1 arg
       59                   pop cx              (cdecl cleanup, 1 word --
                                                  TC 2.0 uses "pop reg" rather
                                                  than "add sp,2" to discard
                                                  exactly one pushed word)
       E8 5822              call timer_irq_install          (16C7A)   -- 0 args
       E8 3819              call dos_critical_error_install          (1635D)
       E8 1C8A              call ui_gfx_alloc          (1D444)
       E8 93FF              call blitter_patch_variant          (149BE)
       E8 53B9              call video_alloc_framebuffer          (10381)
       E8 248C              call player_record_load_publish          (1D655)
       E8 2A20              call keyboard_irq_install          (16A5E)
       FB                   sti                 (enable())
       E8 A3D8              call resource_stripe_table_load          (122DB)   -- 0 args
       E8 6ED8              call sprite_load_boot_sheets          (122A9)
       33C0 50              xor ax,ax; push ax
       E8 6523              call sprite_sheet_select          (16DA6)   -- 1 arg, 0
       59                   pop cx              (cdecl cleanup, 1 word)
       C3                   ret
   The two 0-arg calls immediately after srand's cleanup (timer_irq_install, dos_critical_error_install, ...)
   have no push before them and no pop/add after: void, no return value
   used. */
extern int  biostime();
extern void srand();
extern void timer_irq_install();
extern void dos_critical_error_install();
extern void ui_gfx_alloc();
extern void blitter_patch_variant();
extern void video_alloc_framebuffer();
extern void player_record_load_publish();
extern void keyboard_irq_install();
extern void resource_stripe_table_load();
extern void sprite_load_boot_sheets();
extern void sprite_sheet_select();

void boot_init_seed_rand()
{
    srand(biostime(0, 0L));
    timer_irq_install();
    dos_critical_error_install();
    ui_gfx_alloc();
    blitter_patch_variant();
    video_alloc_framebuffer();
    player_record_load_publish();
    keyboard_irq_install();
    asm sti;
    resource_stripe_table_load();
    sprite_load_boot_sheets();
    sprite_sheet_select(0);
}
