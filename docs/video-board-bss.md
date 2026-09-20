# Video, board, and UI BSS islands

The `VIDEO_BOARD_STATE_BSS` contribution is a mixed 860-byte control block.
This checkpoint promotes only the regions with direct source-level evidence;
the untyped intervening controls remain explicit reconstruction work.

| Offset | Typed storage | Evidence |
|---:|---|---|
| `000` | current board and four far pointers | `F_329F`, `F_2AE2`, `F_60A9`, and the board/render ASM routines identify the board id, VRAM, object-list, object cursor, and sprite-bank roles. |
| `012` | video-adapter and display-mode bytes | `F_50C1`, `F_4F96`, and display selection callers. |
| `034` | 22 resource-view far pointers | `F_55C7` constructs exactly 22 views: nine archive-derived views, nine sprite-bank rows, and four final sprite-bank rows. The first is split at the canonical `_t4` anchor. |
| `08C` | dirty rectangle and 24-entry motion X/Y arrays | `F_5AC3`, `F_5E98`, and `F_5A3B` establish the rectangle fields and both 24-element coordinate arrays. |
| `17C` | two 240-byte UI text rows | `F_90A6` indexes `_gc136` as `char[2][240]`. |

The source reserve names describe demonstrated behavior and do not assert
original identifiers or source-module ownership. The generated BSS object
continues to preserve every canonical public anchor and linker position.
