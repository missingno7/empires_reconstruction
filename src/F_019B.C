/* 1 byte: a bare RET.  Probe for -k / -k-: with standard stack frames on,
   TC 2.01 emits push bp/mov bp,sp/pop bp/ret (5 bytes) for even an empty body. */
void f019b(void) { }
