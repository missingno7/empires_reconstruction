# audio sources (target_sources(empires_core PRIVATE ...))
target_sources(empires_core PRIVATE sound_stub.c)  # placeholder anchor; sound.h is fully implemented by sound_driver.c now
target_sources(empires_core PRIVATE sound_driver.c)  # asm/SOUND.ASM port (Phase 12 part 1)
