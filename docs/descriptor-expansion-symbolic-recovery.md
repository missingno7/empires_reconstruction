# F_B40F symbolic descriptor expansion

`F_B40F` is now a 236-byte symbolic TASM owner. It expands two compact
descriptor tables into the fixed `B07C/B07E` record stream consumed by the
display and state code.

The recovered source shows three phases: initialize the two compact tables,
copy their two- and twelve-entry descriptor ranges into four-byte output
records, then extend the output to 84 entries with the final default pair.
The original direct same-segment calls remain symbolic relative expressions,
matching the source object's no-FIXUPP topology.

Fresh TASM emits the complete original `_TEXT` extent and `_fbb40f` public at
offset zero with no OMF fixups. `python tools/build_exe.py verify` retains the
byte-identical executable, all 106 MZ relocations in order, and zero unresolved
symbols.
