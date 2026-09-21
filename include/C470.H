#ifndef EMPIRES_C470_H
#define EMPIRES_C470_H

/* 27-byte save-slot record. DS:C470 holds the ten live slots (slot_table), DS:C360
   the ten transfer copies (gc360); both are BSS tables of exactly 270 bytes with
   the int slot_select_error between them. Offsets and widths are established by the code in
   the files named below; names describe observed use, not recovered text. */
struct c470_record {
    char text[9];               /* +00  name; text[0]==0 marks a free slot (F_A223, F_AD25) */
    int value;                  /* +09  score-like counter: ++ in F_3986, ultoa in F_A28D, =1 in F_ADCF */
    char flags;                 /* +0B  bit 0x10 selects the Explorer/Expert label (F_A28D);
                                        bit 0x20 tested by F_ACE7 and set by F_ADCF; bits 0..4 or-ed by F_CE68 */
    char byte12;                /* +0C  1-based selection written by F_4943, cleared by F_D26C,
                                        read as a count by F_49F0; meaning not consolidated */
    int sound;                  /* +0D  copied to sound_enabled by F_AB66 and M_CB5C_CD23 */
    int music;                  /* +0F  copied to music_enabled by F_AB66 and M_CB5C_CD23 */
    int option;                 /* +11  0/1 toggle in M_CB5C_CD23; nonzero gate in F_D4B3 */
    int pending;                /* +13  bit mask: set to -1 by M_CB5C_CD23, bits cleared by F_D4B3 */
    char state;                 /* +15  4 written by F_3986, F_9DCC and F_ADCF; passed to energy_set by F_49F0 */
    char sub[4];                /* +16  per-item counters indexed by F_4943 (sub[i]*2+i*8, then ++) */
    char byte26;                /* +1A  cleared by F_ADCF; otherwise unused */
};

extern struct c470_record slot_table[10];    /* DS:C470 live slots */
extern struct c470_record gc360[10];    /* DS:C360 transfer copies */
extern int current_slot;                       /* current slot index 0..9 */
extern int slot_used_count;                       /* number of used live slots */
extern int slot_select_error;                       /* selection/error state between the two tables */

/* A28D: draw one record on row y; mode 0 shows the value, mode 1 the flags. The
   far prototype is byte-significant at the call sites. */
int fa28d(struct c470_record far *p, int y, int mode);

#endif
