# Seventeenth local C wave: complete switch contribution

F_703E reproduces all 292 bytes from C, including its five-entry switch table
at CS:705D..7067 and the complete return path. Explicit cases 1, 2, 3 and 5
leave selector 4 targeting the common continuation. The range check handles
all selectors outside 1..5. The compiler generates the table inside the
complete code contribution; no original table bytes are copied into source.

Fresh OMF binding verifies 25 fixups and the full byte sequence. There are no
initialized-data contributions or MZ relocation sites. All external references
agree with exact profile declarations; three code bindings derive addresses
from existing component publics. A fresh negative control changes case 5 to
case 4 and must fail complete comparison, including dispatch layout.

Coverage is 204 C functions / 22,145 bytes. Raw EXE coverage is 48,138 bytes
across 66 regions, including 26,990 classified machine bytes and 21,148 unknown
bytes. The complete EXE and both DAT archives remain exact.

## Pending conversion candidate

F_28AC has a complete 218-byte extent. A C probe with unsigned SI/DI coordinates
and byte accesses emits 214 bytes. Original conversions zero AH and DX before
moving AX into the coordinate register; the tested byte casts omit the DX
operation and use a different AH instruction. Explicit long masks instead
emit additional AND operations (220 or 222 bytes depending on pointee type).
Changing the coordinate variables themselves to unsigned long introduces
LXURSH@, which is also unlike the original sequence. Stack-local ordering was
recovered, but these conversion expressions are not a match. No ownership or
matching-C credit is claimed for the probe. Further expression or type recovery
remains open, as do the other C candidates.

These results establish fixed-placement matching components, not historical
module boundaries, complete storage allocation or linker-derived layout.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave17.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave17-evidence.json.
Recipe: recipes/c/matching-wave17.json.
