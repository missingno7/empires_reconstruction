int w, v, u, i, n, k;
unsigned char c;
unsigned char far *r;
unsigned char far *q;

/* 1: direct byte negate through far pointer index */
void t1(void)
{
    r[3] = -r[3];
}

/* 2: through int-typed local, read then write back */
void t2(void)
{
    w = r[3];
    r[3] = -w;
}

/* 3: negate into int local, then store */
void t3(void)
{
    u = -r[3];
    r[3] = u;
}

/* 4: cast to int explicitly at the store */
void t4(void)
{
    r[3] = (int) -r[3];
}

/* 5: keep expression int-typed with +0 */
void t5(void)
{
    r[3] = -r[3] + 0;
}

/* 6: or with 0 */
void t6(void)
{
    r[3] = -(r[3] | 0);
}

/* 7: and with 0xff */
void t7(void)
{
    r[3] = -(r[3] & 0xff);
}

/* 8: negate a register int local */
void t8(void)
{
    register int reg;
    reg = r[3];
    r[3] = -reg;
}

/* 9: ~x + 1 */
void t9(void)
{
    r[3] = ~r[3] + 1;
}

/* 10: (0 - r[3]) & 0xff */
void t10(void)
{
    r[3] = (0 - r[3]) & 0xff;
}

/* 11: unary minus on unsigned char cast to unsigned int explicit, stored via int local w declared earlier in fn */
void t11(void)
{
    int t;
    t = r[3];
    r[3] = -t;
}

/* 12: comma expr */
void t12(void)
{
    r[3] = -r[3], 0;
}

/* 13: assignment expression nested */
void t13(void)
{
    r[3] = (r[3] = -r[3]);
}

/* 14: negate through q pointer expression matching original context (c & 0xf)*4+0x2cb+3 */
void t14(void)
{
    q[(c & 0xf) * 4 + 0x2cb + 3] = -q[(c & 0xf) * 4 + 0x2cb + 3];
}

/* 15: -= style */
void t15(void)
{
    r[3] -= r[3] * 2;
}

/* 16: store through w, but w is negative of an int cast expression built via subtraction from 0 stored in int */
void t16(void)
{
    w = 0 - r[3];
    r[3] = w;
}

/* 17: ternary always-false to keep int typing */
void t17(void)
{
    r[3] = r[3] ? -r[3] : 0;
}

/* 18: double negation trick */
void t18(void)
{
    r[3] = -(-(-r[3]));
}

/* 19: negate assigned into c (unsigned char global), then store c into r[3] -- but c is byte; try letting v (int) hold */
void t19(void)
{
    v = -(int)r[3];
    r[3] = v;
}

/* 20: with n int local doing subtraction chain typical of TC codegen for signed-magnitude */
void t20(void)
{
    n = -r[3];
    r[3] = n;
}

/* 21: register int fully round-tripped: negate INTO register, then store FROM register */
void t21(void)
{
    register int reg;
    reg = -r[3];
    r[3] = reg;
}

/* 22: same but via comma-assignment chain */
void t22(void)
{
    register int reg;
    r[3] = (reg = -r[3]);
}

/* 23: cast pointee to int before negating, single statement, no temp */
void t23(void)
{
    r[3] = -(int)(*(r + 3));
}

/* 24: negate an expression widened by adding 0 */
void t24(void)
{
    r[3] = -(0 + r[3]);
}

/* 25: multiply by an int variable holding -1 */
void t25(void)
{
    int minus1;
    minus1 = -1;
    r[3] = r[3] * minus1;
}

/* 26: negate into plain int local (not register), then store, two extra statements */
void t26(void)
{
    int t;
    t = r[3];
    t = -t;
    r[3] = t;
}

/* 27: unsigned cast around int cast */
void t27(void)
{
    r[3] = (unsigned)(-(int)r[3]);
}

/* 28: modulo trick to force int math without separate var */
void t28(void)
{
    r[3] = -r[3] % 256;
}
