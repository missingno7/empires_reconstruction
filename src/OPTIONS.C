/* Option/music/sound toggles and the two record listings. */
#include "C470.H"
#include "DIALOG.H"
extern int sound_enabled,music_enabled;
extern struct dialog near dialog_toggle_option,dialog_toggle_music,dialog_toggle_sound,dialog_slot_backup_list,dialog_slot_list;
extern int dialog_line_height_get(), f6b1a();extern void f03a2();extern void f039f();
extern void music_resume_if_valid(void);
extern void fc834();
extern void dialog_restore_screen();
extern void sound_stop_reset(void);
extern void gfx_color_select(int n);
int options_toggle_option(void)
{
 register int result;
 dialog_toggle_option.initial=!slot_table[current_slot].option;
 result=dialog_run(&dialog_toggle_option);
 if(result>=0) {slot_table[current_slot].option=result;if(result)slot_table[current_slot].pending=-1;}
 return 0;
}

int options_toggle_music(void)
{
 register int result;
 dialog_toggle_music.initial=!slot_table[current_slot].music;
 result=dialog_run(&dialog_toggle_music);
 if(result>=0) {music_enabled=slot_table[current_slot].music=result;if(!result)fc834();else music_resume_if_valid();}
 return 0;
}

int options_toggle_sound(void)
{
 register int result;
 dialog_toggle_sound.initial=!slot_table[current_slot].sound;
 result=dialog_run(&dialog_toggle_sound);
 if(result>=0) {sound_enabled=slot_table[current_slot].sound=result;if(!result)sound_stop_reset();}
 return 0;
}

int slot_backup_list_show(void)
{
 int height;
 register int i,y;
 dialog_draw(&dialog_slot_backup_list,1);gfx_color_select(0);
 f03a2(43,50,39);f03a2(157,50,38);f03a2(223,50,35);
 height=dialog_line_height_get();
 for(i=0,y=0;i<10;i++,y+=height+1) slot_row_draw(&gc360[i],y+52,0);
 f039f(8,0,304,200);
 do {i=f6b1a();} while(i!=27&&i!=13);
 dialog_restore_screen();return 0;
}

int slot_list_show(void)
{
 int height;
 register int i,y;
 dialog_draw(&dialog_slot_list,1);gfx_color_select(0);
 f03a2(43,50,39);f03a2(153,50,38);f03a2(220,50,55);
 height=dialog_line_height_get();
 for(i=0,y=0;i<10;i++,y+=height+1) slot_row_draw(&slot_table[i],y+52,1);
 f039f(8,0,304,200);
 do {i=f6b1a();} while(i!=27&&i!=13);
 dialog_restore_screen();return 0;
}
