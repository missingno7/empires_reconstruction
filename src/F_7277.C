/* F_7277 -- frameless accessor.  The EB 00 is the return's jump to the
   function exit, which is the next instruction. */
extern int gb7e;

int f7277(void)
{
    return gb7e;
}
