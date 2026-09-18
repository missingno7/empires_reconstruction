# Empires Reconstruction

## Vision

`empires_reconstruction` is an attempt to reconstruct the original DOS version of **Super Solvers: Challenge of the Ancient Empires!** into a complete, human-readable, lossless source representation.

The goal is not to create a modern source port.

The goal is not to replace DOS, emulate the game differently, or rewrite the game into clean C++.

The goal is to take the original game apart, understand what every part of it is, give those parts names and meaning, and still be able to put them back together into the original game.

That includes not only executable code, but also:

* embedded data,
* fonts,
* graphics,
* sprites,
* lookup tables,
* scripts,
* level data,
* resource indexes,
* compression formats,
* runtime-generated structures,
* external game data files,
* compiler/runtime support code,
* executable metadata and relocation information.

Conceptually:

```text
original game
    AEPROG.EXE
    AE000.DAT
    AE001.DAT
    ...
        ↓
lossless decomposition
        ↓
code / data / assets / tables / resources / runtime
        ↓
functions / globals / structures / formats / subsystems
        ↓
names + documentation + semantic understanding
        ↓
matching source + reconstructed assets + exact fallback
        ↓
deterministic rebuild
        ↓
original game
```

The original files remain the ground truth.

## Architectural end-state: layout must emerge

The fixed-layout builder is a bootstrap scaffold and comparison oracle. It is
not the intended final build system. This requirement, clarified by the project
owner on 2026-09-18, governs the intermediate examples below.

The final build must compile, assemble, encode and compress independent source
components, then link and pack them. Historical addresses and resource offsets
must emerge from module/resource order, emitted sizes, segment organization,
alignment, library inclusion and recovered linker/packer behavior. They must
not remain externally imposed original placements.

Original EXE/DAT files eventually serve only as verification fixtures. Unknown
byte ranges and opaque resource copies are temporary fallbacks, even when
stored in separate local files. An understood binary format may intentionally
remain a canonical component; that requires established format/ownership
evidence and is distinct from relabeling an unknown blob.

Track four levels separately: **placement reconstruction**, **independent
component reconstruction**, **structural build reconstruction**, and
**emergent whole-build reconstruction**. Byte equality alone does not establish
the final level. Retain the fixed builder while independently proving:

```text
fixed reconstruction == component-driven link/pack == original fixtures
```

The final target is the original **buildable software system**, including its
code, data, assets, modules, compression, libraries, linking and packing rules.
See [build-reconstruction.md](build-reconstruction.md) for the contract and
current evidence boundaries.

---

# North Star

The long-term goal is to reach a point where the original game is no longer an opaque binary artifact.

Instead, the repository should represent the game as a complete understandable source project.

Ideally, someone should be able to inspect the repository and answer questions such as:

* Where is the main game loop?
* How are game objects represented?
* How does collision detection work?
* How are levels stored?
* Which functions update enemies?
* Where is the keyboard interrupt handler?
* How is the HUD drawn?
* What does a particular global variable mean?
* Where is the game font?
* How are glyphs encoded?
* Which sprites are embedded in the executable?
* What compression format is used?
* Which bytes belong to compiler runtime code?
* Which tables drive object behaviour?
* Which data is loaded from `AE000.DAT` and `AE001.DAT`?
* How is a particular resource transformed from its human-readable representation into the bytes used by the original game?

And after understanding all of this, the project should still be able to reconstruct the original program.

The ideal end state is:

```text
human-readable reconstructed project
                ↓
             build
                ↓
        original game files
```

with exact matching wherever technically achievable.

---

# Core idea

We want to progressively transform opaque binaries into something resembling a real source tree without losing information.

At the beginning, a region may simply be:

```text
bytes 0x12340–0x12700
unknown
```

Later it may become:

```asm
F_12340:
    push bp
    mov bp, sp
    ...
```

Then:

```asm
UpdateActiveObjects:
    ...
```

Later we may understand its inputs, outputs and data structures.

Eventually it might even become matching C:

```c
void UpdateActiveObjects(void)
{
    ...
}
```

At every stage, the generated machine representation must continue to match the original program.

Understanding improves.

The original artifact does not change.

---

# What makes this different from a source port

A traditional source port asks:

> How can we recreate the behaviour of this game on a modern platform?

This project asks:

> What exactly is this original program?

A source port is free to change implementation while preserving behaviour.

A reconstruction is not.

For example, these may be equivalent from a gameplay perspective:

```c
x += 2;
```

and:

```asm
inc ax
inc ax
```

But if the original executable contains the latter, a lossless reconstruction cares about that distinction.

The original machine code, layout, data and relationships are evidence.

We preserve them.

---

# What makes this different from a normal disassembly

A raw disassembly gives us instructions.

That is only the first layer.

This project should gradually recover:

* function boundaries,
* meaningful function names,
* global variables,
* data structures,
* field meanings,
* tables,
* dispatch systems,
* asset formats,
* resource formats,
* calling conventions,
* memory ownership,
* subsystem boundaries,
* relationships between code and data,
* relationships between code and assets,
* high-level architecture,
* and eventually much of the original program's intent.

The desired result is not:

```asm
F_17A42:
    mov ax, [bx+13h]
```

but something closer to:

```asm
UpdateObjectCollision:
    mov ax, [bx + GameObject.flags]
```

with documentation explaining what the flags mean, who calls the function and what state it changes.

---

# Losslessness

The fundamental invariant of the project is:

> Semantic improvement must never require throwing away machine truth.

Every byte of the original game files should eventually have an owner.

An owned region may temporarily be represented as:

* raw original bytes,
* exact data,
* mechanical disassembly,
* structured ASM,
* documented ASM,
* matching ASM,
* matching C,
* reconstructed assets,
* known compiler/runtime library code,
* known padding or alignment,
* executable metadata,
* relocation information.

Unknown content is not a failure.

Unknown content simply represents the current frontier.

For code, progression may look like:

```text
RAW
 ↓
DISASSEMBLED
 ↓
STRUCTURED
 ↓
NAMED
 ↓
DOCUMENTED
 ↓
MATCHING_ASM
 ↓
MATCHING_C
```

Not every region needs to end in C.

A perfectly understood interrupt handler may remain ASM forever.

---

# Assets are part of the reconstruction

The game contains more than executable instructions.

Fonts, graphics, sprites, lookup tables, level data and other resources are part of the original artifact and should be reconstructed with the same care as code.

Initially, an embedded asset may simply be represented as:

```text
assets/raw/font_18420.bin
```

This is completely valid.

Later we may discover that the region is:

```text
256 glyphs
8×8 pixels
1bpp
custom ordering
```

At that point the repository may contain a better source representation:

```text
assets/fonts/main_font.png
assets/fonts/main_font.toml
```

with an encoder:

```text
main_font.png
      ↓
font encoder
      ↓
original packed DOS representation
      ↓
EXACT MATCH
```

The important requirement is not that assets must remain stored in their original binary format.

The requirement is:

> Every reconstructed asset must have a lossless and deterministic path back to the exact original representation.

---

# Human-readable asset sources

Where useful, the canonical representation in the repository should be the most understandable lossless source form.

Examples may include:

```text
original embedded bitmap
        ↓
PNG + layout metadata
```

```text
packed font bytes
        ↓
font.png + character mapping
```

```text
raw lookup table
        ↓
named ASM/C structure
```

```text
opaque resource block
        ↓
documented binary format + encoder
```

```text
level blob
        ↓
structured level description
```

The build system then converts those representations back into the exact historical bytes.

For example:

```text
assets/sprites/player.png
        ↓
ega_planar_encode
        ↓
compression
        ↓
player.bin
        ↓
    linked through recovered module/segment rules
```

and:

```text
generated player.bin
==
original player bytes
```

---

# Preserve historical quirks

A human-readable reconstruction must not silently clean up historical artifacts.

The original resource may contain:

* unused glyphs,
* duplicate sprites,
* redundant tiles,
* odd padding,
* garbage bytes,
* unusual ordering,
* unnecessary table entries,
* non-optimal compression,
* unused fields.

These are still part of the original artifact.

If a reconstructed source format is used, its encoder must preserve whatever is necessary to reproduce those bytes.

For example:

```text
font.png
font_layout.toml

unused_glyph = 137
duplicate_glyph = 211
trailing_padding = "00 FF 00"
```

may be preferable to silently removing those quirks.

The same principle applies to compiler-generated code.

Ugly or redundant original behaviour is evidence, not something to automatically fix.

---

# Asset recovery progression

Assets can have their own reconstruction depth.

For example:

```text
RAW
 ↓
IDENTIFIED
 ↓
FORMAT UNDERSTOOD
 ↓
DECODED
 ↓
REBUILDABLE
 ↓
DOCUMENTED
```

Example:

```text
RAW
    region_18420.bin

IDENTIFIED
    main game font

FORMAT UNDERSTOOD
    8×8 1bpp glyphs

DECODED
    main_font.png

REBUILDABLE
    PNG → encoder → exact original bytes

DOCUMENTED
    glyph ordering, character mapping, loader and usage known
```

This progression is just as important as code recovery.

---

# Matching is the authority

Reverse engineering involves interpretation.

Names can be wrong.

Types can be wrong.

Comments can be wrong.

Asset interpretations can be wrong.

Our assumptions about the original architecture can be wrong.

Matching provides a hard constraint.

For a recovered code region:

```text
source
  ↓
original-compatible compiler / assembler
  ↓
machine code
  ↓
comparison
  ↓
EQUAL
```

For a recovered asset:

```text
human-readable asset
  ↓
encoder / packer / compressor
  ↓
historical binary representation
  ↓
comparison
  ↓
EQUAL
```

This lets us make the repository increasingly readable while continuously proving that the underlying artifact has not changed.

For example, we may start with:

```c
struct rec {
    char pad[17];
    int f11;
    int f13;
};

extern struct rec gc470[];
```

and eventually recover:

```c
struct GameObject {
    char unknown_00[17];
    int animation_state;
    int collision_flags;
};

extern struct GameObject objects[];
```

If the relevant matching regions still compile identically, we have gained semantic information without losing binary fidelity.

The same applies to data and assets.

---

# PortForge as an observation instrument

PortForge is useful to this project, but it is not part of the reconstructed game.

The VM acts as a microscope.

It can run the original program and tell us what it actually does.

For example:

```text
Function F_104B4

observed calls: 36

entry:
    DS:SI -> source
    ES:DI -> destination
    CX = width
    DX = height

observed writes:
    destination buffer

called by:
    room drawing
    scrolling
    background restoration
```

That evidence may lead us to rename:

```text
F_104B4
```

to:

```text
CopyMapRegion
```

Nothing in the original executable needs to change.

The VM helps discover meaning.

Matching proves preservation.

---

# PortForge can also help identify assets and data

Dynamic execution is useful not only for code.

It can reveal:

* which executable regions are read as graphics,
* which blocks are copied into video memory,
* which tables are interpreted as structures,
* which external file offsets correspond to levels,
* which memory regions hold decoded assets,
* which routines decompress data,
* which resources are referenced together.

For example:

```text
EXE region 0x18420
        ↓ read by DrawCharacter
        ↓ copied as 8-byte chunks
        ↓ indexed by ASCII-like character id
```

may provide strong evidence that the region is a font.

The same replay and snapshot infrastructure can therefore help recover both code semantics and data formats.

---

# Replays and snapshots

Existing PortForge replay and snapshot infrastructure can provide dynamic evidence.

A replay corpus may contain scenarios such as:

```text
boot_to_menu
start_new_game
enter_first_room
walk
jump
enemy_collision
pickup_item
death
level_transition
```

During these scenarios we can record:

* which functions execute,
* their callers,
* their callees,
* register values,
* memory reads,
* memory writes,
* indirect call targets,
* branches taken,
* changes to globals and structures,
* resource reads,
* decompression operations,
* transfers into video/audio memory.

Snapshots can make individual functions directly reproducible.

Instead of repeatedly starting the game, we can:

```text
restore real machine state
        ↓
execute target function
        ↓
observe behaviour
```

This turns dynamic reverse engineering into repeatable evidence rather than one-off debugging.

---

# Static and dynamic evidence should meet

The most useful knowledge appears when static and dynamic analysis reinforce each other.

For example:

```text
Object field +0x13
```

Static analysis may show:

```text
written by SpawnObject
tested by CollisionCheck
cleared by DestroyObject
```

Dynamic analysis may show:

```text
bit 2 changes when object becomes solid
bit 5 changes when object is destroyed
```

The reconstruction can then represent the field as something meaningful while preserving uncertainty where needed.

For example:

```c
uint16_t object_flags;
```

with documented bit evidence.

The same principle applies to assets.

Static analysis may show:

```text
table referenced by DrawGlyph
stride = 8 bytes
256 entries
```

Dynamic analysis may show:

```text
entry 65 used when drawing "A"
```

Together this can justify reconstructing the block as a font and assigning a character mapping.

We should prefer explicit uncertainty over invented certainty.

---

# The reconstructed source tree

The project should gradually resemble a plausible source project of the original game.

For example:

```text
src/
    startup/

    game/
        main_loop.asm
        state.asm

    player/
        movement.asm
        collision.asm

    objects/
        object_manager.asm
        enemies.asm
        projectiles.asm

    world/
        levels.asm
        rooms.asm
        map.asm

    graphics/
        ega.asm
        sprites.asm
        hud.asm

    input/
        keyboard.asm

    audio/
        sound.asm

assets/
    fonts/
        main_font.png
        main_font.toml

    sprites/
        player.png
        enemies.png

    ui/
        hud.png

data/
    levels/
    strings/
    object_tables/
    lookup_tables/

formats/
    level_format.md
    sprite_format.md
    font_format.md

memory/
    globals.inc
    structs.inc
    segments.inc

tools/
    font_encoder/
    sprite_encoder/
    compression/
    executable_builder/

docs/
    architecture.md
    game-loop.md
    object-system.md
    rendering.md
    asset-pipeline.md
    memory-map.md
```

This structure does not need to match the exact filenames or source modules originally used by the game's developers.

We are reconstructing the program's meaning, not pretending to possess the original source tree.

---

# Generated artifacts should not become source truth

Where a higher-level lossless source exists, generated binary files should generally be treated as build artifacts.

For example:

```text
assets/fonts/main_font.png
```

is source.

```text
build/assets/main_font.bin
```

is generated.

Likewise:

```text
assets/sprites/player.png
```

may eventually replace:

```text
raw/player_sprite.bin
```

once the encoding pipeline is proven exact.

Raw original bytes remain valuable as verification fixtures and archaeological evidence, but they do not need to remain the canonical editable representation forever.

---

# Reconstruction depth

Progress should not be represented by one number.

Several independent dimensions matter.

For example:

```text
Executable accounted for          100%
External data accounted for       100%

Code/data classified               96%
Reachable code identified          91%
Functions named                    63%
Functions documented               42%
Important structures recovered     35%

Embedded assets identified         68%
Asset formats understood           41%
Assets losslessly decoded          28%
Assets rebuildable from source     21%

Matching C                         18%
Remaining unknown bytes             4%
```

A region can be byte-perfect but semantically unknown.

Another region can be deeply understood but intentionally remain ASM.

An asset can be identified but not yet decoded.

These are different dimensions.

---

# Whole-game reconstruction

The long-term goal is not merely to reconstruct code inside `AEPROG.EXE`.

The complete game should eventually be described.

That includes:

```text
AEPROG.EXE
AE000.DAT
AE001.DAT
other required original game files
```

The repository should progressively explain what each file contains and how each meaningful region is used.

Ideally:

```text
build
 ↓
AEPROG.EXE
AE000.DAT
AE001.DAT
...
 ↓
hash / structural comparison
 ↓
identical to original
```

Where no higher-level representation has yet been recovered, exact binary
fallback is acceptable during reconstruction. Unknown fallback is not an
acceptable final substitute for independently reconstructed components.

---

# Whole-program reconstruction

For DOS, rebuilding `AEPROG.EXE` may require reconstructing more than code:

* MZ header,
* relocation table,
* segment layout,
* alignment,
* padding,
* startup/runtime code,
* compiler-generated support routines,
* code and data ordering,
* linker behaviour,
* embedded assets.

Where historically exact build metadata is unknown, the project may initially reconstruct the executable through explicit original layout placement.

This is acceptable as the bootstrap scaffold. The priority is losslessness
throughout the transition. Recovering compatible modules and linker behavior
is required for the final build to derive the original layout; recovering the
historical developers' exact filenames is not required.

---

# Existing matching work

`empires_forged` already contains valuable recovered material.

In particular it contains regions proven as:

* `MATCHING_C`,
* `MATCHING_ASM`,
* known runtime/library code,
* and other correspondence information.

Those results should be treated as existing evidence, not redone from scratch.

The reconstruction project can absorb them into the complete executable representation.

This gives us a useful starting condition:

```text
known matching bricks
        +
exact fallback for everything else
        ↓
complete executable
```

Over time, raw fallback should shrink as more regions become understood source, structured data or reconstructed assets.

---

# Relationship to empires_forged

The two projects have different goals.

## empires_reconstruction

```text
preserve original machine
        ↓
understand original machine
        ↓
understand original data/assets
        ↓
produce matching source representation
```

## empires_forged

```text
understand original behaviour
        ↓
replace original machine
        ↓
produce native implementation
```

They complement each other.

A useful model is:

```text
                  original game
                       │
              empires_reconstruction
                       │
             understanding / formats
                       │
          contracts / types / assets
                       │
                  empires_forged
                       │
                  native game
```

The reconstruction can become the authoritative knowledge base for later native recovery.

---

# Why this is useful

The project has value even if no native port is ever produced.

## Preservation

It transforms an opaque historical executable and its associated game data into a reproducible and inspectable representation.

## Documentation

It can explain how the game actually works at a level much deeper than a wiki or gameplay description.

## Asset archaeology

It can recover historical fonts, graphics, level formats, resource encodings and build-time assumptions in a form that can be inspected and reproduced.

## Modding

Once structures, tables, assets and subsystems are known, modifications to the original game become precise and controlled.

## Source-port research

A native port can consume known contracts and asset formats instead of rediscovering behaviour while replacing it.

## Reverse-engineering research

The progression:

```text
binary
 → disassembly
 → structure
 → semantics
 → matching source
```

and:

```text
binary asset
 → identification
 → format recovery
 → decoded source
 → exact reconstruction
```

creates excellent material for testing automated and agentic reverse-engineering techniques.

---

# What this project is not

This project is not:

* a DOS emulator,
* a rewritten engine,
* a clean-room gameplay clone,
* a modern C++ remake,
* an attempt to immediately replace every ASM routine with C,
* an exercise in making code aesthetically modern,
* an asset remaster project,
* a reason to replace original graphics with approximations,
* a reason to discard awkward compiler-generated behaviour,
* a reason to discard odd historical resource encodings.

Ugly original behaviour is still part of the artifact.

If the game contains strange code, redundant instructions, unusual layouts, duplicate assets, compiler quirks or non-optimal packing, preserving and explaining them is preferable to silently "fixing" them.

---

# MVPs versus the North Star

Early milestones do not need to implement the entire vision.

For example, MVP1 may intentionally use:

```text
raw exact fallback
```

for essentially all unknown code and embedded assets.

That is acceptable.

The purpose of the vision is to define where the project should eventually go, not to force every capability into the first milestone.

The evolution may look roughly like:

```text
MVP1
    exact whole-program reconstruction skeleton
    + existing matching bricks

later
    structural decomposition

later
    semantic naming and types

later
    dynamic evidence integration

later
    data-format recovery

later
    asset decoding and exact re-encoding

North Star
    complete human-readable lossless reconstruction
```

Every milestone should preserve the ability to build on the previous one without discarding evidence.

---

# Guiding principle

The project should continuously move from:

```text
we know these bytes exist
```

toward:

```text
we know what these bytes mean
```

and, for assets:

```text
we know this binary block exists
```

toward:

```text
we know this is the game font,
we understand its format,
we can edit its source representation,
and we can reproduce its exact historical bytes
```

without ever losing the ability to prove:

```text
this is still the same original artifact
```

In one sentence:

> **Empires Reconstruction aims to make the original game completely legible without ceasing to be the original game.**

Or, operationally:

> **PortForge tells us what the program means. Matching proves we have not changed what the program is.**

And for assets:

> **Decode for humans, encode for history.**
