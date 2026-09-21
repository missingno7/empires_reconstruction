# Supervisor workflow

Read `status.json`, `blockers.json`, and `interface-conflicts.json`. Work on one
recurring failure class, encode the resolution in the tools/rules and tests, then
refresh the queue. Do not turn supervision into manual recovery of every function.

Runtime roots come from exact public offsets; recursive traversal adds direct
branches, loops, calls and fallthrough, stopping at terminal/indirect jumps.
Unreachable gaps remain unknown. Add table extents or indirect target sets to
`recipes/runtime/oracle.json` only with explicit evidence and bounds. Their
classification affects queue safety. The public index is derived from real final
objects and pinned library modules and checked against fresh objects at acceptance.

`layout/production-plan.json` is the explicit consumed plan: ordered objects,
source members, tools, flags, publics, initialized-DATA and BSS contributions.
Regenerate it with `python tools/production_plan.py` after an authorized structural
change. The builder rejects a stale plan, compiles final shared modules directly,
emits source DATA/BSS and invokes TLINK once. Historical probes remain available
for isolated evidence; no production stage chains through their reports.

Research builds: `python tools/build_exe.py --research`. They retain exact hash
and relocation checks but are explicitly not fresh ACCEPTANCE. Normal
`python tools/build_exe.py verify` is uncached, even if caches already exist.
The construction-only `--no-verify` path still checks the pinned full EXE hash and
ordered relocation metadata and never opens the original fixture.

ABI/layout suggestions are syntactic evidence, not automatic rewrite instructions.
The census records return/argument types, pointer shape, definitions/declarations,
storage bindings, aliases, record field offsets and parser confidence. It distinguishes
exact field-layout matches from merely equal 27-byte sizes. Test a proposed
consolidation with `python tools/probe_module.py` before adopting a common header,
and run full acceptance afterwards.

Known limitations are deliberate: indirect target inference is conservative;
semantic names are not guessed; unsupported C declarators remain unknown; runtime
cards currently accept the established zero-fixup contract. Broader OMF changes
need supervisor work and a new validated contract. There is no generic decompiler.
