# Historical startup and runtime DATA publics

Four reconstructed C references previously used address-derived names and
required the structural linker path to modify historical objects or caller
EXTDEFs:

| Previous reference | Historical public | Provider |
|---|---|---|
| `_g81` | `__8087` | `C0C.OBJ` |
| `_g6d` | `__argv` | `C0C.OBJ` |
| `_g7d` | `__osmajor` | `C0C.OBJ` |
| `_g37cb` | `__ctype` with source index `+1` | `CC.LIB` CTYPE |

The C sources now use the historical declarations directly. For CTYPE, the
one-byte EOF sentinel is expressed by indexing `_ctype[c + 1]`, so the emitted
EXTDEF targets `__ctype` and the compiler preserves the original address in the
instruction displacement.

Fresh Turbo C objects retain exact code bytes. The fixed reconstruction and
Turbo Link 2.0 build remain byte-identical. The source DATA report now records
empty `temporary_startup_data_aliases` and `temporary_runtime_data_aliases`.
