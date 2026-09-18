/* F_7313 -- advance the 0..4 clamped counter gb80 by n and report it.
   The dead `jmp` at 732A is the skip-over-the-else jump TC 2.0 emits at the
   end of EVERY then-branch that has an else, even when the then-branch ends
   in a `return` and the jump is therefore unreachable.  The `< 0` test uses
   the flags left by `add [gb80],ax`, which is why the += is written inside
   the condition. */
extern int gb80;
extern void f7298();

int f7313(n)
int n;
{
    if ((gb80 += n) < 0) {
        gb80 = 0;
        return -1;
    } else {
        if (gb80 > 4)
            gb80 = 4;
    }
    f7298();
    return gb80;
}
