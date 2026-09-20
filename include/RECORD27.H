#ifndef EMPIRES_RECORD27_H
#define EMPIRES_RECORD27_H

/* Recovered 27-byte transfer/list record used at DS:C360 and DS:C470. */
struct record27 {
    char a[9];
    int w9;
    char b11;
    char b12;
    char pad13[4];
    int w17;
    char pad19[2];
    char b21;
    char b22;
    char b23;
    char b24;
    char b25;
    char b26;
};

#endif
