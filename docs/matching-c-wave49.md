# Forty-ninth matching-C wave

This pass tested three complete raw executable extents with fresh Turbo C
objects. No candidate passed the exact full-extent comparison, so ownership and
the exact game build remain unchanged.

| Candidate | Original | Fresh probe | Result |
|---|---:|---:|---|
| F_D818 | 13 bytes | 21 bytes | DI store lowered through a longer pointer sequence |
| F_C877 | 33 bytes | 33 bytes | register save differs at byte 3 (`push cx` versus `push si`) |
| F_D825 | 58 bytes | 76 bytes | packed pointer/record writes spill into a longer C sequence |

The machine-readable [probe evidence](matching-c-wave49-evidence.json) stores
the source and object hashes, complete probe bytes, and first mismatch facts.
The probes remain ignored research artifacts; no raw bytes are used as source
inside a claimed C owner.

`F_7202` is not part of this open set: its 104-byte C contribution was already
promoted in wave six and continues to rebuild exactly. The remaining raw
frontier is reduced only by future complete proofs or independently verified
format/data ownership.

```powershell
python -m unittest discover -s tests -q
python tools/reconstruct_game.py
```
