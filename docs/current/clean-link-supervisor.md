# Clean source/link supervisor result

The production build compiles 293 source modules and 41 BSS contributions,
then runs TLINK once with 442 inputs. All 293 compiler objects are preserved
byte-for-byte. RAND is packaged as a complete native library member. No
compiler TEXT trimming, DATA externalization, library payload patching, or
synthetic code-padding objects remain. The final EXE is copied directly from
TLINK output; the original EXE and relocation metadata are verification inputs.

## Corrections removed

* Code extents are checked, never truncated to historical lengths.
* F_56C6 and F_A658 reference shared record storage directly instead of emitting
  duplicate DATA that the build must remove.
* F_A28D and F_D5BA retain their native initialized DATA in their code objects.
* F_9D8E now owns its 209-byte dialog text and 20-byte descriptor together.
  The descriptor's odd offset is natural within the word-aligned contribution.
* C_75F3_7856 owns seven bytes of typed UI state before its 43 caption bytes:
  int gb80=4, char gb82=4, int gb83=0, int gb85=0. Existing consumers establish
  these widths. The last state byte was incorrectly categorized as padding.
  Three callers now use the same source symbol spellings as the definitions.
* Four ASM modules declare WORD-aligned TEXT. TLINK supplies the four former
  synthetic code-pad bytes naturally, without changes to instruction bytes.
* The complete RAND compiler object is inserted into the library unchanged.
  Library page references are recalculated; old object records/fixups no longer
  supply the wrapper for newly compiled segment bytes.

These are evidenced, exact reconstructed source groupings; they do not prove
historical filenames or original translation-unit boundaries.

## Acceptance evidence

`python -m unittest discover -s tests -p test_build_exe.py` checks exact EXE
bytes, 106 ordered relocations, raw-vs-staged compiler objects, native DATA
public offsets and sizes, and RAND membership. A separate test prohibits reads
of assets/AEPROG.EXE during construction. Trimming and DATA externalization
helpers are patched to fail if called. The build itself refuses any changed
compiler object and any initialized compiler DATA lacking complete ownership.

`test_native_data_plan.py` checks natural contribution order and alignment
ownership, including rejection cases. `test_native_library.py` verifies all
member bytes and dictionary targets when a member grows across library pages.
The current receipt records empty pre_link_corrections lists and 293 untouched
compiler objects. The grinder queue is separately checked by
`python tools/audit_grinder_queue.py`.

## Remaining historical layout dependencies

This is not a claim that every absolute address or historical layout recipe
has been eliminated:

* Runtime instruction operands and dispatch tables still contain historical
  CS addresses, and 3,867 runtime bytes remain raw (3,866 in 72 CHEAP cards;
  one unproven C3 byte remains SUPERVISOR).
* M_DDD9_DF98 still emits symbolic instructions using descending ORG directives
  to reproduce historical FIXUPP order. Natural source/toolchain ordering needs
  separate recovery before those directives can be removed safely.
* Generated DATA/global aliases and BSS still use component/public offset
  recipes. Typed declarations and original storage boundaries are incomplete.
* Module and DATA link order remains explicit reconstructed build input.
  Linker placement is now natural for all compiled contributions, but this
  does not establish the original source-file organization.
* Native RAND packaging targets the pinned Borland library and linker. Other
  pinned runtime/startup library modules have not been reimplemented from source.
* Graphics overlays are archive resources, not EXE compilation blockers. Their
  full boundary recovery is recorded separately; canonical archive ASM/checker
  integration remains required before issuing grinder cards.

The earlier odd-address DATA experiments and their failure evidence are retained
in native-data-blockers.json, now marked resolved by source ownership recovery.
Compiler -a/-a- probes changed structure packing, not DATA segment alignment;
no such flag was adopted and no SEGDEF rewriting was introduced.

## Runtime table address follow-up

Runtime tables now derive addresses from 183 instruction labels. The computed
entry tables use label differences; absolute CS entries use label minus runtime
base plus the loader ABI base 039Ch. This removes 178 absolute CS EQU constants
and the unused copied table's numeric +424h adjustments. It adds no fixups and
changes no emitted byte. The fixed loader ABI base remains explicit, and other
absolute runtime operands still require separate recovery.

A linear-order probe of M_DDD9_DF98 produces identical instruction bytes but a
different far-fixup sequence. arithmetic-order-evidence.json records both
sequences. It is not a promoted replacement; full-link relocation comparison
is still needed. No binary relocation table is patched or sorted.

## Position-dependency goal: verified outcome

The runtime now has 184 verified local target labels: 183 table destinations
and one remaining owner-relative conditional-branch target. The symbolic table
consumers reference the named mode, blit, planar-entry, even-pixel and odd-pixel
tables. The planar computed-entry anchor and both loop-back jumps use labels.
All absolute table expressions share RT_CS, the single explicit 039Ch loader
ABI base. No runtime fixups or bytes changed, and no CHEAP bytes were consumed.

The linear arithmetic full-link experiment preserves the entire load image
and the same 106 relocation locations but changes MZ relocation entries 72–81.
The ten far-call fixups occur in the opposite order. Thus normal source order
is not an exact replacement for this pinned assembler/linker combination.
The original descending-ORG source was restored unchanged. The recorded
candidate hash and comparison are in arithmetic-order-evidence.json. Removing
this constraint needs a separately proven natural emission order (potentially
historical compiler module recovery); sorting or patching the final relocation
table would violate the no-correction requirement.

Remaining position dependencies are explicit: the loader ABI base, address
operands still inside raw runtime cards, global DATA/BSS declaration recipes,
and the arithmetic module's descending ORG emission. Neither passing byte
checks nor symbolic table ownership proves all indirect caller domains safe.
