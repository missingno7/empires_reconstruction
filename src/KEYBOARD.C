/* src/KEYBOARD.C: Keyboard: INT 9 handler chain and BIOS keyboard helpers.
   One translation unit; the sections below were the separate member
   sources of grouped module C_6990_6B74 and keep their original ids. */

extern int near keyboard_state[];                 /* DS:0B72 */

/* ---- F_6990 (original code at 0x6990) ---- */
/* F_6990 -- set the flag at DS:0B72.  No frame (rule 11). */
void keyboard_chain_enable()
{
    keyboard_state[0] = 1;
}


/* ---- F_6997 (original code at 0x6997) ---- */
void keyboard_chain_disable(void)
{
    keyboard_state[0] = 0;
}


/* ---- F_699E (original code at 0x699E) ---- */
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
void interrupt keyboard_irq_handler(void)
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


/* ---- F_6B1A (original code at 0x6B1A) ---- */
/* F_6B1A -- blocking INT 16h read with the F1..F10 hot-key check. */
extern int menu_list_active();
extern void menu_loop_run();

int f6b1a()
{
    asm xor ah,ah
    asm int 16h
    asm or  al,al
    asm jnz L_ascii
    asm mov al,ah
    asm mov ah,1
    asm cmp al,3bh
    asm jb  L_out
    asm cmp al,44h
    asm jg  L_out
    asm mov si,ax
    asm call near ptr menu_list_active
    asm or  ax,ax
    asm mov ax,si
    asm jz  L_out
    asm sub ax,013bh
    asm push ax
    asm call near ptr menu_loop_run
    asm pop ax
    asm mov ax,si
    asm jmp short L_out
L_ascii: asm xor ah,ah
L_out: ;
}

/* ---- F_6B4A (original code at 0x6B4A) ---- */
/* F_6B4A -- non-blocking INT 16h keyboard poll. */
int f6b4a()
{
    asm mov ah,1
    asm int 16h
    asm jz  L_none
    asm or  al,al
    asm jnz L_ascii
    asm mov al,ah
    asm mov ah,1
    asm jmp short L_out
L_ext: asm mov ax,100h
    asm jmp short L_ascii
L_none: asm xor ax,ax
    asm jmp short L_out
L_ascii: asm xor ah,ah
L_out: ;
}


/* ---- F_6B66 (original code at 0x6B66) ---- */
/* F_6B66 -- drain the BIOS keyboard buffer: while F_6B4A still reports a
   key, take one.  The BIOS call is C through the `_AX` pseudo-register and
   the `__int__` intrinsic (same bytes as the former asm `xor ax,ax` /
   `int 16h`).  There is no frame because there is no parameter and no local
   (rule 11).  The leading EB04 is the while's jump to its test. */
extern int f6b4a();
void __int__(int);

void keyboard_buffer_drain()
{
    while (f6b4a()) {
        _AX = 0;
        __int__(0x16);
    }
}


/* ---- F_6B74 (original code at 0x6B74) ---- */
/* F_6B74 -- read back the flag at DS:0B72.  The trailing EB00 is the
   `return`'s jump to the epilogue at zero displacement (rule 7's shape). */
extern int near keyboard_state[];                 /* DS:0B72 */

int keyboard_chain_active()
{
    return (keyboard_state[0]);
}
