# Follow-up runtime supervisor findings

All three MEDIUM cards (18A5–18C6, 1500–151F, 1488–14A2) were transcribed and
individually passed FAST and fresh full acceptance. Their 90 bytes are now
symbolic instructions. The two mutable near jumps at 07C2 and 0F3C use the
approved exact-E9 helper; JMP AX at 089D is symbolic. These eight bytes were
verified without claiming that their caller argument domains are now proven.

## A duplicated table with a suspicious original reference

The nine words in 191C–192E equal the proven table at 1570–1582 plus 0424h,
entry for entry. Pixel code at 1494–14D7 is byte-identical to 18B8–18FB at
that same translation. Every translated word points at an already reachable
instruction boundary in the later routine. RET at 191B and the next public
routine at 192E bound the table. It is now typed as an unreferenced dispatch
copy; no consumer or CFG root was invented for it.

The interesting part is that the actual dispatcher at 18B3 still reads from
CS:190C, the *earlier* table, rather than CS:1CB8, the translated copy. This
routes short-width paths into the earlier compositor. The two routines have
different local-frame sizes (four versus two bytes). This is evidence worth
investigating as a possible stale reference, but is not a demonstrated gameplay
bug: the short-width cases may never occur for the actual resources passed by
F_9A0E. Its callers obtain dimensions from resource data, not a fixed constant.
The reconstruction preserves the exact historical operand. No repair was made.

## Trailer and remaining uncertainty

19A3–19AB is explicitly the NUL-terminated ASCII identifier EGA.DRV. The adjacent
19A2 byte is C3, which decodes as RET in isolation, but has no proven incoming
edge or entry. It remains a one-byte supervisor task rather than an invented
function or padding classification.

The generated blockers report now separates raw-byte tasks from unresolved
control-flow obligations. Six sites retain incomplete domains: 019C, 02A2,
0305, 089D, 07C2 and 0F3C. Their source can be exact and symbolic while the
normal-mode or width assumptions remain explicit research obligations. In
particular, source recovery of the patched jumps does not establish safety
for arbitrary widths. Closed public roots remain 12/20.
