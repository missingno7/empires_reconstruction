# Fixture-free structural EXE construction

`python tools/build_exe.py --no-verify` now constructs `build/AEPROG.EXE`
without opening `assets/AEPROG.EXE`. It still freshly compiles the recovered
C/ASM sources, emits source DATA and `GAMEBSS.ASM`, applies the explicitly
tracked OMF adapters, and invokes Turbo Link 2.0.

The temporary baseline `DGSCF.OBJ` no longer copies initialized bytes from the
fixture. Its required dimensions come from `layout/mz-header.json` and the
canonical manifest; its discarded initialized portion is zero-filled before
the source-DATA stage replaces the whole contribution.

The structured MZ header now supplies the canonical ordered relocation list
for construction-time group and interleaving checks. The original executable
is reserved for `python tools/build_exe.py verify`, which independently checks
the MZ fields, relocation order, load image, and complete file identity.

This removes one construction adapter: EXE-fixture dependency **1 -> 0**.
The four remaining topology adapters are unchanged: candidate DATA/code
interleaving, arithmetic FIXUPP ordering, aggregate BSS ownership, and empty
Turbo-C-compatible DGROUP metadata. See the machine-readable
[audit](fixture-dependency-audit.json).
