# compat sources
target_sources(empires_core PRIVATE dos_compat.c sync_win32.c sha256.c clock.c)

# cclib.c (Turbo C 2.0 CC.LIB semantics: movmem/setmem, rand/srand LCG,
# itoa/ltoa/ultoa) and dosio.c (save-slot file I/O + BIOS tick clock over
# stdio) -- see portable/include/cclib.h / dosio.h.
target_sources(empires_core PRIVATE cclib.c dosio.c)
