# Compact-model startup object evidence

The local Turbo C 2.0 installation contains `C0C.OBJ`, pinned by hash in
`layout/toolchain.json` and copied only into the ignored local `toolchain/`
directory by `setup_toolchain.py`. The object declares a 444-byte `_TEXT`
segment, byte alignment, public combination, class `CODE`, and a `DGROUP`
group containing the startup data, BSS and stack members.

`python tools/audit_startup_object.py` compares that object with the original
load image. Every byte outside the object’s own FIXUPP locations matches the
first 444 load-image bytes exactly. This establishes strong module evidence:
the current fixed manifest’s split from load offset `0` through `0x1BC` is the
complete compact-model startup contribution, even though the current fixed
ownership still divides that extent into smaller source proofs.

The comparison does not resolve `_main`, `_exit`, DGROUP or other linker
targets. Those remain inputs to the relocatable TLINK experiment.
