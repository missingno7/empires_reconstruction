# Seventy-eighth matching-C wave

`F_652A` occupies file offsets `26410..26476` (load offsets `0x652A..0x656C`)
and is now a complete matching-C component. It allocates the historical
0x200-byte stack buffer, reads three sectors through BIOS `INT 13h`, obtains the
drive number from `DS:C0C8`, and finishes with DOS disk reset `INT 21h/AH=0Dh`.

The canonical source uses inline assembler for the exact BIOS register and
stack-buffer sequence. Turbo C emits all 66 bytes with one DGROUP fixup for the
drive byte and no MZ loader relocations. Fresh binding and byte comparison pass,
and the complete EXE and both DAT archives remain byte-identical.
