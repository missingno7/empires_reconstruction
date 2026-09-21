/* Keyboard IRQ: Turbo C interrupt ABI and native hardware intrinsics.
   keyboard_state[0] gates chaining; keyboard_state[1] is the adjacent modifier state. */
void __sti__(void);
unsigned char __inportb__(int);
void __outportb__(int,unsigned char);
extern int near gb68,gb6a,gb6c,gb6e,gb70,keyboard_state[];
extern char near b856;
#define g856 b856
#define gb74 keyboard_state[1]
extern void interrupt (*int9_saved_vector)(void);   /* saved INT 9 vector */
extern void sound_effects_toggle(void);
void interrupt f699e(void)
{
 int ack;
 unsigned char scan;
 register int down,handled;
 __sti__();
 scan=__inportb__(0x60);handled=1;
 if(scan==0xe0 || scan==0xe1) {
  ack=__inportb__(0x61);
  __outportb__(0x61,ack|0x80);__outportb__(0x61,ack);__outportb__(0x20,0x20);
  return;
 }
 down=scan<0x80?1:0;
 switch(scan&0x7f) {
 case 0x58: if(!g856) break;
 case 0x47: gb68=gb6c=down;if(scan&0x80)gb70=1;break;
 case 0x49: gb68=gb6e=down;if(scan&0x80)gb70=1;break;
 case 0x29: if(!g856) break;
 case 0x48: gb68=down;if(scan&0x80)gb70=1;break;
 case 0x2b: if(!g856) break;
 case 0x4b: gb6c=down;break;
 case 0x4e: if(!g856) break;
 case 0x4d: gb6e=down;break;
 case 0x4a: if(!g856) break;
 case 0x50: gb6a=down;break;
 case 0x46: case 0x54: break;
 case 0x1f: if(gb74 && down) sound_effects_toggle();else handled=0;break;
 case 0x1d: gb74=down;
 default: handled=0;break;
 }
 if(!handled || keyboard_state[0]) int9_saved_vector();
 else {ack=__inportb__(0x61);__outportb__(0x61,ack|0x80);__outportb__(0x61,ack);__outportb__(0x20,0x20);}
}
