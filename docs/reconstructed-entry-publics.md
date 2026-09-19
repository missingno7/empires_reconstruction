# Reconstructed entry publics

Recovered callers now use the selected OMF public exported by each reconstructed
code owner. Sixty-four references across 19 callers were changed from temporary
semantic or numeric aliases to relocatable owner publics. This includes the
former `init_*`, `done_*`, `show`, `load`, `pane`, `msg`, single-letter helper,
and underscore/case variants.

The startup-facing owner `F_4A93` now defines and exports `main` / `_main`
directly. The large dispatch owner begins with `f039c` / `_f039c`, its actual
entry, rather than the scaffold name `_runtime_block`. Turbo C emits the same
machine bytes for both changes.

The linker probe also stopped interpreting a numeric-looking external name as
an address when the manifest already supplies a component-owned target. This
removes the false `_f53bf` internal label at code offset `0x53BF`; that spelling
already belongs to the reconstructed owner whose evidenced entry is `0x51BF`.

The active code-symbol adapter set consequently fell from 43 transformed
objects to one at this checkpoint:

```text
RUNTIME_BLOCK: 27 internal PUBDEF labels
all other reconstructed objects: no symbol transform
```

Those 27 labels name genuine entry points at offsets inside the 6,571-byte
runtime dispatch block. Canonical inline assembly now emits them naturally,
closing this final symbol adapter in the subsequent
[runtime-public checkpoint](natural-runtime-publics.md).

The fixed reconstruction and exact Turbo Link 2.0 structural link remain
byte-identical with zero unresolved symbols and no code-placement divergence.
