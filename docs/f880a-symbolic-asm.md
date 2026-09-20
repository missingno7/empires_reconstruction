# F_880A symbolic TASM recovery

`F_880A` is a 506-byte state-driven record-selection loop recovered as
symbolic TASM. Its fresh object exactly binds to load range `0x880A..0x8C04`;
the normal structural Turbo Link 2.0 build remains byte-identical to the
original executable.

The recovered source exposes eleven direct near calls as ordinary external
references to their reconstructed public owners. Its two local jump tables
use named labels and symbolic `dw` entries. The terminal continuation into
`F_8C04` remains a local `f880a_end` boundary label, avoiding an artificial
external short-jump relationship.

TASM 1.0 emits a short-jump-plus-NOP sequence for this routine's historical
near jumps. The narrowly scoped `JMP_NEAR` macro therefore emits the original
near-jump opcode while keeping its displacement assembler-resolved from the
symbolic target. This is an encoding constraint, not a byte capsule. The
`ORG 0F5h` before `state_keys` likewise preserves the observed odd-address
table placement without an assembler-inserted alignment byte.

The recovery also preserves the four-entry mode table and the small branch
shims around the zero-count and decrement paths. Those compiler artifacts are
necessary for exact `_TEXT`, object fixups, and the final executable layout.
