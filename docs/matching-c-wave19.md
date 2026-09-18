# Nineteenth local C wave: verified library data

F_A525 now reproduces its complete 307-byte code contribution. Placing the
assignment inside the switch expression avoids an extra three-byte stack
reload. The sparse selector and target tables are emitted by Turbo C; no
original instruction or table fragments fill the C contribution.

## Complete CTYPE contribution

The pinned CC.LIB contains a CTYPE module with a complete 257-byte _DATA
segment, empty _TEXT and _BSS segments, and the __ctype public at data offset
zero. The module SHA-256 is:

`099281b47a8ab616a91c74516c8f084301b442cf1e1fea24765ada2bd59f7083`

Its complete data segment matches file bytes 78,842..79,099 (end exclusive),
DS:37CA..38CB. LIB_CTYPE_DATA now owns that entire contribution. Generation
reads the identity-checked local library, not original game bytes. Each build
verifies both library and module hashes, full segment size, all emitted bytes
and relocation obligations. No library binary is committed.

The C routine references the verified __ctype public plus one, reaching the
character-indexed portion after the initial sentinel entry. DGROUP library
bindings now require the verified _DATA module and exactly one matching public.
Both public and addend must fit the owned contribution. Fixed placement remains;
this does not recover the historical linker layout. Library code and data are
reported separately (4,267 and 257 bytes), with 4,524 total runtime bytes.

## Independent initialized text

DS:12D0 holds eight underscores and a terminator, immediately preceding the
already owned TEXT_12D9. TEXT_12D0 encodes its complete nine bytes from ASCII
source. F_A525 fills and edits this buffer; the source represents its original
initialized state. Its binding derives from that data owner.

Fresh C comparison and a split-assignment negative control cover the complete
function. Library controls reject a changed table byte, shortened contribution,
wrong or missing public, missing verified module, wrong coordinate, out-of-range
addend, shifted public plus addend, and changed module identity. The standard
text encoder and full build check the initialized string.

## Remaining probes

F_4F96 was tested as C with nested sparse switches. Its 298-byte output differs
from the original 299 bytes: switch comparisons reverse operand encoding and
branch relaxation/padding differs. It remains unpromoted; investigating the
historical compiler-to-assembler path may be useful. F_ADCF continues to depend
on the undeclared DS:C360 array base. These are open reconstruction tasks.

Coverage is 207 matching C functions / 23,026 bytes. Raw EXE coverage is
46,991 bytes, including 26,109 classified machine bytes and 20,882 unknown
bytes. The complete EXE and both DATs remain byte-identical. The goal is active.

```powershell
python tools/promote_c_candidates.py --recipe recipes/c/matching-wave19.json
python -m unittest discover -s tests
python tools/reconstruct_game.py
```

Evidence: matching-wave19-evidence.json.
Recipe: recipes/c/matching-wave19.json.
