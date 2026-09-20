# F_28AC symbolic board-record draw

`F_28AC` is now a 218-byte symbolic TASM owner. It walks ten three-byte
records through a far board-region pointer, derives a starting cell and stride
from each record, calls the appropriate fixed renderer, then marks six cells in
the target region.

The recovered source preserves the original frame-local traversal state, the
two direct same-segment renderer calls, and the historical near/short branch
forms. Fresh TASM emits the exact `_TEXT` bytes and `_f_28ac` public at offset
zero with no OMF FIXUPP records.

The full `python tools/build_exe.py verify` path remains byte-identical with
106 relocations in historical order and zero unresolved symbols.
