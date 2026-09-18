# Runtime library ownership wave

The six previously held CC.LIB modules ATEXIT, EXIT, IOERROR, OPEN, SETARGV
and SETENVP now have explicit component ownership. Their `_DATA`/`_BSS`
segment bases and external targets were derived from complete library fixups,
object addends and the original linked words, then checked by fresh
`bind_region` proofs. Together they remove 860 bytes from raw fallback while
preserving the exact executable.

The matching-C count is unchanged; the remaining executable blocker is the
76-byte F_4E9F matching-ASM extent, whose two DS targets still need an
independent binding proof.
