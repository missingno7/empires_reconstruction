# SDL3 platform layer: the ONLY directory allowed to include <SDL3/SDL.h>.
# Builds the `empires` executable (Milestone A: window + test-pattern present).
add_executable(empires
  main.c
  video_sdl.c
  video_sdl.h
  clock.c
  input_sdl.c
  input_sdl.h)

target_link_libraries(empires PRIVATE empires_core SDL3::SDL3)

set_target_properties(empires PROPERTIES WIN32_EXECUTABLE OFF)

# SDL3 is fetched as a shared library (SDL_SHARED ON); copy SDL3.dll next to
# the exe so it runs without installing SDL3 system-wide.
add_custom_command(TARGET empires POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:SDL3::SDL3>
          $<TARGET_FILE_DIR:empires>
  COMMENT "Copying SDL3 shared library next to empires.exe")
