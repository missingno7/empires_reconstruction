# Portable game core: everything except the SDL3 platform layer.
add_library(empires_core STATIC)
target_include_directories(empires_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Each subsystem adds its own sources with target_sources(empires_core ...).
add_subdirectory(compat)
add_subdirectory(resource)
add_subdirectory(gfx)
add_subdirectory(audio)
add_subdirectory(generated)
add_subdirectory(game)

if(EMPIRES_BUILD_APP)
  add_subdirectory(platform/sdl3)
endif()

if(EMPIRES_BUILD_TESTS)
  add_subdirectory(tests)
endif()
