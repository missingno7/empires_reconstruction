# Nested formats and editable image sources

The structural payload frontier now covers **153 resources**, up from 69:

| Payload representation | Resources |
|---|---:|
| Standalone 4-bit bitmap | 49 |
| LE16 resource bank | 75 |
| Sequential bitmap bank | 7 |
| Contiguous 1-bit font | 2 |
| Two-part level | 20 |

All payloads reproduce every original decoded byte. They contain 596 nested
4-bit bitmap records, 49 standalone bitmaps, 182 monochrome records, and 256
font glyphs. Mixed banks retain **100 opaque records explicitly**. Level
record blocks and unknown fields likewise remain preserved; structural
partitioning is not a claim that their formats are fully recovered.

`python tools/probe_resource_formats.py` regenerates local JSON research sources
and a report with format/record counts and separate payload/compressed equality.
The metadata-only snapshot is [resource-format-evidence.json](resource-format-evidence.json).

## Derived internal layout

Type-1 banks contain an LE16 offset table with an end sentinel. The encoder
derives its size from record count and derives every offset from encoded record
lengths. Empty slots and trailing bytes are preserved. Nested `0x47` records
retain their control byte and full bitmap source; other known records can
have a dedicated encoder, while unknown records retain `opaque-record-v1`.
Offsets beyond 16 bits are rejected instead of wrapped.

The seven sequential banks contain only complete `0x47` records. Their encoder
concatenates emitted records; the decoder rejects incomplete or unowned tails.
The 182 `0x32` records in AE001:034 contain four header bytes and packed 1-bit
rows. Width is twice the width-pairs field; the row stride is rounded up to
whole bytes. Control bytes and row padding bits survive unchanged.

AE000:000 and :001 have a control byte, glyph-count-minus-one, line height,
widths, low offset bytes, high offset bytes, then glyph rows. In these two
fonts every glyph is contiguous in table order. The encoder derives offsets
from `ceil(width / 8) * line_height`, preserving all row padding bits and any
trailer. Aliased/gapped font variants are explicitly rejected by this adapter.
Resources :002 and :003 also have type `0x46` but are not classified as fonts;
the proven resource identities, not the type byte alone, select this adapter.

Format evidence came from the read-only upstream project's
`ancient_empires/game_data/game_graphics_records.py` and
`ancient_empires/rendering/bitmap_font.py`. Strict complete-payload identity
was established independently here; upstream permissive rendering does not
grant lossless ownership by itself.

## PNG plus JSON as canonical source

Twenty-five standalone images now use local `raw/AE000/png/NNN.png` pixels
plus `NNN.json` display metadata. The JSON retains both original 16-byte colour
lookup tables. PNG dimensions generate the DOS dimensions; pixel indices
generate high-nibble-first packed rows. AE000:080 additionally moved from an
unstructured decoded blob to a structured four-image bank. Together with the
first level, all **27 currently matching resources have structured sources**.
The number of exact compressed resources remains 26; these source promotions
do not disguise unresolved compressor-policy mismatches.

The PNG is a **logical-index editing surface**, with 16 distinct grayscale
palette labels for pixel indices 0–15. It is not a colour-correct rendered
preview: actual EGA/CGA/VGA mappings remain in JSON, and DOS transparency is
determined by the pixel index. This avoids losing distinct logical indices
when display colours happen to be duplicated.

Keep PNGs indexed, 4-bit or 8-bit, non-interlaced, with the logical palette
unchanged. The decoder supports all five PNG row filters and validates CRCs,
dimensions, palette indices, chunk ordering and exact deflate termination.
RGB conversion, palette reordering, added alpha and out-of-range indices fail
explicitly instead of quantizing or remapping pixels. No external image
library is required by the build.
An independent Windows System.Drawing decoder also checked all 25 canonical
PNGs against the original payloads: **9,762 logical pixels agree**.

```powershell
# Export a standalone bitmap JSON research source to an editable pair:
python tools/bitmap_sources.py export build/resource-structures/AE000_005.json build/image-edit/005.json
# Encode such a pair to the decoded DOS bitmap payload:
python tools/bitmap_sources.py encode build/image-edit/005.json build/image-edit/005.bin
# Encode all adopted source components and derive archive placement:
python tools/pack_archives.py
# Check unchanged originals separately:
python tools/pack_archives.py verify --fixed-output build
```

`tools/promote_bitmap_png.py` adopted the 25 pairs only after encoding each PNG
back to its complete original compressed resource. `prepare` recreates the
local pairs on a fresh checkout. Source PNGs, JSON asset contents, raw payloads,
and generated files remain excluded from Git. Only parsers, recipes, tests,
proof digests and documentation are committed.

## Modified DOS assets

After a supported PNG pixel/dimension edit, the same packer emits a changed
resource and computes subsequent offsets from its actual compressed length.
A regression test edits the PNG, builds the archive without fixtures, decodes
the emitted resource and checks the changed dimensions/pixels and offset shift.
Original-equality verification intentionally fails for modified content.
This proves the encoding/packing path, not that arbitrary dimension changes
are accepted by every game caller. Game-specific usage constraints remain to
be recovered; the fixed scaffold still enforces original identity.
