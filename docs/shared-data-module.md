# Shared compilation with exact DATA

The DATA audit suggested testing F_75F3 through F_778B together: their three
initializer owners are contiguous, have lengths 15, 13 and 15, and separate
word-aligned objects insert padding that the original does not contain.

The five-function recipe C_75F3_7856 now compiles into one fresh Turbo C object
with exactly 611 TEXT bytes, the correct ordered publics, 53 checked code
fixups, 43 exact DATA bytes and no BSS. The shared DATA comparison uses a common
base only after proving complete contiguous ownership and byte identity.
The fresh compiler supplies the initializer offsets within that segment.

The recovered F_7747 declaration of f778b conflicted with its definition when
compiled together. Changing that declaration from void to int resolves the
conflict and leaves the independently compiled executable bytes unchanged.

`python tools/probe_shared_module_link.py` replaces the five objects in a copy
of the full staged link with this fresh object. The [receipt](shared-data-module-link.json)
shows no linker errors, the full TEXT still at 00000..0FA22, and every downstream
code contribution unchanged. Five separate objects become one actual compiler
object; no function placement addresses are written into it. Temporary public
aliases and the original full-link DGROUP scaffold are retained.

Two bytes of inter-object DATA alignment disappear. The retained scaffold was
sized for separate compilation, so initialized DATA and the BSS start are now
two bytes short; the first load mismatch is 0x81. BSSEND happens to agree with
the original, while the BSS start does not. This does not prove BSS recovery.
No compensating padding was inserted and the baseline path remains available.

This is a compatible shared-compilation group with exact DATA and a tested
structural linker input. It is stronger evidence than code-only concatenation,
but does not prove the historical translation-unit boundaries. The known
four-function C_6C26_6C87 experiment still passes. Full fixed reconstruction
still reproduces all 690,588 bytes of the three original game files.
