#ifndef EMPIRES_DIALOG_H
#define EMPIRES_DIALOG_H

/* Dialog descriptor shared by dialog_layout (geometry), dialog_draw (drawing) and dialog_run
   (input loop). 20 bytes, byte packed. Field offsets and the -1 "automatic"
   sentinels are established by F_7E07; the ten static descriptors in DATA all
   carry cx=cy=w=lines=-1. Names describe observed use, not recovered text. */
struct dialog {
    int kind;                   /* +00  box kind 0..7; selects geometry and buttons */
    char far *title;            /* +02  optional heading string; 0 = none */
    char sub;                   /* +06  button-set selector 0..2 (dialog_layout) */
    char far *text;             /* +07  body text; CR or LF separates lines */
    unsigned char initial;      /* +0B  initial selection/direction for kinds 4..7 */
    int cx;                     /* +0C  requested x; -1 = centre horizontally */
    int cy;                     /* +0E  requested y; -1 = centre vertically */
    int w;                      /* +10  requested width; -1 = measure the text */
    int lines;                  /* +12  requested line count; -1 = count the text */
};

void dialog_layout(struct dialog far *p);               /* 7E07: compute the C1xx layout block */
void dialog_draw(struct dialog far *p, int first);    /* 8480: lay out and draw; first != 0 also calls dialog_draw_shadow */
int  dialog_run(struct dialog far *p);               /* 86C9: draw, run the key loop, return the selection (-1 = Escape) */

#endif
