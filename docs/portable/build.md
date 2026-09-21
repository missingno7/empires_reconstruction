# Building the portable SDL3 port

This covers the `portable/` tree only (the SDL3 executable, its core
library, and the ctest unit tests).  The historical Turbo C / TASM build
under `src/`, `asm/`, `include/`, `tools/build_exe.py` is a **separate**,
unrelated toolchain — you do not need DOS, Turbo C, TASM, or MS-DOS Player
to build or run the portable port.

## Prerequisites

- CMake >= 3.24
- MSVC 2022 or newer (Visual Studio "Desktop development with C++" workload)
- git (required so CMake's `FetchContent` can clone SDL3 when no system
  SDL3 package is found)

No other dependencies are required for Milestone A. SDL3 is resolved
automatically (see below); later milestones add Nuked-OPL3 the same way.

## Configure

```
cmake -S . -B build-portable -G "Visual Studio 17 2022" -A x64
```

(Any recent Visual Studio generator works, e.g. `"Visual Studio 18 2026"` if
installed; the important part is `-A x64`.)

Relevant options (all CMake cache variables, `-D<NAME>=<VALUE>` to override):

- `EMPIRES_BUILD_APP` (default `ON`) — build the `empires` SDL3 executable.
- `EMPIRES_BUILD_TESTS` (default `ON`) — build and register the ctest unit
  tests.
- `EMPIRES_FETCH_SDL3` (default `ON`) — when no system/vcpkg SDL3 package is
  found via `find_package(SDL3 CONFIG)`, fetch and build SDL3 from source
  with `FetchContent`. Pinned to tag `release-3.4.16` of
  https://github.com/libsdl-org/SDL.git. Set to `OFF` to require a
  preinstalled SDL3 instead.

The first configure with `EMPIRES_FETCH_SDL3=ON` and no system SDL3 clones
and configures the SDL3 source tree as part of the CMake configure step;
expect this to take a few minutes on a fresh clone. Subsequent configures
reuse the already-fetched source.

SDL3 is built as a **shared** library (`SDL_SHARED ON` / `SDL_STATIC OFF`
in the fetched build). A post-build step copies `SDL3.dll` next to
`empires.exe` so the executable runs without installing SDL3 system-wide.

## Build

```
cmake --build build-portable --config Debug
```

Use `--config Release` for a release build once the generator has been
configured (multi-config generators like Visual Studio build whichever
config you pass here; no reconfigure needed to switch).

## Run

The executable lands at:

```
build-portable/portable/platform/sdl3/Debug/empires.exe
```

(`Release/empires.exe` for a Release build.) Running it with no arguments
opens a resizable window titled "Empires (portable)", integer-scaled from a
320x200 logical framebuffer, showing a test gradient. Close the window,
press Escape, or send SDL_EVENT_QUIT to exit.

`--selftest` runs the loop for about 300 ms and then exits with status 0,
with no human interaction required — this is what CI and automated checks
should invoke:

```
build-portable/portable/platform/sdl3/Debug/empires.exe --selftest
```

## Tests

```
ctest --test-dir build-portable -C Debug
```

Tests that need `assets/AE000.DAT`/`AE001.DAT` (not committed to the repo)
skip cleanly (exit code 77) when those files are absent.
