# F_B122 symbolic board descriptor expansion

`F_B122` is now a 693-byte symbolic TASM owner. It initialises the two compact
source tables, registers eight counted descriptor tables, expands their far
pointers into the `B07C/B07E` record stream, appends the terminal table, and
sets the completion state.

The repeated table loop is represented as a TASM macro so the source exposes
the actual common record transformation instead of preserving eight copies of
raw instruction bytes. Its historical helper calls remain symbolic relative
expressions; fresh TASM emits exact `_TEXT` and no OMF FIXUPP records.
