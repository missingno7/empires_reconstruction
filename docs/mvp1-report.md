# MVP1 result — 2026-09-18

**Full EXE identity achieved. No blocking container or linker differences.**

| Measure | Result |
|---|---:|
| Executable bytes | 79,154 |
| Accounted bytes | 79,154 (100%) |
| Matching C | 14,077 bytes / 125 owners |
| Matching ASM | 2,456 bytes / 20 owners |
| Raw fallback | 62,621 bytes / 64 owners |
| Other exact data | 0 bytes |
| Load image | EQUAL, 78,642 bytes |
| Relocation table and map | EQUAL, 106 entries |
| Full file | EQUAL |

Original and rebuilt SHA-256:

```text
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10
```

Bring-up passed successively with raw-only ownership, freshly assembled
`F_D89A`, and freshly compiled `F_56C6` alongside that ASM region. The final
import expands the same mechanism to 145 freshly rebuilt source regions.
Every region is compared in full, including applied fixups. There is no
matching exemption, runtime fallback, cached-object acceptance or native port.

Seven automated tests passed, including actual Turbo C and TASM mutations
that preserve output length but change an instruction/operand. A deliberately
wrong external address binding, corrupt raw byte, truncated region, missing
relocation, ownership gaps/overlaps, malformed OMF and stale published success
are also rejected. `build/report.json` contains the current per-owner proof
and tool/source identities; this document records the milestone.

Deliberately deferred: stale upstream C proof `F_01CE`, library extraction,
runtime-generated memory, original linker/module reconstruction, semantic
analysis and decoding embedded/external assets. All on-disk EXE bytes still
have exact ownership. Upstream files were only read, and external DAT files
were not modified.
