#ifndef EMPIRES_C470_H
#define EMPIRES_C470_H

/* Recovered DS:C470 record layout: 27 bytes, with a state byte at +15h. */
struct c470_record {
    char text[9];
    int value;
    char flags;
    char pad[9];
    char state;
    char tail[5];
};

#endif
