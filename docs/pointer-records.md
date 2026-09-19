# Symbolic pointer records

DATA_011D90_RECORDS replaces the former 160-byte RAW_011D90 owner. Eight
20-byte records contain a u16, a far pointer and byte, another far pointer and
byte, then eight bytes. The 16 pointer segment words coincide exactly with MZ
relocation sites. All pointer targets are the beginnings of existing structured
string owners. Canonical source records name those owners and contain no final
load addresses or segment values.

Independent code references also touch record starts: F_CDDD uses DGROUP 219C
(record 3), F_CE00 uses 21B0 (record 4), and F_CE2A uses 21EC (record 7). These
support the repeated structure without assigning gameplay types to its fields.

`pointer_records.compile_records` emits unbound bytes and symbolic references.
The fixed verifier resolves those references using its existing manifest
coordinates. `records_object` instead emits byte-aligned public DATA with
EXTDEFs and DGROUP-framed pointer32 FIXUPPs; TLINK determines the addresses.

Run `python tools/probe_pointer_records.py` for the independent source-only
link experiment. Its [receipt](pointer-record-link.json) verifies 160 linked
bytes against emergent target publics and all 16 relocation sites. It reads
canonical source files, not AEPROG.EXE. The minimal DATA-only executable has no
stack, producing the expected warning. This is a component link test, not a
claim of a runnable game or historical module recovery.

The full fixed build still matches all three original files. Both typed tables
are integrated into the source-DATA TLINK path, including the six-record table's
10 descending fixups. Canonical raw EXE ownership is now 4,012 bytes across 10
owners. Exact historical module ownership remains open; the typed objects prove
layout and fixup behavior rather than translation-unit boundaries.
