# C470 record model

`include/C470.H` defines the recovered 27-byte record stored at DS:C470. The
evidence is mechanical: F_A223 tests byte 0 for an unused slot, F_A28D reads a
9-byte text field followed by a word and flags byte, and F_9DCC writes a state
byte at offset 15h. The resulting layout is:

| Offset | Field | Evidence |
| --- | --- | --- |
| 00h | `text[9]` | F_A223 and F_A28D |
| 09h | `value` | F_A28D numeric display |
| 0Bh | `flags` | F_A28D option branch |
| 0Ch | `pad[9]` | required to reach the independently addressed state byte |
| 15h | `state` | F_9DCC stores value 4 |
| 16h | `tail[5]` | exact 27-byte stride |

The header is currently used by F_9DCC, F_A09D, F_A13F, F_A223, F_A24E, F_A28D,
and F_A33F. Fresh compilation of each complete owner remains byte-identical.
F_A33F uses a `char (*)[27]` matrix-view macro over the typed storage because
Turbo C emits four extra bytes when its matrix decay is replaced directly with
a typed-record address. The macro preserves its original expression and exact
object while keeping one declaration of the underlying storage.
