# Twenty-eighth local C wave: staging-buffer loader

F_21DB adds 113 matching C bytes. The fresh Turbo C object reproduces the
complete stripe-copy loop, its `farmalloc` call, both `movmem` calls and all
13 fixups. The historical `_malloc` public in the pinned FMALLOC module is
bound explicitly.

The DS:96EE destination is a 674-byte buffer beyond the DOS load image. Its
binding is backed by independent direct-address observations: F_21DB writes
the buffer and F_2269 reads it. The buffer-storage evidence checks the address
instruction, function extent hashes, read/write split and out-of-image
placement without copying BSS bytes into the executable.

Coverage is now 219 matching C routines / 26,348 bytes. The full EXE and both
DAT archives remain byte-identical.
