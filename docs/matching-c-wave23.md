# Twenty-third local C wave: nested sparse dispatch

F_86C9 reproduces its complete 321-byte function from C, including the nested
switches, eleven-entry sparse selector and target tables, loop conditions,
boolean inversion and complete return path. Turbo C emits the dispatch data
within the code contribution. No table or instruction bytes are copied from
the original as source.

The argument is accessed through a 16-bit field at offset zero and an unsigned
byte at offset eleven. A mechanical structure declaration preserves those
accesses. It does not identify all intervening fields or claim a complete data
format for callers. All external symbols have established declarations, and
15 references derive from existing component publics. Full fresh OMF binding
and byte comparison cover the complete contribution and relocation obligations.
A changed sparse-switch selector must fail the fresh compiler comparison.

The inspected F_AA1F has several still-undeclared structured-data references
and remains raw. F_6CA6 and F_4E9F use explicit segment/register state and LOOP;
F_51BF contains BIOS and port-I/O sequences. Those inspections do not establish
that matching C is impossible, and none is credited as a C match here.

Coverage is 213 C routines / 24,133 bytes. Raw EXE coverage is 45,884 bytes,
including 25,002 classified machine bytes and 20,882 unknown bytes. The full
EXE and both DAT archives remain byte-identical. Original modules, data formats
and linker relationships are still incomplete; the matching-C goal is active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave23.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave23-evidence.json.
Recipe: recipes/c/matching-wave23.json.
