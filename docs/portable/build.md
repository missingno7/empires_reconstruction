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

The executable lands at
`build-portable/portable/platform/sdl3/<Config>/empires.exe` (with
`SDL3.dll` beside it).  It needs the original game data files `AE000.DAT`
and `AE001.DAT`: put them next to the executable, or in `assets/` when
running from the repository root, or pass `--assets DIR`.  Save slots are
written as small overlay files (`AE000_061.rec` ...) into `--saves DIR`
(default: the asset directory); the archives themselves are never modified.

Historical switches pass through (`-V` VGA is the default; `-M`, `-E`,
`-T`, `-C` select the other display paths, `-I`/`-SI`/`-SA`/`-ST` the sound
backend).  Bring-up options: `--demo` (primitive test scene), `--selftest`
/ `--selftest-ms N` (exit after N ms), `--dump-vram FILE`
(`--dump-interval MS`) to write the presented frame(s) as PPM
(`python tools/portable/ppm2png.py in.ppm out.png`), `--script "..."` to
inject keys (see `portable/platform/sdl3/main.c`), `--deterministic` to
run on virtual time (reproducible replays), `--volume PCT` master output
volume (0..200, default 100).  Environment: `EMPIRES_TRACE=1` prints
high-level flow markers, `EMPIRES_NOSOUND=1` disables the sound state
machine, `EMPIRES_VOLUME=PCT` is the same as `--volume`.

## Tests

```
ctest --test-dir build-portable -C Debug
```

Unit tests cover the resource decoders (all 220 archive records against a
SHA-256 manifest), the software graphics drivers (209 cases captured from
the real 8086 code under MS-DOS Player), the timer, keyboard, CC.LIB
compat, the ported assembler modules and the sound state machine.
`replay_*` tests run the whole game headlessly (SDL dummy drivers) with a
scripted input on virtual time and compare frame hashes
(`portable/tests/replay/*.json`; refresh with
`python tools/portable/replay_test.py <exe> <manifest> --update` after
reviewing the frames).  Tests that need the original assets skip when they
are absent.
