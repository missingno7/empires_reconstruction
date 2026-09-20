# Gameplay workspace BSS layout

The 52-byte `GAMEPLAY_STATE_BSS` contribution is now fully represented by
three named integer fields and twelve far-pointer workspace slots.

`F_B593` calls `farfree` on the nine historically exposed workspace pointers
(`_gc580`, `_gc584`, `_gc58a`, `_gc58e`, `_gc59a`, `_gc59e`, `_gc5a4`,
`_gc5a8`, and `_gc5ac`). The two unanchored pointer slots between `_gc58e` and
`_gc59a` are named `gc590_workspace` and `gc594_workspace` solely from their
verified four-byte layout. `F_A33F`, `F_B772`, `F_D089`, and `F_CE68` prove the
four public scalar roles at `_gc57e`, `_gc588`, `_gc5a2`, and `_gc5b0`.

The names are reconstruction names. The layout preserves every original BSS
public and has no claim to original translation-unit ownership.
