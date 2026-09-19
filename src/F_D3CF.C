/* F_D3CF -- branch continuation for the far-record decoder. */
void fd3cf()
{
    asm db 0c6h,05h,01h,08bh,0f7h,083h,0c6h,03h,0ach,0ebh,0cfh
    asm _fd3cf_end label byte
    asm public _fd3cf_end
}
