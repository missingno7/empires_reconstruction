# Portable source port of AEPROG.EXE (Windows x64 / SDL3).
# The historical Turbo C / TASM tree under src/, asm/, include/ is NOT built
# here; it stays the behavioral oracle (see docs/portable/architecture.md).
cmake_minimum_required(VERSION 3.24)
project(empires_portable LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(EMPIRES_BUILD_TESTS "Build the portable unit tests" ON)
option(EMPIRES_BUILD_APP "Build the SDL3 executable" ON)
option(EMPIRES_FETCH_SDL3 "Download SDL3 with FetchContent when no system SDL3 is found" ON)
option(EMPIRES_FETCH_NUKED_OPL3 "Download Nuked-OPL3 with FetchContent" ON)

if(MSVC)
  add_compile_options(/W4 /wd4244 /wd4245 /wd4310 /utf-8)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
  add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

if(EMPIRES_BUILD_TESTS)
  enable_testing()
endif()

# --- SDL3 dependency resolution -------------------------------------------
# Prefer a system/vcpkg-provided SDL3; fall back to FetchContent when
# EMPIRES_FETCH_SDL3 is ON.  Only built when EMPIRES_BUILD_APP is ON.
if(EMPIRES_BUILD_APP)
  find_package(SDL3 CONFIG QUIET)
  if(NOT SDL3_FOUND AND EMPIRES_FETCH_SDL3)
    include(FetchContent)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
      SDL3
      GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
      GIT_TAG        release-3.4.16
      GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(SDL3)
  elseif(NOT SDL3_FOUND)
    message(FATAL_ERROR "SDL3 not found and EMPIRES_FETCH_SDL3 is OFF")
  endif()
endif()

# --- Nuked-OPL3 dependency resolution --------------------------------------
# portable/audio/opl_backend.c (the only file allowed to include its
# headers, docs/portable/architecture.md "Layering") always needs this,
# regardless of EMPIRES_BUILD_APP: the audio unit tests link it too.
# Pinned to one commit (not a tag/branch) for reproducibility.
if(EMPIRES_FETCH_NUKED_OPL3)
  include(FetchContent)
  FetchContent_Declare(
    nuked_opl3
    GIT_REPOSITORY https://github.com/nukeykt/Nuked-OPL3.git
    GIT_TAG        765ec962e473aeb767e4cba74ffdc8f588ffbfe8)
  FetchContent_MakeAvailable(nuked_opl3)

  # Nuked-OPL3 ships plain opl3.c/opl3.h with no build system of its own;
  # build it ourselves as a small static library.
  add_library(nuked_opl3 STATIC ${nuked_opl3_SOURCE_DIR}/opl3.c)
  target_include_directories(nuked_opl3 PUBLIC ${nuked_opl3_SOURCE_DIR})
  # Third-party code: silence its warnings rather than fixing/suppressing
  # them individually (top-level /W4 or -Wall/-Wextra is for OUR code).
  if(MSVC)
    target_compile_options(nuked_opl3 PRIVATE /W0)
  else()
    target_compile_options(nuked_opl3 PRIVATE -w)
  endif()
else()
  message(FATAL_ERROR
    "EMPIRES_FETCH_NUKED_OPL3 is OFF: vendor Nuked-OPL3 (opl3.c/opl3.h, "
    "commit 765ec962e473aeb767e4cba74ffdc8f588ffbfe8) under "
    "third_party/nuked-opl3/ and add a nuked_opl3 static library target "
    "for it before add_subdirectory(portable) below.")
endif()

add_subdirectory(portable)
