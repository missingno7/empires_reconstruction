/* puzzle_grid[4][6]: the 4x6 slot grid, one 2-byte tile per cell (DS:C316).
   F_8A37 names the fields a/b, F_8AA2 (via ROW.e[6]) a/b (unsigned),
   F_8BAB (struct Piece) kind/rotation, F_90A6 (struct piece_desc) a/b.  puzzle_held_piece, the
   single "held piece" scalar used by F_8BAB, shares this same 2-byte shape
   (its second byte is the adjacent gc133 storage); F_8A37 and F_90A6 only
   ever touch puzzle_held_piece as the lone leading byte, so they keep their own plain
   `char puzzle_held_piece` declaration and are not part of this header. */
struct gc316_tile {
    char kind;    /* offset 0: F_8A37 .a / F_8AA2 .a / F_8BAB .kind (signed; F_8AA2 casts to unsigned for its 0xff sentinel compare) / F_90A6 .a */
    char rot;            /* offset 1: F_8A37 .b / F_8AA2 .b / F_8BAB .rotation / F_90A6 .b */
};

extern struct gc316_tile puzzle_grid[4][6];   /* DS:C316 */
