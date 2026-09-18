# Tenth local C wave: independent text components

F_A33F's complete 203-byte C body is now owned. Its previously held string
references resolve through two independently encoded data components:

| Owner | Address | Bytes |
|---|---|---:|
| TEXT_12D9 | DS:12D9 | 12 |
| TEXT_1660 | DS:1660 | 15 |

These are complete null-terminated ASCII strings. Their source documents under
src/data contain text, including explicit control-character escapes where
needed. The ascii-nul-v1 encoder requires exactly format and text, encodes
ASCII and appends one terminator. It rejects embedded zeroes, unsupported
characters and extra fields. It never reads original game files to generate
bytes. Each string has a separate owner, preserving the distinct placements
without pretending they form one contiguous historical compiler contribution.

C bindings derive both string addresses from these owners. The recipe tool now
accepts this identified text alongside C and compiler-generated data, checks
all candidate bytes and text relocation absence before changing ownership,
and reports encoded-text bytes separately from compiler-generated data.
Bootstrap extraction skips canonical text documents instead of overwriting
them from the original EXE.

Tests cover fresh C plus both encoded strings, malformed sources, byte-exact
round trips, text edits and placement changes. A same-length text edit changes
the output; moving a text owner changes its bound code reference. The normal
whole-game build and bootstrap path both pass.

Coverage: 190 matching C functions / 19,505 bytes. Classified static data is
86 bytes: 59 compiler-generated and 27 independently encoded. Raw EXE bytes
are 50,792, including 29,602 classified unresolved machine bytes. All three
files remain byte-identical. Fixed placement still exists, and the overall
matching-C goal is unfinished.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave10.json
python tools/extract_raw.py
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Recipe and fresh metadata proof: recipes/c/matching-wave10.json and
matching-wave10-evidence.json.
