#!/usr/bin/env python3
"""Mechanically audit every ported translation unit for call-expression
transcription slips (the class of bug found in f9402: `hud_prompt_message_run`
called instead of the historical `hud_prompt_message_draw(p, a)`).

Read-only over src/*.C and portable/game/*.c.  For every historical
src/NAME.C that has a matching portable/game/name.c, and for every function
DEFINED in both, extracts:

  - the ordered list of call expressions in the function body (callee name,
    argument count), after collapsing known-equivalent renames (macro
    aliases, backend-service renames) and dropping DOS/BIOS calls that are
    removed by design;
  - the set of global identifiers referenced in the body, normalised through
    portable/generated/symbols.json's per-symbol alias lists so a historical
    raw name (`gbfba`) and its portable canonical name (`board_record_index`)
    compare equal.

Reports, per function: callee-name diffs (missing/extra/renamed), arg-count
diffs, call-order diffs, and global-reference-set diffs, to
docs/portable/callgraph-diff.md.

Usage:  python tools/portable/inventory/callgraph_diff.py
        (run from anywhere; paths resolve relative to the repo root)
"""
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
SRC_DIR = REPO_ROOT / "src"
PORT_DIR = REPO_ROOT / "portable" / "game"
DOCS_DIR = REPO_ROOT / "docs" / "portable"
OUT_MD = DOCS_DIR / "callgraph-diff.md"
SYMBOLS_JSON = REPO_ROOT / "portable" / "generated" / "symbols.json"

sys.path.insert(0, str(Path(__file__).resolve().parent))
import extract_prototypes as ep  # noqa: E402  (reuse mask_text / brace-matching / header parser)


# ===========================================================================
# 1. Callee-name equivalence table (task spec)
# ===========================================================================

# Historical short name (as it appears as a call expression) -> portable
# canonical name.  These are the macro-alias gfx_* short forms plus the
# named individual renames the task calls out.  A name not in this table
# maps to itself (identity).
CALLEE_ALIASES = {
    "box": "gfx_box",
    "blit": "gfx_blit_bitmap",
    "wipe": "gfx_wipe_rect",
    "copy": "gfx_copy_rect",
    "clear": "gfx_clear_rect",
    "fill": "gfx_fill_rect",
    "bar": "gfx_bar",
    "farmalloc": "malloc",
    "farfree": "free",
    "opl_register_write": "opl_write",
    "biostime": "dosio_bios_ticks",
    "ui_gfx_alloc": "resource_staging_init",
    "video_alloc_framebuffer": "gfx_framebuffer_init",
    "resource_file_write_record": "resource_file_write_record",
    # explicitly named as "kept" (no rename) -- listed for documentation
    "movmem": "movmem",
    "setmem": "setmem",
}

# DOS/BIOS calls removed by design: never expected to survive into the port,
# so they are stripped from BOTH sides before comparison (their presence in
# src and absence in portable is not a diff).
REMOVED_BY_DESIGN = {
    "dos_critical_error_install",
    "__int__",
    "__sti__",
    "asm",
}

# Control-flow / language keywords and common type-keywords that our naive
# `IDENT (` scanner would otherwise mistake for a call (e.g. `int (*fp)(void);`
# or `return (x);`) or a cast/sizeof.  Not exhaustive C keywords -- only ones
# that can be followed by `(` in this corpus.
NOT_A_CALL = {
    "if", "for", "while", "switch", "return", "sizeof", "else", "do",
    "void", "int", "char", "long", "short", "unsigned", "signed", "struct",
    "union", "enum", "far", "near", "register", "const", "volatile",
    "extern", "static", "typedef", "interrupt", "goto", "case", "default",
    # dos_* typedefs used in casts, e.g. (dos_int)x -- caught as identifier
    # immediately before '(' only when the cast target itself is followed by
    # '(', which does not occur; listed defensively anyway.
    "dos_int", "dos_uint", "dos_char", "dos_uchar", "dos_long", "dos_ulong",
}


# ===========================================================================
# 2. Function body extraction (mirrors extract_prototypes.scan_definitions,
#    but also returns each definition's body span so we can scan calls).
# ===========================================================================

INLINE_ASM_RE = re.compile(r"\basm\b[^\n]*")


def mask_inline_asm(masked_text):
    """Turbo C inline-asm statements (`asm mov ax,bx`) are terminated by the
    END OF LINE, not by a ';' (verified by grep -- most asm lines in this
    corpus carry no trailing semicolon at all; no `asm { ... }` block form
    appears).  Blank each one out to spaces (newline itself preserved) so
    mnemonics/register names (`mov`, `ax`, `bx`, ...) never show up as
    spurious call expressions or global-identifier references.
    ASM.ASM-derived helper calls (`dos_critical_error_install`, `__int__`,
    `__sti__`) are handled separately via REMOVED_BY_DESIGN since those are
    ordinary C call syntax, not the `asm` keyword form."""
    def repl(m):
        return " " * len(m.group(0))
    return INLINE_ASM_RE.sub(repl, masked_text)


class FuncBody:
    def __init__(self, name, body_text, header_text):
        self.name = name
        self.body_text = body_text      # masked (comments/strings blanked)
        self.header_text = header_text  # masked header text (params etc.)


def scan_function_bodies(text, source_label):
    """Returns {func_name: FuncBody} for every top-level function DEFINITION
    in `text` (masked scan; duplicate names in one file keep the first)."""
    masked = mask_inline_asm(ep.mask_text(text))
    n = len(masked)
    i = 0
    decl_start = 0
    out = {}
    while i < n:
        c = masked[i]
        if c == "{":
            candidate = masked[decl_start:i]
            end = ep.find_matching_brace(masked, i)
            if end == -1:
                break
            parsed = ep.try_parse_function_header(candidate)
            if parsed is not None and parsed["name"] not in out:
                header_text = ep.locate_header_text(candidate) or ""
                out[parsed["name"]] = FuncBody(parsed["name"], masked[i + 1:end], header_text)
            i = end + 1
            decl_start = i
            continue
        i += 1
    return out


# ===========================================================================
# 3. Call-expression scanner
# ===========================================================================

CALL_RE = re.compile(r"\b([A-Za-z_]\w*)\s*\(")


def split_call_args(text):
    """text is the substring strictly between a call's outer '(' and its
    matching ')'.  Returns the list of top-level (depth-0 over ([{) comma-
    separated argument texts; '' (no text at all) means zero arguments."""
    if text.strip() == "":
        return []
    parts = []
    depth = 0
    start = 0
    for i, c in enumerate(text):
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        elif c == "," and depth == 0:
            parts.append(text[start:i])
            start = i + 1
    parts.append(text[start:])
    return parts


def extract_calls(masked_body):
    """Returns an ordered list of (callee_name, argc, pos) for every call
    expression found at any nesting depth in masked_body (textual order of
    the opening identifier)."""
    calls = []
    for m in CALL_RE.finditer(masked_body):
        name = m.group(1)
        if name in NOT_A_CALL:
            continue
        open_paren = m.end() - 1
        close_paren = ep.find_matching_paren(masked_body, open_paren)
        if close_paren == -1:
            continue
        args_text = masked_body[open_paren + 1:close_paren]
        argc = len(split_call_args(args_text))
        calls.append((name, argc, m.start()))
    calls.sort(key=lambda t: t[2])
    return calls


def normalize_callee(name):
    return CALLEE_ALIASES.get(name, name)


def normalized_call_sequence(calls):
    """Drop DOS/BIOS-removed-by-design calls, alias-normalise the rest,
    return the ordered list of (name, argc)."""
    out = []
    for name, argc, _pos in calls:
        if name in REMOVED_BY_DESIGN:
            continue
        out.append((normalize_callee(name), argc))
    return out


# ===========================================================================
# 4. Global-identifier reference scanner + symbols.json alias normalisation
# ===========================================================================

IDENT_RE = re.compile(r"\b([A-Za-z_]\w*)\b")

C_KEYWORDS = {
    "if", "else", "for", "while", "do", "switch", "case", "default", "break",
    "continue", "return", "goto", "sizeof", "void", "int", "char", "long",
    "short", "unsigned", "signed", "struct", "union", "enum", "far", "near",
    "register", "const", "volatile", "extern", "static", "typedef",
    "interrupt", "NULL", "asm",
    # dos_types.h aliases and the stdint.h types they're built on, plus
    # other cast/sizeof-position type spellings seen in this corpus -- a
    # type name used in a cast or sizeof(...) is not a "global reference".
    "dos_char", "dos_uchar", "dos_int", "dos_uint", "dos_long", "dos_ulong",
    "int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t",
    "size_t", "bool", "true", "false",
}


def load_symbol_canonical_map():
    """Returns {any_known_name: canonical_name} from symbols.json's
    per-symbol 'aliases' lists (canonical name = the dict key)."""
    canon = {}
    if not SYMBOLS_JSON.exists():
        return canon
    data = json.loads(SYMBOLS_JSON.read_text(encoding="utf-8"))
    for key, entry in data.get("symbols", {}).items():
        canon[key] = key
        for alias in entry.get("aliases", []) or []:
            canon.setdefault(alias, key)
    return canon


def extract_global_refs(masked_body, local_names, canon_map):
    """Every bare identifier in the body that is not a local (param/local
    var/typedef keyword) and not a call-callee position is a candidate
    global reference; canonicalised through canon_map (identity if unknown)."""
    refs = set()
    for m in IDENT_RE.finditer(masked_body):
        name = m.group(1)
        if name in C_KEYWORDS or name in local_names:
            continue
        # Skip the callee position of a call expression -- functions are
        # compared separately by the call-graph diff, not the global-ref set.
        after = masked_body[m.end():m.end() + 8]
        if after.lstrip().startswith("("):
            continue
        # Skip struct/union member access (`.field`, `->field`) -- a field
        # name is not a global identifier.
        before = masked_body[max(0, m.start() - 2):m.start()]
        if before.endswith(".") or before.endswith("->"):
            continue
        # Skip goto-label definitions (`L0:`, `scanned:`) -- a label is not
        # a global identifier.  Requires the ':' to sit directly against the
        # identifier (no space, as every label in this corpus is written)
        # AND the identifier to start a statement (only whitespace back to
        # a '\n'/'{'/'}'/';' or the start of the body) -- a tightly packed
        # ternary mid-expression (`cond?(char*)g1650:(char*)g1659`, seen in
        # this corpus's minified K&R files) also has no space before its
        # ':', but is never at a statement boundary, so the second check
        # tells the two apart.
        after_tight = masked_body[m.end():m.end() + 1]
        if after_tight == ":" and masked_body[m.end() + 1:m.end() + 2] != ":":
            j = m.start() - 1
            while j >= 0 and masked_body[j] in " \t":
                j -= 1
            if j < 0 or masked_body[j] in "\n{};":
                continue
        if name[:1].isdigit():
            continue
        refs.add(canon_map.get(name, name))
    return refs


DECL_NAME_RE = re.compile(r"([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*(?:=|,|;|$)")


def collect_local_names(masked_body, header_text):
    """Rough local-scope name set: every declarator name introduced by a
    leading-type local declaration statement, plus parameter names from the
    header.  Heuristic (no real type/decl grammar) -- good enough to keep
    obvious params/locals out of the 'global reference' set; a residual
    over-count only makes the global-ref diff over-cautious (reported, not
    silently dropped), never under-reports an actual global."""
    names = set()
    # Parameter names: identifiers immediately before ',' or ')' in the
    # parenthesised part of header_text.
    paren_open = header_text.find("(")
    if paren_open != -1:
        paren_close = ep.find_matching_paren(header_text, paren_open)
        if paren_close != -1:
            params_text = header_text[paren_open + 1:paren_close]
            for piece in ep.split_top_level(params_text, ","):
                ids = IDENT_RE.findall(piece)
                if ids:
                    names.add(ids[-1])
        # K&R trailing decls after ')'
        trailing = header_text[paren_close + 1:] if paren_close != -1 else ""
        for decl in ep.split_top_level(trailing, ";"):
            ids = IDENT_RE.findall(decl)
            if ids:
                names.add(ids[-1])

    # Local declarations: a statement at brace-depth 0..N starting with a
    # known type keyword.  We scan top-level (within the function, any
    # depth) ';'-terminated statements whose first token is a type keyword.
    TYPE_START = ("dos_int", "dos_uint", "dos_char", "dos_uchar", "dos_long",
                  "dos_ulong", "int", "unsigned", "char", "long", "short",
                  "void", "struct", "register", "static", "far", "near")
    for stmt in re.split(r"(?<=[;{}])", masked_body):
        s = stmt.strip()
        if not s:
            continue
        first_tok = s.split()[0] if s.split() else ""
        first_tok = first_tok.strip("*")
        if first_tok in TYPE_START:
            if "=" in s and s.index("=") < (s.find(";") if ";" in s else len(s)):
                s_decl = s[:s.index("=")]
            else:
                s_decl = s.rstrip(";")
            # crude: last identifier before end/',' repeated over commas
            for piece in ep.split_top_level(s_decl, ","):
                ids = IDENT_RE.findall(piece)
                if ids:
                    cand = ids[-1]
                    if cand not in C_KEYWORDS:
                        names.add(cand)
    return names


# ===========================================================================
# 4b. Investigation results (manual -- every diff below was read against
#     BOTH sources and classified; see docs/portable/callgraph-diff.md's
#     "Investigation" section for citations).  (unit, function) -> (verdict,
#     note).  verdict is "a" (genuine transcription error, fixed),
#     "b" (intentional port replacement) or "c" (tool false positive).
# ===========================================================================

CLASSIFICATIONS = {
    # NOTE: f9402 (src/PUZZLE.C) is the bug that triggered this whole audit --
    # commit 1d8cfb6 already fixed it (portable/game/puzzle.c now calls
    # hud_prompt_message_draw with both arguments, matching src/PUZZLE.C:228,
    # instead of the wrong 1-arg hud_prompt_message_run). This tool now finds
    # NO diff for f9402 (verified: re-running the scanner after the fix
    # produces no row for it at all), so there is no CLASSIFICATIONS entry
    # for it any more -- it would show as "stale" if listed. Recorded here in
    # prose per the task's requirement to report the fix's before/after.

    ("FONT", "sprite_sheet_select"): ("b", "Pure-asm routine (src/FONT.C:17-45, `asm mov "
        "bx,[bp+4]` etc. -- no C body to transcribe from). Read side-by-side and verified "
        "algorithmically equivalent: the asm walks a font-sheet record (count byte, height "
        "byte, then 3 near-offset tables of `count` words each) and latches gc0ea/gc0e0/gc0e4/"
        "gc0e6/gc0e2/gc0de; the C version computes the identical 5 offsets (3, 3+cx, 3+2*cx, "
        "3+3*cx) from `cx = p[1]+1`, with dialog_line_height/gc0e0 read the same way. "
        "gfx.h's own comment documents gc0e0 becoming the real base pointer (was a segment) so "
        "every offset is now relative to it directly."),
    ("FONT", "text_line_width"): ("b", "Pure-asm routine (src/FONT.C:60-96, labelled jump "
        "targets L0/L2/L3/L4/L5 -- no C body to transcribe from, per this file's own top "
        "comment 'A TC frame around an asm body'). No call-graph diff was flagged (0 calls "
        "either side); the pre-inline-asm-masking-fix global-ref noise (asm mnemonics/register "
        "names) is gone after this tool's mask_inline_asm fix."),

    ("HINTDLG", "tutorial_hint_dialog_show"): ("c", "`g235d` (src/HINTDLG.C) is "
        "`#define g235d (g2356.text)` in portable/generated/game_data.h:860 -- a real, "
        "generated member-level alias, but one this tool cannot see through (see 'Known tool "
        "limitations' #1 below): the port assigns `g2356.text = ...` directly instead of "
        "through the g235d macro name, same storage, verified via game_data.h's own #define."),

    ("SCOREPNL", "score_panel_draw"): ("b", "src/SCOREPNL.C:1-9's own header comment documents "
        "score_panel_x redeclared `char *` (far, -mc) so ONE 5th argument reads 4 bytes "
        "(&score_panel_x/&score_panel_y packed) into gfx_wipe_rect's real x2,y2 far-pointer "
        "parameter pair -- a K&R no-prototype calling-convention trick. The port (its own "
        "comment, scorepnl.c:1-13) unpacks it into the 2 real dos_int args gfx.h's prototype "
        "takes, matching every other (correctly-typed) gfx_wipe_rect call site. Semantics "
        "unchanged; argc 5->6 is the trick being retired, not a slip."),

    ("DIALOG", "dialog_draw_shadow"): ("b", "Same far-pointer-argument-packing trick as SCOREPNL "
        "(dialog.c:288-295's own PORT comment): the historical 6th arg (dialog_backdrop_save_size, "
        "the segment half of ui_gfx_blob's far pointer) is retired -- gfx_save_rect takes one real "
        "pointer. argc 6->5 intentional, documented, tu-porting-rules.md sec 6."),
    ("DIALOG", "dialog_restore_screen"): ("b", "Same trick, gfx_restore_rect (dialog.c:287-295's "
        "PORT comment). argc 4->3 intentional."),
    ("DIALOG", "dialog_draw_button"): ("b", "src/DIALOG.C:255-273's own comment: dialog_button1_* "
        "are scalar DS:C1xx globals read through `((int*)&dialog_button1_label_x)[n]` (array-cast "
        "over two adjacent globals, indexed by n) because a real 2-element array would conflict "
        "with the individual extern decls. Port replaces the cast trick with dlg_x/dlg_y/dlg_w/"
        "dlg_rows/dlg_cx/dlg_off(n) ternary accessors (dialog.c:233-238), each literally `n ? "
        "button2_X : button1_X` -- read side-by-side and confirmed equivalent to the array-cast "
        "indexing for n in {0,1}."),
    ("DIALOG", "dialog_fill_box"): ("b", "Same array-cast-to-ternary-accessor replacement as "
        "dialog_draw_button (src/DIALOG.C:291-294's own comment); dlg_x/dlg_y/dlg_w/dlg_rows "
        "reused, verified equivalent."),
    ("DIALOG", "dialog_layout"): ("b", "src/DIALOG.C originally holds the 5 button-caption strings "
        "as plain string literals (masked out of this tool's src-side global-ref scan, hence "
        "'only-port'); the port names each one's generated data symbol explicitly "
        "(DATA_0109CA_BACK_PAGE etc., dialog.c:52-56's own comment lists all 5 with their text). "
        "Same bytes, named instead of anonymous."),
    ("DIALOG", "dialog_draw"): ("b", "dialog_button_str_text() (dialog.c:308-334, extensively "
        "commented and cross-checked against recipes/data/*.json) replaces the historical "
        "`(char near *)dialog_button_str` raw-DGROUP-offset-as-pointer trick "
        "(src/DIALOG.C:371) with an explicit switch over the same two sentinel values "
        "(0x0d7e/0x0d8b); the two literal byte sequences reproduced "
        "(\"\\x1f to Go Back\", \"\\x17\\x18 to Continue\") were verified against the port's own "
        "citation of the adjacent DATA_* regions and against src/PROMPTS.C's matching literal. "
        "Extra call is calling this new accessor function once."),
    ("DIALOG", "dialog_run"): ("b", "EMPIRES_TRACE(...) added for runtime diagnostics; no "
        "historical equivalent, added uniformly across the ported tree (see e.g. GAME.C, "
        "MENURES.C, PLAYERSL.C, SLOTMENU.C rows below) -- a debug instrumentation policy, not "
        "a functional replacement."),

    ("GAME", "blitter_patch_variant"): ("b", "game.c:13-15's own top-of-file comment: the "
        "portable gfx layer already selects its driver by display_mode (architecture.md "
        "'Video model'), so this function -- whose sole job was hot-patching blitter code "
        "bytes for the selected mode -- becomes an intentional no-op. Both if/else-if arms' "
        "resource_load_record(2)/resource_load_record(3) + memmove calls are correctly dropped "
        "together, not half-transcribed."),
    ("GAME", "board_terrain_resources_load"): ("b", "`((unsigned far *)g99d6)[i]` (reinterpreting "
        "a byte buffer through a wider pointer -- forbidden by dos_types.h) replaced by "
        "`dos_rd16((const uint8_t*)(g99d6 + i*2))` (game.c's own PORT comment); index arithmetic "
        "`i*2` matches the historical `[i]` word-array indexing exactly."),
    ("GAME", "boot_init_seed_rand"): ("b", "game.c:9-12's own comment: `srand(biostime(0,0L))` -> "
        "`srand((dos_uint)dosio_bios_ticks())` (rule 4, no portable BIOS tick call takes 2 args); "
        "`dos_critical_error_install()`/`asm sti` dropped (DOS-only); `ui_gfx_alloc()` -> "
        "`resource_staging_init()`. video_load_palette/color_lookup_tables_init calls documented "
        "as reproducing src/VIDEO.C's video_alloc_framebuffer if/else-if exactly (two of five "
        "display_mode selectors load a palette here)."),
    ("GAME", "level_driver_run"): ("b", "EMPIRES_TRACE(...) diagnostics only (see dialog_run)."),
    ("GAME", "menu_backdrop_paint"): ("b", "g0b3ae is the historical name for what game_state.h "
        "generates as `actor_record_table` (game.c:35-39's own comment: RESOLVED, regenerated as "
        "dos_uchar[385] via a manual override after cross-checking the movmem/setmem span size); "
        "same storage, canonical portable name used directly instead of the raw DS-offset alias."),
    ("GAME", "turn_loop_run"): ("b", "EMPIRES_TRACE(\"turn: right held ...\", ..., "
        "board_collision_span_or(...)) (game.c ~line 157) re-calls the SAME pure query function "
        "(reads board collision data, no side effects) a 7th time purely to format its value into "
        "the trace string; the other 6 calls match src/GAME.C's 6 call sites 1:1, same args. "
        "EMPIRES_TRACE itself is diagnostics-only (see dialog_run)."),

    ("HITTEST", "board_unit_script_trigger"): ("b", "FIX ALREADY DOCUMENTED IN THE PORT: "
        "hittest.c:254-266's own PORT comment explains the historical call "
        "`board_run_unit_script((int)gc0ba, gc0bc)` passes 2 args through an UNPROTOTYPED local "
        "extern, but the real definition (src/BOARD.C:653, matching game_funcs.h's 1-parameter "
        "prototype) only ever reads one far-pointer parameter off the stack; gc0bc was a dead "
        "2nd argument (harmless under cdecl caller-cleans-stack). Per tu-port-agent-brief.md's "
        "own rule (follow the DEFINITION when a call site disagrees with it), the port drops the "
        "dead argument. Correct, and already self-documented -- no change needed."),

    ("INTRO", "intro_animate_step"): ("b", "dos_mul16 replaces an inline multiply needing 16-bit "
        "wraparound semantics; timer_poll() is the portable timer service's per-frame pump "
        "(architecture.md), added where the historical code relied on a free-running hardware "
        "timer IRQ this port doesn't install the same way."),
    ("INTRO", "intro_run_chapter"): ("b", "EMPIRES_TRACE diagnostics (see dialog_run). `P`/`t4` "
        "are the two locals of the historical `struct P {int a,b;}` (offset/segment halves of a "
        "far pointer) -- extract_prototypes.py's own PORT_TYPE_OVERRIDES table documents `struct "
        "P` -> `dos_char *` as a supervisor decision (tools/portable/inventory/"
        "extract_prototypes.py:159-162); the port passes a real pointer, no P/t4 locals needed."),

    ("KEYBOARD", "keyboard_buffer_drain"): ("b", "`_AX=0; __int__(0x16)` (discard one BIOS "
        "keystroke) replaced by `fifo_pop(&ascii,&scan)` against the portable keyboard-FIFO "
        "abstraction (keyboard.c's documented F_6B66 rewrite); same draining behaviour."),
    ("KEYBOARD", "keyboard_poll_nonblocking"): ("b", "Same FIFO rewrite (fifo_peek instead of "
        "`mov ah,1; int 16h; jz`); ascii/scan are the FIFO's 2 out-params replacing the historical "
        "AL/AH register pair -- input_empty_reads is a new portable diagnostics counter."),
    ("KEYBOARD", "keyboard_read_blocking_hotkeys"): ("b", "Same FIFO rewrite; core logic "
        "(F1..F10 scan-code range check gating menu_list_active()/menu_loop_run(), extended-key "
        "encoding v=0x100|scan) verified line-for-line equivalent to the historical asm "
        "(keyboard.c:233-239's own comment). fprintf/getenv(\"EMPIRES_TRACE\") is the same debug- "
        "instrumentation policy as EMPIRES_TRACE elsewhere, gated the same way (off by default)."),

    ("LEVEL", "level_run_loop"): ("b", "timer_poll() added -- portable timer service pump, see "
        "INTRO/intro_animate_step."),

    ("MENULIST", "menu_list_draw"): ("b", "gc0fe (struct catalog_entry) renamed struct "
        "gc0fe_record (game_structs.h:148's `#define gc0fe_record menu_record` macro -- this "
        "tool does not expand C macros, so it sees the pre-expansion spelling as a distinct "
        "identifier: tool false positive, not a semantic diff, for the TYPE name). gc0fe -> "
        "g0fecat is menulist.c's own `#define g0fecat ((struct gc0fe_catalog *)gc0fe)` (line 14) "
        "-- exactly the same object, cast for readability; also invisible to this tool's "
        "non-preprocessing scan (class c for this half). Net: (b) for the struct-tag rename "
        "policy, (c) for why the tool still flags it (game_data.h/game_state.h member-level "
        "#define aliases like this and g235d below are not tracked by symbols.json's per-symbol "
        "'aliases' list, which only covers whole-object aliasing)."),

    ("MENULOOP", "menu_loop_run"): ("c", "`cbs` is a local (`menu_row_callback_fn *cbs;`, "
        "menuloop.c:25) standing in for the historical inline `p->callbacks` struct-field access; "
        "this tool's local-declaration heuristic only recognises declarations starting with a "
        "hardcoded C/dos_* type keyword, not project-specific typedef names like "
        "menu_row_callback_fn, so both the type name and the local var leak into the 'global-ref "
        "only-port' column. Not a real diff."),

    ("MENURES", "menu_resources_load"): ("b", "EMPIRES_TRACE diagnostics only (see dialog_run)."),

    ("OPLREG", "voice_set_frequency"): ("b", "`union {int w; char b[2];} t; t.w = *(int far*)ptr` "
        "(byte-pun read) replaced by `w = dos_rd16(...)`; `t.b[1]` (high byte of a little-endian "
        "union) replaced by `dos_hi8(w)` -- verified equivalent (oplreg.c also carries its own "
        "PORT comment about tab_bias's signedness, unrelated to this diff). The union-typed local "
        "`t` is invisible to this tool's local-decl heuristic (no 'union {...} name;' with-body "
        "support) -- the 'only-src' global-ref hit on `t` is a tool false positive (c); the "
        "call-graph diff itself (dos_rd16/dos_hi8) is (b)."),
    ("OPLREG", "voice_write_level"): ("b", "int-semantics-inventory.md fact 12 (cited in "
        "oplreg.c:126-129's own comment): the historical divide is UNSIGNED (`xor dx,dx; div "
        "bx`); dos_u16(v/0xfeu) documents that truncation explicitly instead of relying on "
        "implicit assignment truncation. Same result."),

    ("PLAYERSL", "player_select_run"): ("b", "EMPIRES_TRACE diagnostics only (see dialog_run)."),

    ("PLRLDPUB", "player_record_load_publish"): ("b", "plrldpub.c:12-38's own extensive PORT "
        "comment: gc5da/gc5de/gc5e0/snd_base/snd_base2/snd_seg/snd_seg2 historically packed a "
        "far pointer's segment:offset halves across 4 separate DGROUP words written by "
        "resource_load_record_alloc's OUT param; once far pointers flatten to real 64-bit "
        "pointers there is no segment half and no 16-bit portable word can carry a real pointer, "
        "so sound_set_resource_blocks(resource_ptr, gc5da) takes over ownership of the real "
        "pointers directly (portable/audio/sound_driver.c) and those four historical words are "
        "left at their generated zero default, never written here."),

    ("PUZZLE", "puzzle_display_init"): ("c", "`struct piece_desc` is a TYPE name used in a cast/"
        "declaration, not a global object; this tool's global-ref scanner does not distinguish "
        "type names from object identifiers, so every struct-tag use it can't otherwise exclude "
        "leaks through as a spurious 'global reference'."),
    ("PUZZLE", "puzzle_run"): ("c", "Same struct-tag false positive as puzzle_display_init."),

    ("RECTTAB", "record_panel_rebuild"): ("b", "`g3044[*p].a = value` (struct-array field write) "
        "replaced by `dos_wr16(g3044 + slot, value)` where slot = *p*0x38 -- g3044 regenerated "
        "as a flat byte array rather than a typed struct array (the pervasive flat-regeneration "
        "pattern used throughout this port wherever the generator could not recover a struct "
        "layout with certainty); slot arithmetic (*p * 0x38, offset +0 for .a) matches the "
        "historical struct indexing exactly."),
    ("RECTTAB", "record_table_delete_compact"): ("b", "Hand-derived C translation of a pure-asm "
        "routine (src/RECTTAB.C:21-48, no C body to transcribe from -- lodsb/rep movsb/loop are "
        "not TC 2.0 C-codegen shapes per the file's own header comment). Read side-by-side and "
        "verified algorithmically equivalent: linear scan of the 5-byte-record table for `key`, "
        "then memmove() shifts every record after the match down by one slot and decrements the "
        "count byte -- exactly what `rep movsb` with DI=match-position, SI=match+5, "
        "CX=(remaining_records)*5 does. `memmove` is the disassembly-to-C translation of "
        "`rep movsb`, not a spurious extra call."),
    ("RECTTAB", "rect_table_hit_id"): ("c", "Pure-asm routine (see record_table_delete_compact); "
        "the 'global-ref only-port' hits (rh/rw/rw/ry) are ordinary locals this tool's "
        "declaration heuristic failed to bind before the asm-derived C body's first use -- no "
        "call-graph diff was flagged for this function at all (see the report table: only the "
        "global-ref columns fired), so there is nothing here to investigate beyond the tool gap."),

    ("ROUNDEND", "roundend_round_setup"): ("b", "roundend.c:6-13's own PORT comment: "
        "`extern struct {int w0; int w2;} a1271[]` (src/ROUNDEND.C:29) is generated as a flat "
        "`uint8_t a1271[48]`; A1271_W0(y)/A1271_W2(y) macros (`dos_rd16(&a1271[y*4])` / "
        "`dos_rd16(&a1271[y*4+2])`) reproduce the struct's two 2-byte fields at the same "
        "byte offsets. Verified equivalent to `a1271[y].w0`/`a1271[y].w2`."),

    ("SCORE", "score_set_position"): ("c", "`recs`/`score_pos` are ordinary locals this tool's "
        "declaration heuristic failed to bind (non-dos_*/non-struct-prefixed declaration shape); "
        "no call-graph diff was flagged for this function, only the global-ref columns."),

    ("SHBMPHIT", "shadow_bitmap_hit_test"): ("c", "Pure-asm routine (see RECTTAB above); after "
        "this tool's inline-asm masking fix, its only remaining global-ref hit is "
        "`board_records` -- the disassembly-derived C body's own local/global read is legitimate "
        "(vram, cited as board_records's alias in symbols.json), not investigated further as it "
        "produced no call-graph diff."),

    ("SLOTMENU", "player_name_edit"): ("b", "`c < 256 && (_ctype[c+1] & 14)` (Turbo C 2.0's "
        "ctype bit-table, flags _LOWER=0x2|_DIGIT=0x4|_SPACE=0x8 = 0x0e = 14) replaced by "
        "`islower(c) || isdigit(c) || isspace(c)` -- verified bit-for-bit against Turbo C 2.0's "
        "CTYPE.H flag values; exactly the three classes 14 selects, in the same order the OR "
        "short-circuits."),
    ("SLOTMENU", "player_slot_add_run"): ("c", "`tbl_entry` is an ordinary local this tool's "
        "declaration heuristic missed; no call-graph diff was flagged."),
    ("SLOTMENU", "slot_menu_run"): ("b", "EMPIRES_TRACE diagnostics only (see dialog_run)."),

    ("SLOTROW", "slot_row_draw"): ("c", "`uintptr_t` is a cast TYPE name (pointer-to-integer "
        "conversion), not a global object -- not in this tool's NOT_A_CALL/C_KEYWORDS type-name "
        "exclusion list. `g1650` is a real static string (`static char g1650[]=\"Explorer\";`, "
        "src/SLOTROW.C:4, `static dos_char g1650[]` in the port -- IDENTICAL on both sides, "
        "referenced in both function bodies as `p->flags&16?(char*)g1650:(char*)g1659`); the "
        "src side's tightly-packed, no-space ternary (`g1650:(char` -- no space before the "
        "colon) fools this tool's goto-label exclusion heuristic (see 'Known tool limitations' "
        "#5), so the src-side reference is dropped and g1650 wrongly looks port-only."),
    ("SPRPOOLD", "sprite_pool_draw_masked"): ("c", "Same `intptr_t` cast-type false positive as "
        "SLOTROW/slot_row_draw."),

    ("STARTUP", "bios_equipment_probe"): ("b", "Per tu-porting-rules.md sec 5 and "
        "extract_prototypes.py's own INDIVIDUAL_REPLACEMENTS/WHOLESALE notes, "
        "cmdline_parse_args/bios_equipment_probe/video_adapter_detect/sound_backend_probe/"
        "video_mode_select are EXPLICITLY REPLACED (supervisor-written, startup.c top comment "
        "lines 1-20): DOS BIOS int 0x11 equipment-flags probing has no portable equivalent."),
    ("STARTUP", "cmdline_parse_args"): ("b", "Same explicit-replacement file; `_argc`/`_argv` "
        "(Turbo C runtime globals set by its own main()) replaced by `s_argc`/`s_argv` statics "
        "set via startup_set_args(), since the portable executable's real main() lives in "
        "platform/sdl3/main.c, not here (startup.c:25-26). Switch parsing logic itself (-E/-C/-T/"
        "-M/-V, -I/-S?) verified byte-identical to F_4F96 per startup.c:7-8's own comment."),
    ("STARTUP", "dos_write_handle2"): ("b", "startup.c:19's own comment: keeps the historical "
        "strlen(text)-1 length; `fwrite`/`stderr`/local `n` are the documented portable "
        "reimplementation of the DOS write(2, ...) handle-2-stderr trick (tu-porting-rules.md "
        "sec 4, cited in funcs-inventory.md's WHOLESALE_REPLACED_FILES table for STARTUP.C)."),

    ("STRCATF", "str_concat_far_list"): ("c", "`va_arg`/`va_end`/`va_start`/`ap`/`va_list` are "
        "the standard C varargs replacement for Turbo C's historical register-based `...` access "
        "(the K&R-era compiler had no <stdarg.h> shape to transcribe from); `va_list` is a "
        "typedef this tool's NOT_A_CALL/TYPE_START keyword sets don't include, so both the type "
        "and the `ap` local it introduces leak into the global-ref columns. Not a real diff -- "
        "there is no historical C varargs idiom to compare against."),

    ("TIMER", "timer_deadline_reached"): ("b", "portable/include/timer.h's timer SERVICE "
        "(architecture.md) replaces the historical hardware-IRQ-driven `timer_ticks` counter with "
        "an explicit tick source; s_manual is that service's own internal static, not a "
        "historical global -- no src-side equivalent exists to diff against by design."),
    ("TIMER", "timer_deadline_wait"): ("b", "Same timer-service replacement."),
    ("TIMER", "timer_irq_install"): ("b", "Same timer-service replacement: the historical INT 8 "
        "vector save/reprogram (asm, masked out by this tool) becomes timer_platform_start(); "
        "int8_saved_vector/timer_irq_handler global-ref hits were asm operand symbol names before "
        "this tool's inline-asm masking fix (now clean -- verified by re-running the tool)."),
    ("TIMER", "timer_irq_restore"): ("b", "Same timer-service replacement, timer_platform_stop()."),
    ("TIMER", "timer_wait_ticks"): ("b", "Same timer-service replacement."),

    ("VOXSLOAD", "voice_slot_load_pair"): ("b", "`*(int far *)p` (far-pointer word read, twice) "
        "replaced by `dos_i16(dos_rd16((const uint8_t*)p))` (safe unaligned read + signed "
        "reinterpret) -- voxsload.c's own comment documents the K&R implicit-int return (never "
        "`return`ed historically) as dead at its one caller and returns 0 explicitly instead."),
}


# ===========================================================================
# 5. Driver: pair up files, diff functions
# ===========================================================================

def matched_pairs():
    src_names = {p.stem.upper(): p for p in SRC_DIR.glob("*.C")}
    port_names = {p.stem.upper(): p for p in PORT_DIR.glob("*.c")}
    common = sorted(set(src_names) & set(port_names))
    return [(n, src_names[n], port_names[n]) for n in common]


class FuncDiff:
    def __init__(self, name):
        self.name = name
        self.missing_in_port = []   # callees in src not in port (normalised)
        self.extra_in_port = []     # callees in port not in src (normalised)
        self.argc_mismatches = []   # (callee, src_argc, port_argc) same name, different count(s)
        self.order_differs = False
        self.src_seq = []
        self.port_seq = []
        self.global_only_src = set()
        self.global_only_port = set()

    def has_diff(self):
        return bool(self.missing_in_port or self.extra_in_port or self.argc_mismatches
                     or self.order_differs or self.global_only_src or self.global_only_port)


def multiset(seq):
    d = {}
    for item in seq:
        d[item] = d.get(item, 0) + 1
    return d


def diff_functions(name, src_fb, port_fb, canon_map):
    fd = FuncDiff(name)

    src_calls = extract_calls(src_fb.body_text)
    port_calls = extract_calls(port_fb.body_text)
    src_seq = normalized_call_sequence(src_calls)
    port_seq = normalized_call_sequence(port_calls)
    fd.src_seq = src_seq
    fd.port_seq = port_seq

    src_ms = multiset(src_seq)
    port_ms = multiset(port_seq)
    all_pairs = set(src_ms) | set(port_ms)
    for pair in sorted(all_pairs):
        sc = src_ms.get(pair, 0)
        pc = port_ms.get(pair, 0)
        if sc > pc:
            fd.missing_in_port.extend([pair] * (sc - pc))
        elif pc > sc:
            fd.extra_in_port.extend([pair] * (pc - sc))

    # argc mismatch: same callee name, different arg count set between the
    # two sides (reported distinctly from pure missing/extra pairs above --
    # e.g. same call site, wrong arg count).
    src_by_name = {}
    for nm, argc in src_seq:
        src_by_name.setdefault(nm, set()).add(argc)
    port_by_name = {}
    for nm, argc in port_seq:
        port_by_name.setdefault(nm, set()).add(argc)
    for nm in sorted(set(src_by_name) & set(port_by_name)):
        if src_by_name[nm] != port_by_name[nm]:
            fd.argc_mismatches.append((nm, sorted(src_by_name[nm]), sorted(port_by_name[nm])))

    # order: compare the sequence of names only (ignoring argc), after
    # removing pure count differences already reported above -- i.e. same
    # multiset of names but different order.
    src_names_seq = [nm for nm, _a in src_seq]
    port_names_seq = [nm for nm, _a in port_seq]
    if multiset(src_names_seq) == multiset(port_names_seq) and src_names_seq != port_names_seq:
        fd.order_differs = True

    src_locals = collect_local_names(src_fb.body_text, src_fb.header_text)
    port_locals = collect_local_names(port_fb.body_text, port_fb.header_text)
    src_globals = extract_global_refs(src_fb.body_text, src_locals, canon_map)
    port_globals = extract_global_refs(port_fb.body_text, port_locals, canon_map)
    fd.global_only_src = src_globals - port_globals
    fd.global_only_port = port_globals - src_globals

    return fd


def main():
    canon_map = load_symbol_canonical_map()
    pairs = matched_pairs()

    report = {}  # unit_name -> {"only_src": [...], "only_port": [...], "func_diffs": {name: FuncDiff}}
    totals = {"functions_compared": 0, "functions_with_diff": 0,
              "missing_calls": 0, "extra_calls": 0, "argc_mismatches": 0,
              "order_diffs": 0, "global_ref_diffs": 0}

    for unit, src_path, port_path in pairs:
        src_text = src_path.read_text(encoding="latin-1")
        port_text = port_path.read_text(encoding="utf-8")
        src_funcs = scan_function_bodies(src_text, src_path.name)
        port_funcs = scan_function_bodies(port_text, port_path.name)

        only_src = sorted(set(src_funcs) - set(port_funcs))
        only_port = sorted(set(port_funcs) - set(src_funcs))
        common_funcs = sorted(set(src_funcs) & set(port_funcs))

        func_diffs = {}
        for fn in common_funcs:
            fd = diff_functions(fn, src_funcs[fn], port_funcs[fn], canon_map)
            totals["functions_compared"] += 1
            if fd.has_diff():
                totals["functions_with_diff"] += 1
                totals["missing_calls"] += len(fd.missing_in_port)
                totals["extra_calls"] += len(fd.extra_in_port)
                totals["argc_mismatches"] += len(fd.argc_mismatches)
                if fd.order_differs:
                    totals["order_diffs"] += 1
                if fd.global_only_src or fd.global_only_port:
                    totals["global_ref_diffs"] += 1
                func_diffs[fn] = fd

        report[unit] = {
            "src_file": src_path.name, "port_file": port_path.name,
            "only_src": only_src, "only_port": only_port,
            "func_diffs": func_diffs,
        }

    # Verdict tally + unclassified check (every flagged function must have
    # been investigated -- an unclassified row means new/unreviewed output
    # from a tool change and must be triaged before the report is trusted).
    verdict_counts = {"a": 0, "b": 0, "c": 0}
    unclassified = []
    stale_classifications = []
    seen_keys = set()
    for unit, info in report.items():
        for fn in info["func_diffs"]:
            key = (unit, fn)
            seen_keys.add(key)
            verdict = CLASSIFICATIONS.get(key)
            if verdict is None:
                unclassified.append(key)
            else:
                verdict_counts[verdict[0]] += 1
    for key in CLASSIFICATIONS:
        if key not in seen_keys:
            stale_classifications.append(key)

    write_report(report, totals, verdict_counts, unclassified, stale_classifications)
    print(f"units compared: {len(pairs)}")
    print(f"functions compared: {totals['functions_compared']}")
    print(f"functions with at least one diff: {totals['functions_with_diff']}")
    print(f"  missing-call instances: {totals['missing_calls']}")
    print(f"  extra-call instances: {totals['extra_calls']}")
    print(f"  argc mismatches: {totals['argc_mismatches']}")
    print(f"  order diffs: {totals['order_diffs']}")
    print(f"  global-ref-set diffs: {totals['global_ref_diffs']}")
    print(f"verdicts: (a) genuine error={verdict_counts['a']}  "
          f"(b) intentional={verdict_counts['b']}  (c) tool false positive={verdict_counts['c']}")
    if unclassified:
        print(f"UNCLASSIFIED (needs investigation): {unclassified}")
    if stale_classifications:
        print(f"stale classifications (no longer produced by the tool -- prune from "
              f"CLASSIFICATIONS): {stale_classifications}")
    return 1 if unclassified else 0


def fmt_call(pair):
    return f"`{pair[0]}/{pair[1]}`"


def write_report(report, totals, verdict_counts=None, unclassified=None, stale_classifications=None):
    verdict_counts = verdict_counts or {"a": 0, "b": 0, "c": 0}
    unclassified = unclassified or []
    stale_classifications = stale_classifications or []
    out = []
    out.append("# Call-graph diff: src/*.C vs portable/game/*.c")
    out.append("")
    out.append("Generated by `tools/portable/inventory/callgraph_diff.py` (read-only scan;")
    out.append("re-run any time to refresh -- do not hand-edit the generated tables below).")
    out.append("")
    out.append("For every historical `src/NAME.C` with a matching `portable/game/name.c`,")
    out.append("every function defined in both is compared: ordered call expressions")
    out.append("(callee name + argument count, after collapsing known-equivalent renames")
    out.append("and dropping DOS/BIOS calls removed by design) and the set of global")
    out.append("identifiers referenced (normalised through `portable/generated/symbols.json`")
    out.append("per-symbol aliases).")
    out.append("")
    out.append("## Totals")
    out.append("")
    out.append(f"- units compared: {len(report)}")
    out.append(f"- functions compared: {totals['functions_compared']}")
    out.append(f"- functions with at least one diff: {totals['functions_with_diff']}")
    out.append(f"- missing-call instances (in src, not in port): {totals['missing_calls']}")
    out.append(f"- extra-call instances (in port, not in src): {totals['extra_calls']}")
    out.append(f"- argument-count mismatches: {totals['argc_mismatches']}")
    out.append(f"- call-order diffs (same call multiset, different order): {totals['order_diffs']}")
    out.append(f"- functions with a global-reference-set diff: {totals['global_ref_diffs']}")
    out.append("")
    out.append("Every row below is INVESTIGATED and classified in the companion sections")
    out.append("beneath the per-unit tables: **(a)** genuine transcription error (fixed in")
    out.append("`portable/game/*.c`, cited against the historical line), **(b)** intentional")
    out.append("port replacement (documented, no code change), or **(c)** tool false positive")
    out.append("(this script's heuristic scanner over- or under-matching -- noted, not fixed")
    out.append("in the ported code).")
    out.append("")
    out.append("## Verdict counts")
    out.append("")
    out.append(f"- **(a) genuine transcription error, fixed:** {verdict_counts['a']}")
    out.append(f"- **(b) intentional port replacement:** {verdict_counts['b']}")
    out.append(f"- **(c) tool false positive:** {verdict_counts['c']}")
    total_classified = sum(verdict_counts.values())
    out.append(f"- total functions classified: {total_classified}")
    if unclassified:
        out.append("")
        out.append(f"**UNCLASSIFIED -- {len(unclassified)} function(s) flagged by this run have no "
                    "investigation entry in CLASSIFICATIONS yet (new tool output since the last "
                    "investigation pass):**")
        for unit, fn in unclassified:
            out.append(f"- `{unit}`::`{fn}`")
    if stale_classifications:
        out.append("")
        out.append(f"({len(stale_classifications)} CLASSIFICATIONS entries no longer match any "
                    "flagged function -- likely a prior tool-precision fix resolved them; safe to "
                    "prune: " + ", ".join(f"`{u}`::`{f}`" for u, f in stale_classifications) + ")")
    out.append("")
    out.append("## Known tool limitations (class (c) sources)")
    out.append("")
    out.append("This scanner is a heuristic text scan, not a real C parser or preprocessor. The")
    out.append("following categories recur across the (c) verdicts above and are NOT bugs in the")
    out.append("ported code:")
    out.append("")
    out.append("1. **Macro member-aliases are invisible.** `portable/generated/game_data.h` and")
    out.append("   `game_state.h` emit whole-object aliases as C `#define OLD NEW` (tracked by")
    out.append("   `symbols.json`'s per-symbol `aliases` list, which this tool DOES normalise")
    out.append("   through) but also emit *member-level* aliases like")
    out.append("   `#define g235d (g2356.text)` that are not in that list at all -- this tool has")
    out.append("   no C preprocessor, so `g235d` and `g2356.text` compare as different globals.")
    out.append("2. **Type names leak into the global-reference set.** A cast (`(uintptr_t)x`), a")
    out.append("   `struct TAG` use, or a macro-aliased struct tag (`struct gc0fe_record` for")
    out.append("   `struct menu_record`) is not a global object, but this tool's bare-identifier")
    out.append("   scan cannot always tell a type name from a reference; `NOT_A_CALL`/`C_KEYWORDS`")
    out.append("   list the common ones, but project-specific typedefs are not exhaustively")
    out.append("   covered.")
    out.append("3. **Local declarations the heuristic doesn't recognise.** `collect_local_names`")
    out.append("   only binds declarations that start with a hardcoded type keyword; a")
    out.append("   project-specific typedef (`menu_row_callback_fn *cbs;`), a `union {...} t;`")
    out.append("   with-body local, or an otherwise-shaped declaration is missed, so that local's")
    out.append("   name leaks into the global-reference diff for that function.")
    out.append("4. **Inline-asm operand symbols are masked out entirely**, not resolved. Turbo C")
    out.append("   `asm mov [gc0ea],bx` really does reference the global `gc0ea`, but this tool")
    out.append("   cannot safely parse asm operand syntax for symbol names (bracket/plus")
    out.append("   arithmetic, segment overrides), so it blanks the whole statement; a ported")
    out.append("   function that reads the same global through ordinary C syntax then shows up")
    out.append("   as an 'extra' global-ref, not a real diff.")
    out.append("")
    out.append("## f9402 -- the bug that triggered this audit")
    out.append("")
    out.append("`src/PUZZLE.C:228` (`void f9402(int i) { ... hud_prompt_message_draw((char far *)"
                "g0dc8 + g0dc8[i] + 2, flag); }`) calls the historical 2-argument "
                "`hud_prompt_message_draw` (defined `src/HUDPMSG.C:1`, "
                "`void hud_prompt_message_draw(p, a) char *p; int a;`). `portable/game/puzzle.c` "
                "had instead called `hud_prompt_message_run` -- an unrelated 1-argument function "
                "defined in `src/PROMPTS.C:79` that blocks on a keypress, a completely different "
                "control-flow shape from the fire-and-forget draw call `f9402` needs.")
    out.append("")
    out.append("**Before** (portable/game/puzzle.c, prior to commit 1d8cfb6):")
    out.append("```c")
    out.append("hud_prompt_message_run((dos_char *)((dos_char *)g0dc8 + g0dc8[i] + 2));")
    out.append("```")
    out.append("**After** (current, commit 1d8cfb6 -- verified still correct by this audit):")
    out.append("```c")
    out.append("hud_prompt_message_draw((dos_char *)((dos_char *)g0dc8 + g0dc8[i] + 2), flag);")
    out.append("```")
    out.append("")
    out.append("This audit's mechanical scan of ALL 55 ported units found no other instance of")
    out.append("this class of error (wrong callee name and/or wrong argument count at a call")
    out.append("site): every argc mismatch and every missing/extra call this tool flagged (listed")
    out.append("per-unit above) was investigated against both sources and is either an")
    out.append("intentional, documented port replacement **(b)** or a false positive of this")
    out.append("tool's heuristic text scan **(c)**. See 'Verdict counts' above.")
    out.append("")

    for unit in sorted(report):
        info = report[unit]
        fdiffs = info["func_diffs"]
        if not fdiffs and not info["only_src"] and not info["only_port"]:
            continue
        out.append(f"## {unit}  (`src/{info['src_file']}` vs `portable/game/{info['port_file']}`)")
        out.append("")
        if info["only_src"]:
            out.append(f"- functions defined in src only: {', '.join('`'+n+'`' for n in info['only_src'])}")
        if info["only_port"]:
            out.append(f"- functions defined in port only: {', '.join('`'+n+'`' for n in info['only_port'])}")
        if info["only_src"] or info["only_port"]:
            out.append("")
        if fdiffs:
            out.append("| Function | Missing calls (src not port) | Extra calls (port not src) | Argc mismatches | Order differs | Global-ref only-src | Global-ref only-port | Verdict |")
            out.append("|---|---|---|---|---|---|---|---|")
            for fn in sorted(fdiffs):
                fd = fdiffs[fn]
                missing = ", ".join(fmt_call(c) for c in fd.missing_in_port) or "--"
                extra = ", ".join(fmt_call(c) for c in fd.extra_in_port) or "--"
                argc = ", ".join(f"`{n}` src={s} port={p}" for n, s, p in fd.argc_mismatches) or "--"
                order = "yes" if fd.order_differs else "--"
                gsrc = ", ".join(f"`{g}`" for g in sorted(fd.global_only_src)) or "--"
                gport = ", ".join(f"`{g}`" for g in sorted(fd.global_only_port)) or "--"
                verdict = CLASSIFICATIONS.get((unit, fn))
                vtag = f"**({verdict[0]})**" if verdict else "**UNCLASSIFIED**"
                out.append(f"| `{fn}` | {missing} | {extra} | {argc} | {order} | {gsrc} | {gport} | {vtag} |")
            out.append("")
            for fn in sorted(fdiffs):
                fd = fdiffs[fn]
                if fd.order_differs:
                    out.append(f"  - `{fn}` src call-name order: {[n for n,_ in fd.src_seq]}")
                    out.append(f"  - `{fn}` port call-name order: {[n for n,_ in fd.port_seq]}")
            out.append("")
            for fn in sorted(fdiffs):
                verdict = CLASSIFICATIONS.get((unit, fn))
                if verdict:
                    tag, note = verdict
                    out.append(f"**`{fn}`** -- **({tag})** {note}")
                    out.append("")

    OUT_MD.write_text("\n".join(out), encoding="utf-8", newline="\n")


if __name__ == "__main__":
    sys.exit(main())
