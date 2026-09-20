# F_AB66 symbolic record-workspace recovery

`F_AB66` is now a 385-byte symbolic TASM owner. It builds a 27-byte indexed
record workspace, presents the selection path, and retains the selected
record's two descriptor fields for subsequent display code.

The routine has no OMF FIXUPP records: its historical helper invocations are
direct same-segment calls. The recovered source keeps that relationship as
symbolic relative expressions while exposing the retry path, record loop, and
selection-state transitions.

Fresh TASM produces the exact `_TEXT` bytes and `_fab66` public at offset zero.
The canonical Turbo Link build is verified separately to remain byte-identical,
with all 106 relocations in historical order and zero unresolved symbols.
