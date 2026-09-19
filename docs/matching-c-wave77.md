# Seventy-seventh matching-C wave

`F_4F63` occupies file offsets `20835..20886` (load offsets `0x4F63..0x4F96`)
and is now a complete matching-C component. It accepts the caller's far,
terminated buffer, calls the pinned `LIB_STRLEN` implementation, subtracts the
terminator, and issues DOS handle-2 write service `INT 21h/AH=40h` after loading
the caller's segment into `DS`.

The canonical source in `src/F_4F63.C` uses inline assembler only where the
historical register allocation and DOS ABI must be preserved. Turbo C emits the
complete 51-byte extent, including the frame and return, with one external
`_strlen` fixup. Binding that fixup to the complete `LIB_STRLEN` OMF public
reproduces the original bytes; the component has no MZ loader relocations.

The full EXE and both DAT archives remain byte-identical after promotion.
