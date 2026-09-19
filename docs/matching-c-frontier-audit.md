# Matching-C frontier audit

The pinned correspondence currently contains 146 proven C/ASM entries. Every
one of those extents is already covered by a current matching-C, matching-ASM or
known-library owner. The current manifest has 345 matching-C owners, zero
matching-ASM owners and no unresolved machine extent intersecting a RAW owner.
The held-linkage snapshot also has zero candidates and zero unresolved symbols.

`python tools/audit_matching_c_frontier.py` derives these facts from
`docs/upstream-inventory.json`, `docs/linkage-blockers.json` and
`layout/manifest.json`; `tests/test_matching_c_frontier.py` keeps the audit
reproducible. This closes the currently evidenced C/ASM promotion frontier. It
does not claim that a new decompilation outside the pinned correspondence is
impossible, so further executable work would require new disassembly or module
evidence rather than an untried held candidate.
