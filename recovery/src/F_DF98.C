/* F_DF98 -- complete far-call and state update routine. */
void music_voice_frequency_lookup()
{
    asm db 055h,08bh,0ech,083h,0ech,006h,056h,057h,08bh,046h,006h,005h,000h,0e0h,099h,052h,050h,0a1h,06bh,0cah,099h,05bh,059h
#ifndef EMPIRES_SHARED_ARITHMETIC_MODULE
    asm extrn LXMUL@:far
#endif
    asm call far ptr LXMUL@
    asm db 089h,056h,0feh,089h,046h,0fch,08bh,016h,054h,037h,0a1h,052h,037h,03bh,056h,0feh,075h,02ah,03bh,046h,0fch,075h,025h,08bh
    asm db 016h,0e8h,0c5h,0a1h,0e6h,0c5h,08bh,05eh,004h,0d1h,0e3h,0d1h,0e3h,089h,097h,026h,0cah,089h,087h,024h,0cah,0a1h,0e4h,0c5h
    asm db 08bh,05eh,004h,0d1h,0e3h,089h,087h,06dh,0cah,0e9h,09fh,000h,033h,0d2h,0b8h,000h,020h,052h,050h,0ffh,076h,0feh,0ffh,076h
    asm db 0fch
#ifndef EMPIRES_SHARED_ARITHMETIC_MODULE
    asm extrn LDIV@:far
#endif
    asm call far ptr LDIV@
    asm db 08bh,0f8h,00bh,0ffh,07dh,03ah,0b8h,018h,000h,02bh,0c7h,089h,046h,0fah,08bh,046h,0fah,0bbh,019h,000h,099h,0f7h,0fbh,0f7h
    asm db 0d8h,08bh,05eh,004h,0d1h,0e3h,089h,087h,06dh,0cah,0a3h,0e4h,0c5h,08bh,046h,0fah,005h,0e8h,0ffh,0bbh,019h,000h,099h,0f7h
    asm db 0fbh,08bh,0f2h,00bh,0f6h,074h,007h,0b8h,019h,000h,02bh,0c6h,08bh,0f0h,0ebh,01eh,08bh,0c7h,0bbh,019h,000h,099h,0f7h,0fbh
    asm db 08bh,05eh,004h,0d1h,0e3h,089h,087h,06dh,0cah,0a3h,0e4h,0c5h,08bh,0c7h,0bbh,019h,000h,099h,0f7h,0fbh,08bh,0f2h,08bh,0c6h
    asm db 0bah,018h,000h,0f7h,0e2h,005h,0c3h,0c6h,08ch,0dah,08bh,05eh,004h,0d1h,0e3h,0d1h,0e3h,089h,097h,026h,0cah,089h,087h,024h
    asm db 0cah,089h,016h,0e8h,0c5h,0a3h,0e6h,0c5h,08bh,056h,0feh,08bh,046h,0fch,089h,016h,054h,037h,0a3h,052h,037h,05fh,05eh,08bh
    asm db 0e5h,05dh
#ifndef EMPIRES_SHARED_ARITHMETIC_MODULE
    asm db 0c3h
#endif
#ifndef EMPIRES_SHARED_ARITHMETIC_MODULE
    asm _fdf98_end label byte
    asm public _fdf98_end
#endif
}
