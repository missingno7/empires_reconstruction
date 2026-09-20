# F_880A symbolic TASM trial

`F_880A` remains the canonical 506-byte matching-C asm-db capsule. A symbolic
TASM trial was deliberately not retained because its generated `_TEXT` extent
was 504 bytes, rather than 506.

The trial recovered the routine's structured control flow, its two local
jump-table regions, and all eleven direct near-call targets. Replacing those
calls with reconstructed external publics produced ordinary relative OMF
fixups, which Turbo Link can resolve, but it is not sufficient proof of an
exact replacement.

Two source-emission constraints were observed with TASM 1.0:

- a label-led data declaration after the odd-address `jmp word ptr cs:[bx+14h]`
  gained a code-segment alignment NOP; explicitly restoring the original odd
  offset with `ORG` changed the object extent again;
- the assembler selected short jumps where the original Turbo C emission used
  a near jump, and its documented `near ptr` spelling did not preserve that
  encoding in this context.

A later candidate must control these instruction forms and tables while
producing exactly 506 `_TEXT` bytes before it can replace `src/F_880A.C`.
The discarded trial did not change the canonical build or its proof status.
