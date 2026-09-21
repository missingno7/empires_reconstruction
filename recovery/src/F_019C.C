/* F_019C -- write the caller's buffer through DOS handle 2. */
void f019c()
{
    asm mov ah,40h
    asm mov bx,2
    asm int 21h
}
