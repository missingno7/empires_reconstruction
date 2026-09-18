# Thirtieth local C wave: menu dispatcher initializer

F_A658 adds 272 matching C bytes and its one-byte compiled `_DATA`
initializer. The menu loop, sparse key switch and complete teardown match from
a fresh Turbo C object; the empty-string initializer is owned separately at
DS:13C5 and is bound through the module's `_DATA` segment.

Coverage is now 221 matching C routines / 26,692 bytes. The full EXE and both
DAT archives remain byte-identical.
