# Runtime library ownership wave

The six previously held CC.LIB modules ATEXIT, EXIT, IOERROR, OPEN, SETARGV
and SETENVP now have explicit component ownership. Their `_DATA`/`_BSS`
segment bases and external targets were derived from complete library fixups,
object addends and the original linked words, then checked by fresh
`bind_region` proofs. Together they remove 860 bytes from raw fallback while
preserving the exact executable.

Later matching-C waves independently converted every remaining ASM extent,
including the interrupt and hardware bodies. The current executable census is
345 C owners and zero ASM owners; this historical library wave remains useful
as the provenance record for the six pinned CC.LIB modules.
