# Turbo Link 2.0 full-scaffold checkpoint

The pinned Turbo Link 2.0 accepts the synthetic DGROUP object after its PUBDEF
and LEDATA records are split into bounded records. Previously the object held
PUBDEF bodies of several kilobytes and a roughly 14 kB LEDATA body. The change
preserves segment lengths, all public offsets and every initialized byte; it
does not introduce padding or move contributions.

Fresh full-scaffold links with and without the historical demand object have
identical segment maps, detailed code contribution rows and whole EXE hashes.
Both have zero linker diagnostics and zero unresolved symbols. Thus the demand
object is unnecessary for this full scaffold, not only for its partial link.

Reproduce the current experiment:

```powershell
python tools/probe_tlink_layout.py --promote-toupper --normalize-recovered-symbols --normalize-case-symbols --expose-internal-labels --scaffold-dgroup
python tools/report_structural_status.py
```

The generated [status snapshot](structural-status.json) records canonical
coverage, largest raw owners, actual segment sizes, synthetic bytes and byte
comparisons. It does not establish fixture-independent generation.

The EXE is 79,154 bytes and has the expected segment bases, CS:IP, SS:SP and
allocation values. It is **not byte-identical**. The first file mismatch is
`e_crlc` at file `0x06`: 56 versus 106 relocation entries. There are 51 missing
original sites and one extra site at load `0xFAFD`.

The first load-image difference is at `0xC8`, inside C0C's offset16 fixup to
`_BSSEND`. The linked value is `0xCACC`; the original is `0xCAC8`. Paragraph
rounding hides this four-byte boundary difference in the identical stack base.
Recovering BSS sizes must therefore compare the boundary publics as well as
the segment map. Do not compensate by adding or deleting arbitrary bytes.

The first initialized DATA difference is load `0xFAC4`. The synthetic DATA tail
still comes from the oracle and lacks its original pointer fixups. It also
follows library DATA rather than reconstructing historical module DATA order.
Real module contributions and pointer relocations remain the next frontier.

Symbol aliases remain provisional: the current adapter can expose the same
`_getkey` name in two code owners and can confuse the data `_mode` with the
code `_mode`. The existing duplicate-public suppression is not historical
symbol evidence. Source-specific binding recovery must replace these aliases;
zero unresolved names alone does not prove correct bindings.
