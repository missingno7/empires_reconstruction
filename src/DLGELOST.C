#include "DIALOG.H"
extern void gfx_color_select(), gfx_clear_rect(), gfx_box();
/* Shared dialog text and its descriptor form one native DATA contribution. */
char text118c[] = "Sorry, better luck next time!\rYou lost the pieces to the last\rartifact.\rYou must go back, find them again,\rand put them together.\rThen try again to break this\rchamber's code.\r\rYour Energy meter will be reset.";
struct dialog g125d = { 1, 0, 2, text118c, 0, -1, -1, -1, -1 };
void dialog_energy_lost_show(void) { gfx_color_select(0); gfx_clear_rect(8,16,304,145); gfx_box(8,16,304,145); dialog_run(&g125d); }
