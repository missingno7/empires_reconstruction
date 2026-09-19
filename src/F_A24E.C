#include "C470.H"
extern struct c470_record gc470[];
extern char gc563;
void fa24e(int at) { register int i; for(i=at+1;i<10;i++) gc470[i-1]=gc470[i]; gc563=0; }
