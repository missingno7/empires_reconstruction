# Twenty-seventh local C wave: answer-puzzle dispatcher

F_A768 adds 246 matching C bytes. Its complete extent reproduces the answer
dialog setup, four-key switch table, selection state, cleanup calls and return
path. The exact `DS:13D9` object binding is established by the historical
profile, and all code calls resolve through existing component bindings.

The switch case order is deliberate: Turbo C emits the original jump-table
body order only when the `27` case, the paired `328/336` cases and the `13`
case appear in that source order. Reordering the cases changes the jump-table
targets and fails the full extent comparison. The wave test compiles a fresh
mutant with reordered cases and rejects it.

Coverage is now 218 matching C routines / 26,235 bytes. The full EXE and both
DAT archives remain byte-identical.
