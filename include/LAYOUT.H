/* LAYOUT.H -- the dialog geometry block (DS:C102..DS:C12E) computed by
   dialog_layout from a struct dialog and consumed by dialog_draw/dialog_draw_shadow/dialog_draw_button/dialog_fill_box and
   others; plus the line-height word (DS:C0E8).

   All C102..C12E words are one type per object (int), matching the writer
   dialog_layout and every owned reader. dialog_button1_label_off/dialog_button2_label_off are near char pointers holding
   button-set label offsets (dialog_layout assigns constants like (char near *)0xda9
   and gc8480/gc8267 pass them on as (char far *)gcXXX).

   ui_gfx_blob (DS:C5CA, segment at DS:C5CC) is NOT declared here: dialog_draw_shadow/dialog_restore_screen
   push it as a bare `unsigned` word alongside a separately declared
   dialog_backdrop_save_size, matching the six/four explicit word pushes their thunk calls
   compile to (see F_8434.C/F_8453.C); giving it a `char far *` type there
   makes the compiler push it as a 2-word far pointer on its own and adds
   an extra push, breaking byte-exactness. Every other owned reader (an
   actual far pointer use) declares `extern char far *ui_gfx_blob;` locally. */

extern int dialog_text_inset_x;                       /* DS:C102 -- kind-dependent text left inset */
extern int dialog_button_str;                       /* DS:C104 -- button-set label string, as int (cast to near/far ptr by users) */
extern int dialog_box_y;                       /* DS:C106 -- box y (dialog->cy, or centered) */
extern int dialog_box_w;                       /* DS:C108 -- box width */
extern int dialog_box_x;                       /* DS:C10A -- box x (dialog->cx, or centered) */
extern int dialog_divider1_y;                       /* DS:C10C -- kind 4/5/6/7 divider y */
extern int dialog_divider2_y;                       /* DS:C10E -- kind 5/6/7 second divider y */
extern int dialog_button_label_y;                       /* DS:C110 -- kind 0/1/2/3/7 button-label y */
extern char near *dialog_button1_label_off;                /* DS:C112 -- kind 4..7 first button-set label offset */
extern char near *dialog_button2_label_off;                /* DS:C114 -- kind 5..7 second button-set label offset */
extern int dialog_button1_label_w;                       /* DS:C116 -- first button-set label field width */
extern int dialog_button2_label_w;                       /* DS:C118 -- second button-set label field width */
extern int dialog_box_h;                       /* DS:C11A -- box height */
extern int dialog_button1_label_x;                       /* DS:C11C -- first button-set label x */
extern int dialog_button2_label_x;                       /* DS:C11E -- second button-set label x */
extern int dialog_button_label_x;                       /* DS:C120 -- button-label x */
extern int dialog_title_x;                       /* DS:C122 -- title x */
extern int dialog_button1_label_rows;                       /* DS:C124 -- first button-set label height/row count */
extern int dialog_button2_label_rows;                       /* DS:C126 -- second button-set label height/row count */
extern int dialog_text_inset_y;                       /* DS:C128 -- kind-dependent text top inset */
extern int dialog_text_inset_x2;                       /* DS:C12A -- kind-dependent text left inset (x2) */
extern int dialog_button1_label_cx;                       /* DS:C12C -- first button-set label centered x */
extern int dialog_button2_label_cx;                       /* DS:C12E -- second button-set label centered x */

extern int dialog_line_height;                       /* DS:C0E8 -- line height for wrapped text (text_draw_wrapped) */
