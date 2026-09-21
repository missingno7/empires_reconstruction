/* Exact command-line parser. Turbo C -B passes generated assembly to TASM;
 * this naturally reproduces the original switch and branch encodings. */
extern char far * far *_argv;
extern int _argc,g1778;
extern char display_mode;
void cmdline_parse_args(void)
{
 register int i;
 for(i=1;i<_argc;i++) {
  if(_argv[i][0]=='-' || _argv[i][0]=='/') {
   switch(_argv[i][1]) {
    case 'E':case 'e':display_mode=1;break;
    case 'C':case 'c':display_mode=2;break;
    case 'T':case 't':display_mode=3;break;
    case 'M':case 'm':display_mode=4;break;
    case 'V':case 'v':display_mode=5;break;
    case 'I':case 'i':g1778=0;break;
    case 'S':case 's':
     switch(_argv[i][2]) {
      case 'I':case 'i':g1778=0;break;
      case 'A':case 'a':g1778=2;break;
      case 'T':case 't':g1778=1;break;
     }
     break;
   }
  }
 }
}
