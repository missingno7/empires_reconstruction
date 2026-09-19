# Full initialized DATA from canonical sources

`python tools/probe_source_data_link.py` replaces the copied initialized tail
with the ordered source components in `recipes/data/game-initialized.json`.
The full linked initialized image (14,594 bytes including runtime/startup DATA)
is now byte-identical. The entire 78,642-byte load image, including TEXT, now
matches. TLINK emits 72 of the original 106 relocation sites, with no extra sites.
The [receipt](source-data-link.json) records the actual bytes and map results.

The source sequence emits 14,018 game DATA bytes. Structured encoders, six
fresh compiler DATA contributions and local raw source components provide the
bytes; AEPROG.EXE is opened only by the final comparison. The initial two-byte
slice exclusion avoids re-emitting the last startup byte and its alignment byte
that the older table owner includes. It is not a request for a final address.

The code/DATA separation adapter retains each object's empty DATA declaration
and its group/frame semantics, removes its initialized DATA, and turns code
references to that contribution into an external source-component reference.
Code bytes and unrelated fixups are checked unchanged. DATA pointer sources
produce actual EXTDEF/FIXUPP references. The existing compiled DATA_125D pointer
is now placed correctly; the 16 new symbolic record pointers are integrated.

Three temporary DATA labels are added to the pinned startup object without
changing its bytes. The recovered `_g37cb` reference becomes `__ctype + 1` in
the runtime object. Existing manifest binding evidence still routes recovered
names to component-relative exports; this is an acknowledged symbol adapter,
not recovered historical declarations or a final architecture.

The copied DATA adapter is eliminated in this experiment. The remaining
37,250-byte unpartitioned BSS reserve is declared in source and verified against
independent runtime fixups; [boundary evidence](bss-boundary.md) explains why
the previous stack-rounded estimate was two bytes high. Local
raw game DATA sources account for 4,129 bytes, including the 34 missing pointer
relocations. Those source files are not read from the EXE during generation,
but remain opaque fallbacks requiring reconstruction. Six additional canonical
raw bytes are supplied by C0C's real EMUSEG/CRTSEG contributions in this path.

The baseline fixed build and baseline aggregate-scaffold link remain intact.
This experiment is not yet integrated with the optional shared compiler-module
experiment. Historical module boundaries, raw record formats, BSS ownership,
historical symbol ownership, MZ relocation fields and relocation order are still open.
