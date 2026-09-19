# Complete relocation sites, ordering still open

The source DATA path emits all 106 original relocation entries, with exactly
the original segment:offset pairs. The load image and all fixed MZ fields match.
The first file mismatch is 0x3A; every differing byte lies inside the relocation
table. No output header or relocation-table rewriting is performed.

Another six 20-byte records are now symbolic source (DATA_010FA5_RECORDS):
ten far pointers and two null pointers. The last two records replace the
misleading split between a one-byte compiled initializer and 43 raw bytes.
Fresh Turbo C compilation independently reproduces both typed record tables,
280 DATA bytes and 26 fixups. Its [receipt](record-compiler-evidence.json) proves
the layout and descending fixup order, not historical translation units.

The remaining 27 pointer fields are described in the ordered DATA recipe as
component-relative offsets, symbolic target owners and addends. The emitter
replaces their raw words with unbound pointers and emits genuine OMF FIXUPPs.
One target is within the source-declared game BSS reserve. Surrounding payloads
remain explicitly `raw-local`: annotating their pointers does not make their
unknown record or command structure understood. Canonical raw ownership is
4,012 bytes across 10 owners; the source DATA path uses 4,006 of those bytes.

DATA fixups are emitted in the descending order independently observed in
fresh Turbo C objects. Remaining relocation order differences cross source
contributions. For example, the original lists the three relocations of the
shared-string candidate in reverse code order; independent function objects
produce forward order. Original DATA relocations are also interleaved with
code relocations, rather than appended after every code object.

These are constraints on historical module composition and object order. The
next step is to test those constraints with shared source compilations and
relocatable components, not to reorder the finished MZ table. Byte identity
alone would still not close raw source, BSS partition or symbol-adapter work.
