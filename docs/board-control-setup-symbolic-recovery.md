# F_B7F9 symbolic board-control setup

`F_B7F9` is now a 366-byte symbolic TASM owner. It initializes board/control
state, services six display columns, then fills two fixed workspace ranges.

The source exposes the control-state flag transitions, the six-column loop,
the paired far-pointer setup, and the two terminal fill loops. Its historical
helper invocations were direct same-segment calls, retained as symbolic
relative expressions, so the recovered object has no OMF FIXUPP records.

Fresh TASM emits the exact `_TEXT` bytes and `_fb7f9` public at offset zero.
The full canonical link remains byte-identical with 106 relocations in order
and zero unresolved symbols.
