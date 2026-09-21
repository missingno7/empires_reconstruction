# Declaration conflict audit

Review command: `python tools/audit_interface_conflicts.py` (evidence in
`interface-audit.json`, census in `interface-conflicts.json`). Module-level
proof command: `python tools/probe_module.py OWNER [--source OWNER=path]`
compiles one or more production modules with the pinned toolchain, binds
them from their own objects and declared metadata, and compares code and
native DATA bytes against the original. Members of shared multi-source modules
are compiled as their whole module. Known limitation: modules that call a
secondary runtime-block public (F_28AC calls f03cc) cannot be bound by the
probe's single-entry rule and report a bind error; acceptance covers them. It
is a research probe; full acceptance remains
`python -m unittest discover -s tests -p test_build_exe.py`.

## Resolved with byte-exact proofs

Symbols below are cited by their reconstruction names; docs/current/symbol-names.json
maps the readable names now used in the sources back to them.

| Case | Resolution |
| --- | --- |
| f5321 | `void f5321(struct E far *ev, int count, int step, struct P far *q)`. The six stack words are two far pointers and two ints. The locals are an int index, an unsigned long deadline (its zero store emits the high word first, as the original does) and the previous sprite box. F_5321, F_5382 (now void) and caller F_56C6 compile exactly with the typed prototype. |
| fa036 | `long fa036(char far *dest, char far *first, ...)`: the definition walks the far pointer list to a null sentinel. The variadic prototype reproduces the definition. The F_AA1F call site must declare all five arguments (record as `struct c470_record far *`): with a `...` or untyped trailing argument the compiler builds the far record address in four more bytes. Equal stack width did not make the interfaces compatible; the historical caller evidently used a full prototype. |
| f8480 / f86c9 | `include/DIALOG.H`: one 20-byte `struct dialog` shared by f7e07 (geometry), f8480 (void, draw) and f86c9 (int, selection). F_7E07 reads offsets 12..19; all ten static descriptors in DATA carry cx=cy=w=lines=-1, which is why 12-byte views never failed. Every caller now passes `&descriptor` of a typed object; the F_56C6 alias q139d was the kind word of g139d and now names it, with the manifest binding renamed at the same DGROUP offset. |
| fa28d / C470 | `include/C470.H` merges every partial 27-byte view: value@9, flags@11, byte12, sound@13, music@15, option@17, pending@19, state@21, sub[4]@22, byte26. Offsets 13/15 are corroborated by F_AB66 and M_CB5C_CD23 writing g176e/g1772; 17/19 by M_CB5C_CD23 (toggle, -1 mask) and F_D4B3 (nonzero gate, bit mask). Offset 12 and 22..26 keep neutral names. RECORD27.H is retired; all 20 users compile exactly. |
| f652a / f7343 | Both definitions now take `int`; callers already promoted. A `char` prototype at the F_3986 call site drops CBW and is one byte short, so the callers never had one. |
| gb31 | The census now records the far pointer value and the object's near placement separately (`storage`). |
| gc0cc | `extern void interrupt (*gc0cc)(void)` in F_695E, F_697D and F_699E; the census parses function-pointer declarators. |

Full acceptance after these changes: SHA256
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10, 106
relocations in order, unchanged compiler objects, fixture-free construction.

## Parser corrections

Storage qualifiers after `*` no longer change the pointer shape or size;
function-pointer declarators (including `(* near name)(void)`, arrays of
pointers and functions returning pointers) are classified; anonymous struct
objects get per-site tags; string literals are blanked before statement
splitting. No declaration is left unsupported. Object `type` is the value
type; `storage` says whether the object itself is near or far.

## Return-type normalization and storage-view consolidation

Five Sonnet passes normalized return types symbol by symbol (definition type
wins; `void` where nothing is returned or consumed; explicit `int` where a
caller consumes AX after a bare return, as in faf45 and fd89a) and every edit
was probed before installation. Additional shared headers now describe
storage that previously had private struct views: LAYOUT.H (the C102..C12E
dialog geometry block, all int, plus gc0e8), G0DCC.H, G2FD2.H, GC91B.H,
GC316.H, GC0FE.H, GA5E.H, GB3AF.H, TBL.H, and R3E8.H for g43b4. Two objects
deliberately keep local declarations: gc5ca, which F_8434/F_8453 push as a
bare word next to gc5cc through unprototyped thunks (a far pointer type adds
a push), and gc0fe in F_7BFC, whose view does not agree with the catalog
record byte for byte.

Full acceptance after these passes: SHA256
1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10, 106
relocations in order, unchanged compiler objects.

## Remaining census (52 raw conflicts)

| Category | Count | Meaning |
| --- | --- | --- |
| INACTIVE_REFERENCE_ONLY | 29 | Only inactive reference sources disagree. |
| ACTIVE_TYPE_REVIEW | 18 | Signed/unsigned views of the same byte or pointer target (b437a, g96ee, g9bfc, gbf66, gbfcd, gc0c8, gca50, ...), int/char views (g3904, gbe, gc04a, gc04c, gc0ba, gc34e), and the two deliberate local views above. |
| RETURN_TYPE_REVIEW | 1 | f699e, whose caller only takes its address. After the ASM/library pass the runtime-block and library routines (f03xx, f4b0c, f4e9f, f4eeb, fc898, memmove, movmem, setmem, lseek, strcpy) are now declared from their ASM bodies (void unless a caller consumes AX; f6b1a is int) and the Turbo C library signatures, return type only. |
| COMPACT_MODEL_SPELLING | 2 | Remaining `char *` versus `char far *` spellings; the rest now spell `far` explicitly. |
| VARIADIC_PROTOTYPE | 1 | fa036, see above. |
| LOCAL_TAG_REUSE | 1 | File-scoped tag reused for unrelated records. |

These counts are triage categories, not defect counts. The census is not a
preprocessor or type checker and does not prove ABI safety.

Recommended next steps: review the signed/unsigned byte views with probes
(signedness changes CBW versus XOR AH,AH and compare/branch selection, so each
needs a byte-exact check), and group the remaining files into modules only
where link order and compiler behavior support it.
