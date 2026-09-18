/* F_6C57 -- arm a tick deadline.  gb76 is the free-running tick (unsigned
   long), gc0d0 the deadline.  Plain C. */
extern unsigned long gb76, gc0d0;

void f6c57(int n)
{
    gc0d0 = n + gb76;
}
