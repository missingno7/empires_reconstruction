# Shared-module interface constraints

This ledger records compiler evidence that prevents independently matching C
fragments from being compiled as one translation unit. Each item is a source
interface recovery task, not evidence of an original module boundary.

## F_9D79 through F_A525

The contiguous 19-owner compact-model run was compiled in source order on
2026-09-19. Turbo C 2.0 rejected the combined input with 18 declaration
errors. The first mechanically relevant conflicts are:

- `F_9DCC` formerly declared `f9d79`, `f9d8e`, and `f039f` as returning
  `int`; their recovered definitions return `void` and their return values are
  not consumed. On 2026-09-19 these three declarations were normalized and a
  fresh F_9DCC object still matched all 247 bytes and 30 fixups; the complete
  structural link remained byte-identical.
- `F_A004` returns a far character pointer while `F_A036` declares `fa004`
  with a near pointer type.
- several fragments describe the DS:C470 table with incompatible temporary
  record declarations. A shared 27-byte `c470_record` layout is now recovered
  in [C470.H](../include/C470.H) and used by the compatible fragments; see the
  [record evidence](c470-record.md). F_A33F retains a byte-exact matrix view
  pending a compiler-equivalent typed expression.

The compiler also reports unresolved type conflicts for `f01ce`, `f03a8`, and
the C470 declarations. These diagnostics establish the immediate work order:
recover a shared declaration for the C470 record family and normalize only
call signatures whose individual-object bytes remain equal. No source was
changed by this experiment, and the failed candidate is intentionally not a
linker input.

Reproduce the evidence by concatenating the listed owners with the ordinary
`tools/probe_module_group.py` workflow after preparing an explicit candidate
recipe. A future successful probe must still verify public order, all relative
offsets, `_TEXT`, DATA/BSS, and fixups before entering the exact-link chain.
