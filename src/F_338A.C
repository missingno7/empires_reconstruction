/* F_338A -- dispatch one scripted event stream and animate its unit. */
extern int f03b4(), f03cc(), fcaf1(), f250c(), f32fa(), f6181();
extern int g96;
extern unsigned char g43b4[][0x3e8];
extern unsigned char gb3af[];
extern char s9a5c[], s99da[], s9b6e[], s9ae0[];

void f338a(unsigned char far *s)
{
    int w, v, u, i, n, k;
    unsigned char c;
    unsigned char far *r;
    unsigned char far *q;
    register int x, y;

    n = *s;
    if (s[1] < 2) {
        x = s[2];
        x <<= 1;
        y = s[3];
        if (s[1] != 0) {
            x = s[2];
            x <<= 1;
            y = s[3];
            f03b4(x, y + 0x148, 0x18, 8, x, y + 0xb8);
            g96 = 0x167;
            if (s[4] ^= 1)
                f03cc(x, y + 0xb8, s9a5c, 0);
            else
                f03cc(x, y + 0xb8, s99da, 0);
            g96 = 0x9f;
            f03b4(x, y + 0xb8, 0x18, 8, x, y);
        } else {
            x = s[2];
            x <<= 1;
            y = s[3];
            f03b4(x, y + 0x148, 0x18, 9, x, y + 0xb8);
            g96 = 0x167;
            if (s[4] ^= 1)
                f03cc(x, y + 0xb8, s9b6e, 0);
            else
                f03cc(x, y + 0xb8, s9ae0, 0);
            g96 = 0x9f;
            f03b4(x, y + 0xb8, 0x18, 9, x, y);
        }
        fcaf1(8);
    } else if (s[1] == 2)
        fcaf1(0x16);
    s += i = 5;
    for (; i < n; i++, s++) {
        c = *s;
        if ((c & 0x80) == 0) {
            if (c & 0x10)
                f32fa(c & 0xf);
            else if (c & 0x40)
                f6181(c & 0xf);
            else
                f250c(c);
        } else if ((c & 0x30) == 0) {
            i++;
            s++;
            r = g43b4[*s] + (c & 0x7f) * 3 + 0x2ac;
            c = *r;
            *r &= 0xf0;
            *r ^= 0x20;
            if (c & 0x80) {
                if (c & 0x20)
                    r[2] -= 0x30;
                else
                    r[2] += 0x30;
            } else {
                if (c & 0x20)
                    r[1] -= 0x18;
                else
                    r[1] += 0x18;
            }
        } else if (c & 0x10) {
            i++;
            s++;
            r = (q = g43b4[*s]) + (c & 0xf) * 4 + 0x2cb;
            asm les bx,r
            asm mov al,es:[bx+3]
            asm neg ax
            asm les bx,r
            asm mov es:[bx+3],al
            c = r[2] + 2;
            w = r[0];
            v = r[1];
            u = (w + 2) / 4 + ((v + 4) / 8 - 2) * 0x26 - 1;
            for (k = 0; c > k; k++, u++)
                q[u] ^= 0x10;
        } else if (c & 0x20) {
            i++;
            s++;
            c = *s;
            gb3af[c << 5] = 0;
        }
    }
}
