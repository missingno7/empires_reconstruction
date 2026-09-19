# Matching C wave 105

`RUNTIME_BLOCK` now owns the complete 6,571-byte raw runtime dispatch/code
block after the DOS startup stub. Its source preserves the embedded dispatch
table and all code/data bytes explicitly, with an exact OMF boundary label and
no external fixups or loader relocations.

Fresh compile, bind, and complete-game equality pass. Raw executable ownership
is now 11,422 bytes.
