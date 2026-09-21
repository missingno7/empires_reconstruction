/* F_CA83 -- TC frame with a hand-written body.  The frame is not from -k: it is
   forced by the function HAVING A PARAMETER (which the asm body ignores,
   reading AL directly).  Under the pinned -k- default TC emits
   push bp / mov bp,sp ... pop bp / ret around the asm. */
extern int g1e84;               /* DS:1E84; convention _g1e84 -> DGROUP+0x1E84 = phys 0x219B4, storage_objects/G_P219B4 */
void stream_base_value_set(note)
unsigned char note;
{
    asm xor ah,ah
    asm shl ax,1
    asm shl ax,1
    asm mov word ptr g1e84,ax
}
