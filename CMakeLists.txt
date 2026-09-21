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

add_subdirectory(portable)
