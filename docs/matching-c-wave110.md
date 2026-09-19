# Wave 110 symbolic-assembly conversion proof

F_D3CF is now a symbolic-assembly owner. Its complete 11-byte far-record
decoder continuation uses named instructions and a checked fixed-offset jump
into the preceding decoder body. This replaces the earlier inline `asm db`
wrapper. The complete non-returning extent has no fixups or loader relocations
and matches byte-for-byte.

The earlier matching-C proof remains historical evidence; the canonical source
is the readable TASM continuation in `asm/F_D3CF.ASM`.
