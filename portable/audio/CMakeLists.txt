# audio sources (target_sources(empires_core PRIVATE ...))
target_sources(empires_core PRIVATE sound_stub.c)  # placeholder anchor; sound.h is fully implemented by sound_driver.c now
target_sources(empires_core PRIVATE sound_driver.c)  # asm/SOUND.ASM port (Phase 12 part 1)

# Milestone F: PCM mixer + backends (docs/portable/architecture.md "Audio
# model"). opl_backend.c is the only translation unit allowed to include
# Nuked-OPL3's headers, so it links against the nuked_opl3 static library
# the top-level CMakeLists.txt's FetchContent block builds.
target_sources(empires_core PRIVATE audio_mixer.c speaker_synth.c opl_backend.c)
target_link_libraries(empires_core PUBLIC nuked_opl3)
