extern unsigned char _ctype[];ff9be(c) int c;{if(c==-1)return -1;if(_ctype[(unsigned char)c+1]&8)return (unsigned char)c-32;else return (unsigned char)c;}
