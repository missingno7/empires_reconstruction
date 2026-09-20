# F_4B0C symbolic command interpreter

`F_4B0C` is a 915-byte non-returning command interpreter over 32-byte state
records. It is now reconstructed as symbolic TASM in
[`asm/F_4B0C.ASM`](../asm/F_4B0C.ASM), with named record-loop, dispatch,
state-update, collision, and opcode-handler control-flow labels.

The object has a 915-byte `_TEXT` contribution and no OMF fixups, matching the
historical direct-near-call representation. `CALL_REL` therefore deliberately
keeps each direct call's original PC-relative encoding while its source comment
records the recovered destination. The focused test fresh-assembles it, binds
the contribution, and compares the complete region with the verification
oracle.
