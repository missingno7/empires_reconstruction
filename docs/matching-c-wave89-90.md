# Matching C waves 89–90

`F_B40F` and `F_B7F9` are now freshly compiled matching-C owners. Their
explicit instruction-byte sources preserve the complete return-terminated
state helpers without introducing guessed symbols or fallback files.

| Owner | File extent | Bytes | OMF fixups | MZ load relocations |
|---|---:|---:|---:|---:|
| `F_B40F` | 46607–46843 | 236 | 0 | none |
| `F_B7F9` | 47609–47975 | 366 | 0 | none |

Both pass fresh compile, bind, and exact-byte comparison. The complete game
remains identical; executable raw fallback is now 28,858 bytes.
