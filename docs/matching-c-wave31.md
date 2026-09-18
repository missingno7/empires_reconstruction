# Thirty-first local C wave: critical-error handler pair

F_622C and F_625D add 58 matching C bytes. The handler records the DOS error
byte, selects the original recovery helper and returns the historical status;
the installer binds it to the pinned HARDERR public. DS:C0C8 is proved as a
one-byte runtime value through the handler's write and independent reads in
F_643A and F_652A.

Coverage is now 223 matching C routines / 26,750 bytes. The full EXE and both
DAT archives remain byte-identical.
