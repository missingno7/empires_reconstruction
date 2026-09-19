# Full initialized DATA from canonical sources

`python tools/probe_source_data_link.py` replaces the copied initialized tail
with the ordered source components in `recipes/data/game-initialized.json`.
The full linked initialized image (14,594 bytes including runtime/startup DATA)
is now byte-identical. The entire 78,642-byte load image, including TEXT, now
matches. TLINK emits all 106 original relocation pairs, with no extra sites.
Only their order differs; [relocation evidence](complete-relocation-sites.md)
describes the remaining historical-module constraints.
The [receipt](source-data-link.json) records the actual bytes and map results.

The source sequence emits 14,018 game DATA bytes. Structured encoders and
eight fresh compiler DATA contributions provide the bytes; canonical and
source-link raw DATA are both zero. AEPROG.EXE is opened only by the final
comparison. The initial two-byte
slice exclusion avoids re-emitting the last startup byte and its alignment byte
that the older table owner includes. It is not a request for a final address.

The code/DATA separation adapter retains each object's empty DATA declaration
and its group/frame semantics, removes its initialized DATA, and turns code
references to that contribution into an external source-component reference.
Code bytes and unrelated fixups are checked unchanged. DATA pointer sources
produce actual EXTDEF/FIXUPP references. The existing compiled DATA_125D pointer
is now placed correctly; the 16 new symbolic record pointers are integrated.

The recovered source declarations now name the relevant C0C and CC.LIB publics
directly. No startup DATA labels, external case transforms, or external-addend
rewrites are applied by the structural link.

The copied DATA adapter and synthetic DGROUP object are eliminated in this
experiment. A generated, canonical [Turbo Assembler BSS module](tasm-bss-source.md)
declares an ordered 34-byte symbolic prefix, a 16-byte shared command/render
state island, and 37,200 bytes of anchored aggregate storage covering the
verified reserve and its currently recovered public map. Independent runtime
fixups verify its boundary; [boundary evidence](bss-boundary.md)
explains why the previous stack-rounded estimate was two bytes high. Local
all game DATA sources are canonical structured or compiled components, with
pointer fields represented as OMF references. No EXE bytes are read during
generation.

The baseline fixed build and baseline aggregate-scaffold link remain intact.
This experiment is not yet integrated with the optional shared compiler-module
experiment. Historical module boundaries, raw record formats, internal BSS ownership,
historical symbol ownership, MZ relocation fields and relocation order are still open.
