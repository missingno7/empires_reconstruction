extern void f01ce(), f03a8(), f039f(), f86c9();
extern char text118c[];
struct R125D { int a; int b; char c; int d; char far *e; unsigned char tail[9]; };
struct R125D g125d = { 1, 0, 0, 0x200, text118c, { 0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff } };
void f9d8e(void) { f01ce(0); f03a8(8,16,304,145); f039f(8,16,304,145); f86c9((char *)&g125d); }
