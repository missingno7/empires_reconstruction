# Matching C wave 104

`DOS_STUB` now owns the complete 404-byte startup stub after the MZ header.
The source preserves the DOS entry path, interrupt-handler helpers, and their
tail table as one exact component. Its `mov dx, _TEXT+0xFA3` source expression
recreates the single historical OMF fixup and MZ relocation at load offset 1.

Fresh compile, bind, relocation, and complete-game checks pass. Raw executable
ownership is now 17,993 bytes.
