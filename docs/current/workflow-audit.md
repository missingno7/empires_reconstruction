# Production-line audit

Priorities, based on the current sources, tests and recent runtime commits:

1. **Remove the compiler from runtime transcription.** Recent commits deliberately
   hide SI/DI operations in DB to prevent Turbo C saves. Preserve the C source as
   a frozen oracle; assemble the working runtime directly with pinned TASM.
2. **Replace staged linking.** `build_exe.py` still links a sizing baseline, a
   scaffold, source DATA and three shared replacements. Generate the complete
   module plan first, compile final modules once, emit DATA/BSS, link once.
3. **Bound runtime work mechanically.** The old analyzer only follows 20 veneer
   jumps. Recursive decoding must stop at unknown indirect edges and distinguish
   unreachable bytes from proven tables. Resolve publics before owner interiors.
4. **Make progress and failures local.** Owner-wide DB classification conceals
   hundreds of recovered instructions. Measure source emission by byte, verify
   a runtime edit with one assembler invocation, and reject edits outside its card.
5. **Preserve lessons.** Near jump selection, EXTRN scope, DGROUP and descending
   ORG/FIXUPP ordering recur across symbolic-ASM evidence. Collect rules and
   exercise actual pinned tools in focused tests.
6. **Provide a single current queue.** Historical checkpoints contain obsolete
   adapter/blocker claims. Keep evidence, but direct grinders only to generated
   current reports with input fingerprints and explicit supervision classes.
7. **Prepare semantic cleanup.** Inventory declarations and local record layouts;
   flag conflicts with source locations and parser confidence instead of guessing
   that a matching name implies a compatible near/far type.

Existing `docs/data-interleaving.json` was modified before this task and is not
an input to the new production plan. Historical probes remain evidence tools.
