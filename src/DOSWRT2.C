/* Exact 51-byte DOS handle-2 writer using Turbo C register pseudo-variables and __int__; no inline ASM or byte emission.
   Preserve the historical strlen(text)-1 length and DS load exactly. */
extern unsigned strlen(const char far *);
void __int__(int);
void dos_write_handle2(char far *text)
{
 unsigned segment;
 register unsigned length,offset;
 length=strlen(text)-1;
 offset=(unsigned)text;
 segment=(unsigned)((unsigned long)text>>16);
 _BX=2;_CX=length;_DX=offset;_AH=0x40;_DS=segment;
 __int__(0x21);
}
