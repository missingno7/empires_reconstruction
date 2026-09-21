/* src/SLOTS.C: Save-slot table: load, save, find and delete.
   One translation unit; the sections below were the separate member
   sources of grouped module C_A09D_A24E and keep their original ids. */

#include "C470.H"
#include "VIDEO.H"

extern void resource_load_record_into();
extern int resource_load_record();
extern char far *ui_gfx_shadow_a;
extern void resource_file_write_record(unsigned, void far *);
extern char gc563;

/* ---- F_A09D (original code at 0xA09D) ---- */
slot_menu_draw_header()
{
    register int i;

    resource_load_record_into(61, slot_table);
    resource_load_record_into(62, slot_transfer_table);
    resource_load_record(60);
    for (i = 0; i < 3; i++)
        gfx_blit_bitmap(i * 18, 400, ui_gfx_shadow_a + ((int *)ui_gfx_shadow_a)[i] + 2);
    resource_load_record(59);
    gfx_blit_bitmap(0, 440, ui_gfx_shadow_a);
    resource_load_record(58);
    gfx_blit_bitmap(0, 457, ui_gfx_shadow_a);
    resource_load_record(57);
}


/* ---- F_A13F (original code at 0xA13F) ---- */
void slot_table_save() { resource_file_write_record(0x3d,slot_table); resource_file_write_record(0x3e,slot_transfer_table); }


/* ---- F_A15E (original code at 0xA15E) ---- */
/* F_A15E -- draw one menu row's background twice, through the two
   runtime-generated thunks at IP 03ABh and 039Fh.  `mov sp,bp` after each
   call rather than `add sp,8` is rule 13: no register variable is saved, so
   SP can simply be restored. */

void slot_row_highlight(n)
int n;
{
    gfx_fill_rect(0x28, n * 11 + 0x3c, 0xf5, 10);
    gfx_box(0x28, n * 11 + 0x3c, 0xf5, 10);
}


/* ---- F_A19D (original code at 0xA19D) ---- */
void player_type_toggle_draw(void) { gfx_fill_rect(40,126,238,10); gfx_fill_rect(40,136,238,10); gfx_box(40,126,238,20); }


/* ---- F_A1E0 (original code at 0xA1E0) ---- */
void quit_confirm_toggle_draw(void) { gfx_fill_rect(50,103,218,10); gfx_fill_rect(50,113,218,10); gfx_box(50,103,218,20); }


/* ---- F_A223 (original code at 0xA223) ---- */
/* F_A223 -- the index of the first free slot of the 27-byte-stride table at
   DS:C470, or 10 if there is none.  The `mul` by 27 then `mov bx,ax / add
   bx,0C470h / push ds / pop es` is rule 2's struct-member-through-an-array
   shape (offset FIRST).  Both returns end in a jump to the epilogue and the
   second is the zero-displacement EB00 (rule 24). */

int slot_find_free()
{
    register int i;

    for (i = 0; i < 10; i++)
        if (slot_table[i].text[0] == 0)
            return (i);
    return (10);
}


/* ---- F_A24E (original code at 0xA24E) ---- */
void slot_delete(int at) { register int i; for(i=at+1;i<10;i++) slot_table[i-1]=slot_table[i]; gc563=0; }
