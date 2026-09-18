/* F_7E07 -- lay out a dialog box: measure the text, pick the geometry for the
   box kind from three switches, and leave the result in the C1xx block. */
struct D {
    int kind;                           /* +00 */
    char far *msg;                      /* +02 */
    char sub;                           /* +06 */
    char far *text;                     /* +07 */
    char pad;                           /* +0B */
    int cx;                             /* +0C */
    int cy;                             /* +0E */
    int w;                              /* +10 */
    int lines;                          /* +12 */
};

extern int f6cf6();
extern int gc102, gc104, gc106, gc108, gc10a, gc10c, gc10e, gc110;
extern int gc116, gc118, gc11a, gc11c, gc11e, gc120, gc122, gc124, gc126;
extern int gc128, gc12a, gc12c, gc12e;
extern char near *gc112;
extern char near *gc114;

void f7e07(p)
struct D far *p;
{
    int w1;                             /* bp-8 */
    int nlines;                         /* bp-6 */
    char far *q;                        /* bp-4 */
    register int s, d;                  /* si, di */

    d = 0;
    s = 0;
    if ((nlines = p->lines) == -1) {
        nlines = 1;
        q = p->text;
        while (*q != 0) {
            if (*q == 0xa || *q == 0xd) nlines++;
            q++;
        }
    }
    gc11a = nlines * 10 + 10;
    if (p->kind == 2) gc11a -= 2;
    if (p->msg == 0) {
        gc108 = w1 = 0;
    } else {
        gc11a += 14;
        gc108 = w1 = f6cf6(p->msg);
    }
    switch (p->sub) {
    case 0:
        gc104 = 0;
        break;
    case 1:
        gc11a += 13;
        gc104 = 0xd7e;
        s = 0x78;
        break;
    case 2:
        gc11a += 13;
        gc104 = 0xd8b;
        s = 0x94;
        break;
    }
    switch (p->kind) {
    case 0:
    case 1:
        if (p->msg == 0) {
            gc12a = 4;
            gc128 = 6;
            gc102 = 7;
        } else {
            gc12a = 8;
            gc128 = 4;
            gc102 = 9;
        }
        break;
    case 2:
    case 3:
        gc12a = 0;
        gc128 = 2;
        gc102 = 4;
        break;
    case 4:
        gc12a = 6;
        gc128 = 4;
        gc102 = 0xb;
        gc112 = (char near *)0xda9;
        gc116 = 0x78;
        gc124 = 0xd;
        s = 0x8c;
        gc11a += gc124 + 3;
        break;
    case 5:
        gc12a = 6;
        gc128 = 4;
        gc102 = 0xb;
        gc112 = (char near *)0xd9a;
        gc114 = (char near *)0xda9;
        gc116 = 0x78;
        gc118 = 0x78;
        gc124 = 0xd;
        gc126 = 0xd;
        s = 0x10e;
        gc11a += gc124 + 3;
        break;
    case 6:
        gc12a = 6;
        gc128 = 4;
        gc102 = 0xb;
        gc112 = (char near *)0xd9a;
        gc114 = (char near *)0xdb7;
        gc116 = 0x78;
        gc118 = 0x78;
        gc124 = 0xd;
        gc126 = 0xd;
        s = 0x10e;
        gc11a += gc124 + 3;
        break;
    case 7:
        gc12a = 8;
        gc128 = 4;
        gc102 = 4;
        gc112 = (char near *)0xdc0;
        gc114 = (char near *)0xdbc;
        gc116 = 0x28;
        gc118 = 0x28;
        gc124 = 0xb;
        gc126 = 0xb;
        s = 0x6e;
        gc11a += gc124 + 8;
        break;
    }
    if (gc108 < s) gc108 = s;
    gc11a += gc128 + gc102;
    if ((d = p->w) == -1) {
        d = f6cf6(p->text);
        d += gc12a * 2;
        d += 2;
    }
    if (gc108 < d) gc108 = d;
    gc108 += 10;
    if ((gc10a = p->cx) == -1) gc10a = (0x140 - gc108) / 2;
    if ((gc106 = p->cy) == -1) gc106 = (0xc8 - gc11a) / 2;
    gc122 = (gc108 - 2 - w1) / 2 + gc10a;
    gc120 = (gc108 - 2 - s) / 2 + gc10a;
    switch (p->kind) {
    case 0:
    case 1:
    case 2:
    case 3:
        gc110 = gc106 + gc11a - 0x11;
        break;
    case 4:
        gc11c = gc10a + gc108 - 0x88;
        gc10c = gc106 + gc11a - (gc124 + 7);
        break;
    case 5:
        gc11c = gc10a + 14;
        gc11e = gc10a + gc108 - 0x88;
        gc10c = gc10e = gc106 + gc11a - (gc124 + 7);
        break;
    case 6:
        gc11c = gc10a + 14;
        gc11e = gc10a + gc108 - 0x88;
        gc10c = gc10e = gc106 + gc11a - (gc124 + 7);
        break;
    case 7:
        gc110 = gc106 + gc11a - 0x11;
        gc11c = gc10a + gc108 - 0x38;
        gc11e = gc11c - 0x32;
        gc10c = gc10e = gc106 + gc11a - (gc124 + 0x17);
        break;
    }
    switch (p->kind) {
    case 5:
    case 6:
    case 7:
        gc12e = (gc118 - f6cf6((char far *)gc114)) / 2 + gc11e;
    case 4:
        gc12c = (gc116 - f6cf6((char far *)gc112)) / 2 + gc11c;
        break;
    }
}
