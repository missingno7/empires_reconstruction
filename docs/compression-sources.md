# Explicit compression source experiment

Both DAT archives can now be rebuilt byte-for-byte in an independent research
path using decoded payload sources and explicit pair-span instructions. It
needs no original archive or fixed-layout manifest during generation.

```powershell
python tools/probe_compression_sources.py export
python tools/probe_compression_sources.py build
```

Export verifies original identities, creates ignored local sources under
`build/compression-sources/`, checks every codec round trip, and verifies both
complete archives. It refuses an existing directory to protect source edits;
use `--directory build/another-name` for another export. Build reads only that
directory and reports BUILT_UNVERIFIED; verification is the separate export
receipt, not a claim inferred from an old successful run.

The recipe contains resource order, type, compression flags and source paths.
It contains no original offsets, compressed bytes, expected lengths or hashes.
Archive offsets and sizes are derived by packing encoded components.

182 compressed resources have `pair-span-instructions-v1` sources:

* `literal, count` reads that many bytes from the decoded compression-stage
  input. It does not duplicate the literal values in the plan.
* `pair, index` names an earlier two-token emission span. Encoding checks that
  its bytes match the current input before writing the reference code.
* `widen, 1` emits the width escape, preserving even an early legal escape.

Pair references preserve dictionary identity, including duplicate spans. Size
headers and zero bit padding are derived. The already matching RLE policy
reconstructs the intermediate stage from decoded payloads. 153 payloads use
existing structured formats; 67 remain explicitly opaque decoded binary
sources. Nested opaque fields inside structured formats also remain opaque.
Nothing about this representation upgrades their semantic classification.

Unchanged sources reproduce all 611,434 archive bytes exactly. Compatible
payload edits use the same encoder; an edit that invalidates a pair reference
fails and needs a revised plan or a different compressor policy. The plans
are explicit compression syntax, **not recovery of the historical policy**
that selected it. The automatic greedy policy still matches only 26 compressed
resources. A second token-pair transition experiment did not improve that
count and was not adopted.

This path is deliberately separate from canonical ownership: 27 canonical
matching resources and 193 encoded raw fallbacks are unchanged. It advances
the research representation of the 156 unresolved compressed streams and
isolates their parse decisions. Source plans and decoded game content stay
ignored; only tooling, tests and a metadata-only evidence receipt enter Git.
