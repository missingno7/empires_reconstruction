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
The two remaining topology adapters are candidate DATA/code interleaving and
arithmetic FIXUPP ordering. Historical translation-unit ownership remains a separate recovery frontier. See the machine-readable
[audit](fixture-dependency-audit.json).

All standalone TASM sources now emit the required empty DGROUP topology, so
the metadata adapter has been eliminated entirely: active rewrites **24 → 0**
while retaining exact output.
