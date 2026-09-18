# Exact archive reconstruction

The pinned originals are `assets/AE000.DAT` (227,560 bytes, 89 resources) and
`assets/AE001.DAT` (383,874 bytes, 131 resources). Both rebuild in full. The
offset table, resource headers, decoded identities and ownership are checked
into `layout/archives/`; all game payloads and structured asset sources stay
under ignored `raw/`. No original DAT, extracted image, level or compiler
binary is distributed.

This page describes the fixed-layout scaffold and component proofs. The
independent [structural packing path](build-reconstruction.md) now derives
all offsets from emitted component sizes. It uses `recipes/archives/`, builds
without original files or fixed manifests, and verifies against this scaffold
and the originals afterward. Unknown payload fallback remains temporary; full
byte equality is not a claim that the original build system is recovered.

```powershell
python tools/reconstruct_archives.py prepare # restore local sources once
python tools/reconstruct_game.py             # fresh EXE plus both DATs
python tools/probe_archive_codecs.py           # compare all encoder candidates
python tools/probe_resource_formats.py        # emit local structural research
```

`reconstruct_archives.py` without a command rebuilds only the DATs. `init` is
a one-time importer and never replaces an existing manifest. The normal build
does not initialize sources or read upstream. `prepare` explicitly derives
local sources from the pinned originals. Canonical structured resources are
JSON; other exact compressed resources use decoded bytes; remaining resources
retain their encoded payloads. The builder compares originals but never fills
a missing source from them.

## Physical layout

The first LE32 is the byte size of the offset table. There are `size / 4`
offsets, including the terminal sentinel; resource count is one less. Each
resource occupies `[offset[i], offset[i+1])`. Nonempty blocks begin with a
one-byte type and one-byte compression flag. Duplicate offsets represent
explicit empty slots. Bytes after the terminal offset, if any, have a separate
trailing owner. The originals have no empty slots or trailing bytes; synthetic
tests ensure neither would be discarded. Tables, headers, lengths, resource
digests and final archive hashes must all agree.

## Compression evidence

Decode flag bit 1 (`2`) first, then flag bit 0 (`1`). Encoding reverses that
order. Other flag bits fail explicitly. All 220 decoded payloads independently
agree with the upstream decoder. [codec-evidence.json](codec-evidence.json)
records its source hash, each stream's digest, candidate result and first byte
and token differences. This is a snapshot; rerun the probe after codec changes.

**Signed-count RLE:** positive controls copy 1–127 literals; zero or negative
signed controls repeat the following byte `1 - control` times. The encoder
selects runs of 2–129 bytes and ends literals before the next run or at 127
bytes. All **155 observed RLE stages re-encode exactly**, including their
historical run/literal choices. This alone does not prove the surrounding
pair-span stream matches.

**Pair-span codec:** the first LE16 equals the decoded stage length in all
182 original streams. Codes are MSB-first, initially nine bits wide. Literal
codes are 0–255; code 256 increases width. Code `257 + n` references the output
span of the nth completed pair of non-escape codes. Each pair creates a new
dictionary entry. All original streams terminate at the declared output size,
with at most seven zero padding bits. The strict decoder rejects missing
references, oversized output, truncated codes and nonzero/excess trailing bits.

The candidate encoder chooses the longest established byte span and grows the
width only when a selected code requires it. Earliest/latest duplicate-span
tie policies both reproduce only **26 of 182 compressed streams** (3,721
encoded payload bytes). All 156 first token divergences choose a shorter
span in the original than in the candidate. For example, AE000:006 token 81
at stage offset 110 uses code 268 (two output bytes); the candidate uses
already-established code 281 (six bytes). This identifies match selection or
dictionary availability as the next question; it does not establish the
historical encoder algorithm. Shorter candidate output is still a mismatch.

The probe's `--promote` option accepts only full original encoded equality.
It verifies both complete archive candidates before changing ownership and
invalidates published archive/game success when it promotes anything.
Uncompressed identity copies do not count as compression recovery.

## Structured assets and levels

All 49 standalone type `0x47` payloads have exactly 34 header bytes plus
`row_bytes * height` packed pixel bytes. The source preserves both 16-byte
display lookup tables, dimensions and rows of nibble pixels (high nibble
first). Each hex digit in `pixel_rows` is one logical pixel; no palette
conversion, colour quantization or transparency normalization occurs.

All 20 level payloads are exactly two 13,068-byte parts. Each part preserves
a 64-byte header, ten 1,000-byte room records, a four-byte separator and a
3,000-byte record block. Each room splits into two preamble bytes, eighteen
38-byte tile rows and 314 preserved bytes. Cross-boundary link fields are not
duplicated or normalized. The remaining record block and unknown fields stay
explicitly opaque; this does not claim actor/script recovery.

All **69 structured payloads round-trip exactly**. Twenty-five bitmaps and
uncompressed level AE001:000 additionally reproduce their complete original
resource blocks and now use structured canonical sources. The other 43
structured payloads remain derived research under `build/resource-structures/`
because recompression differs. One further exact compressed resource uses a
decoded-byte source, for **27 matching resources overall**. The other 193
resources retain raw encoded payloads. `probe_resource_formats.py --promote`
rechecks full resource and archive identity before promoting a structured form.

These levels/bitmaps follow the physical formats documented in the read-only
`ancient-empires-reverse-engineered` project's
`games/ancient_empires/dat_archives.md`, `graphics.md`, and
`ancient_empires/game_data/level_format.py`. The reconstruction contains its
own strict parsers/encoders; the upstream module was used only for the
independent decoder comparison. [resource-format-evidence.json](resource-format-evidence.json)
records payload and compressed-stream proof separately.

## Reports and failure handling

`build/archives-report.json` reports full identity, type counts, decoded and
structured payload coverage, canonical matching resources, exact compressed
bytes, raw bytes and implementation hashes. `build/game-report.json` combines
those with the EXE's source/library/header/raw coverage and linkage evidence.
Raw byte identity and reconstructed representation depth are separate facts.
Structured header byte counts overlap resource byte counts and must not be
added to them as independent coverage.

Both archives are verified before either output is published. A failed archive
build removes older DAT outputs and archive/game success reports. A direct EXE
build also invalidates combined game success. Individual successful files may
remain after a different component fails, but no combined success is published.
