# Upstream audit and reuse

This records the initial MVP1 audit/import. Subsequent promotions, including
`F_01CE` and all 35 library modules, are described in [progress.md](progress.md).
The canonical manifest and fresh build report describe current ownership;
the JSON inventory below retains the initial audit rather than being another
source of current ownership.

Read-only source audited on 2026-09-18:
`D:/Games/DOS/dos_recosystem/empires_forged`.
The current matching material is in `controls/correspondence/`; older pilot
paths mentioned inside its comments are historical provenance.

| Artifact | Relevance / disposition |
|---|---|
| `controls/correspondence/F_*.json` | Extent addresses, file offsets, sizes, digest, source public/segment, flags and declared bindings. Imported into canonical owners. |
| `controls/correspondence/c/`, `asm/` | 126 matching C and 20 matching ASM entries. Copied only selected matching sources, preserving their bytes. |
| `controls/correspondence/records/` | Existing `EQUAL` receipts, source/toolchain identities, resolved bindings and relocation obligations. Used for audit/import, never accepted as a substitute for rebuilding. Receipt hashes retained per owner. |
| `profile.tc20-tasm10-dosbox.json` | Turbo C 2.0/2.01 identity, TASM 1.0, compact model, flags `-c -mc -1- -f- -N-`; entry-specific `-B` and `-k -B` retained. |
| `dosbox.conf` | DOSBox Staging environment: `svga_s3`, 16 MB, automatic core/type, maximum cycles, sound disabled. Recreated with a project-local mount and batch; new config hash recorded each build. |
| `layout-bindings.json`, profile layout | Physical frame information, global/function bindings and documented naming conventions. Only bindings consumed by selected extents are frozen in the manifest. |
| `toolchain/dos/TC/BIN` | Only `TCC.EXE`, `CPP.EXE`, `TASM.EXE` needed. Copied locally, digest-checked, Git-ignored. |
| `CC.LIB`, `C0C.OBJ`, `LIB_*.json` | 35 known toolchain-library entries. Audited but remain raw; no final historical link is attempted. References to helpers use their declared original addresses. |
| `ARENA_1049C.json` | One `GENERATED_AT_RUNTIME` entry. Does not grant ownership of original file bytes from a runtime memory address. Left outside this source import. |
| Remaining entries | 186 `UNRECOVERED_MACHINE` entries. Their file bytes are covered by raw owners; no decompilation attempted. |
| `port_forge/tools/pf_match.py` | Reused only `ObjectModule`, `ObjectReader`, `OmfReader` as a local source snapshot in `tools/omf.py` (about 370 lines). Original file digest in `layout/toolchain.json`. |
| `pf_match` comparison/provider | Studied fixup arithmetic and public-span selection. Its decoder, voting heuristics, VM, native recovery, model-wide gates and compiler runner are not build dependencies. |

The standalone builder adds OMF record framing/checksum checks around the
reused reader, resolves a deliberately limited set of fixup forms, and compares
every emitted byte. Unlike the upstream correspondence comparison, it does
not exclude external control transfers or infer bindings from the comparison
image. Fresh proof: 1,485 applied fixups and 12 source-derived MZ relocation
locations in the imported 145 regions.

`F_01CE` was skipped because the current source digest matches neither its
receipt's source bytes nor their CRLF-normalized form. The other 125 C and
all 20 ASM entries rebuild exactly. Several source digests in older receipts
refer to CRLF staging; their current LF form normalizes to those exact bytes.
These are distinguished from the actual stale `F_01CE` proof. No source was
rewritten or re-decompiled to increase the imported count.

The machine-readable [upstream-inventory.json](upstream-inventory.json) records
all 368 entries, their extents and import status. It is an audit inventory,
not a second ownership manifest. Per-owner provenance includes source, entry,
receipt and profile hashes; new build receipts record the actual compiler
commands, source/OBJ digests and applied fixups.

## Reimporting (maintenance only)

For incremental work, prefer `tools/promote_upstream.py`, documented in
[progress.md](progress.md). It replaces only raw owners, freshly verifies all
candidates before publication, and preserves existing recovered regions.
The initial bulk importer below resets ownership to its initial C/ASM subset,
so it would discard later library promotions from the layout.

```powershell
python tools/import_upstream.py
```

This explicitly replaces the canonical layout, copies selected upstream
sources, and extracts raw gaps from the pinned original. It is never run by
the build. Use a clean working tree and review the resulting changes before
adopting a new upstream snapshot. `--ids F_D89A` or
`--ids F_D89A F_56C6` limits the import; `--ids` with no values creates the
raw-only baseline. `--upstream PATH` changes the read-only source location.
Old unreferenced raw files are not deleted automatically. They have no
ownership and can be removed after reviewing the new manifest.
