struct R { char pad[11]; char b; char rest[15]; };
extern struct R gc470[];
extern int g13ed;
int face7(void) { return (gc470[g13ed].b & 0x20) == 0x20; }
