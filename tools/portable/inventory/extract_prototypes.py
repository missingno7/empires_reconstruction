#!/usr/bin/env python3
"""Generate portable/include/game_funcs.h from the historical source tree.

Read-only over src/*.C and asm/*.ASM (never edits the historical tree).
Writes:
  - portable/include/game_funcs.h  (regenerated, deterministic)
  - docs/portable/funcs-inventory.md section "## Prototype extraction"
    (the rest of that file -- the CC.LIB mapping table -- is hand-written
    by portable/compat/cclib.{h,c}'s author and is preserved verbatim;
    this script only replaces its own marked section)

Usage:  python tools/portable/inventory/extract_prototypes.py [--check]
        (run from anywhere; paths resolve relative to the repo root, taken
        as three levels above this file, i.e. tools/portable/inventory/../../..)
        --check: exit 1 if regenerating would change game_funcs.h (CI use).

Parsing approach
-----------------
This is a small hand-rolled scanner, not a real C parser -- Turbo C 2.0
compact-model source is regular enough (no nested functions, no function-
local structs with function-pointer surprises) that a brace-depth walk
over a "masked" copy of each file (comments and string/char literals
blanked to spaces, preserving layout) is enough to find every top-level
`{`...`}` block and classify it as a function definition or not.

A candidate block's header text (from the end of the previous top-level
`;`/`}` to the `{`) is a function definition when it contains a top-level
`NAME(...)` group whose parameter text is either ANSI (typed) or K&R
(bare names, with the types given as `;`-terminated declarations between
the `)` and the `{`).  Everything else at top level (struct/union/enum
bodies, array/struct initializers) is skipped over (by brace matching)
without being recorded.
"""
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
SRC_DIR = REPO_ROOT / "src"
ASM_DIR = REPO_ROOT / "asm"
DOCS_DIR = REPO_ROOT / "docs" / "portable"
INCLUDE_DIR = REPO_ROOT / "portable" / "include"
ASM_INVENTORY_MD = DOCS_DIR / "asm-module-inventory.md"

GENFUNCS_OUT = INCLUDE_DIR / "game_funcs.h"
FUNCS_INVENTORY_MD = DOCS_DIR / "funcs-inventory.md"


# ===========================================================================
# 0. Text masking (blank out comments and string/char literals, keep layout)
# ===========================================================================

def mask_asm_text(text: str) -> str:
    """Return a same-length copy of a TASM/MASM source with ';'-to-end-of-
    line comments blanked (newlines preserved).  ASM comments routinely
    contain unmatched English apostrophes ("the caller's span") that would
    confuse mask_text's quote-literal scanner (which finds the next stray
    `'` anywhere in the file, including inside a later 'CODE'-style segment
    attribute, and can blank out the rest of the file) -- PUBLIC-line
    scanning never needs quote/string handling at all, so this is a
    separate, simpler mask."""
    lines = text.split("\n")
    for idx, line in enumerate(lines):
        pos = line.find(";")
        if pos != -1:
            lines[idx] = line[:pos] + " " * (len(line) - pos)
    return "\n".join(lines)


def mask_text(text: str) -> str:
    """Return a same-length copy of `text` with /*...*/, //...  comments and
    "..."/'...' literals replaced by spaces (newlines preserved)."""
    out = list(text)
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2)
            j = n if j == -1 else j + 2
            for k in range(i, j):
                if out[k] != "\n":
                    out[k] = " "
            i = j
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            j = text.find("\n", i)
            j = n if j == -1 else j
            for k in range(i, j):
                out[k] = " "
            i = j
            continue
        if c == '"' or c == "'":
            quote = c
            j = i + 1
            while j < n:
                if text[j] == "\\" and j + 1 < n:
                    j += 2
                    continue
                if text[j] == quote:
                    j += 1
                    break
                j += 1
            for k in range(i, min(j, n)):
                if out[k] != "\n":
                    out[k] = " "
            i = j
            continue
        i += 1
    masked = "".join(out)
    # Preprocessor directives (#include, #define, #ifdef, ...) carry no
    # trailing ';' and would otherwise get silently absorbed into the next
    # function header's prefix text (seen live: an #include between two
    # functions in src/FONT.C polluted the following definition's return
    # type).  Blank each directive line but plant a ';' at its last column
    # so the top-level scanner treats it as a statement boundary, without
    # shifting any character position.
    lines = masked.split("\n")
    for idx, line in enumerate(lines):
        if line.lstrip().startswith("#"):
            if line:
                lines[idx] = " " * (len(line) - 1) + ";"
            else:
                lines[idx] = line
    return "\n".join(lines)


# ===========================================================================
# 1. Type mapping (dos_types.h discipline, tu-porting-rules.md sec 2)
# ===========================================================================

DROP_TOKENS = {"register", "near", "far", "interrupt", "huge", "const", "volatile", "extern"}

BASE_TYPE_MAP = {
    "": None,  # implicit int; caller decides the marker comment
    "void": "void",
    "int": "dos_int",
    "signed": "dos_int",
    "signed int": "dos_int",
    "short": "dos_int",
    "short int": "dos_int",
    "unsigned": "dos_uint",
    "unsigned int": "dos_uint",
    "unsigned short": "dos_uint",
    "char": "dos_char",
    "signed char": "dos_char",
    "unsigned char": "dos_uchar",
    "long": "dos_long",
    "long int": "dos_long",
    "signed long": "dos_long",
    "unsigned long": "dos_ulong",
}


# Historical types that only existed to spell 8086 far pointers as two ints:
# src/INTRO.C's `struct P {int a, b;}` is the (offset, segment) pair of one
# `char far *` entry of the gbfee/buf pointer table, passed by value to
# gfx_copy_rect.  The port passes the pointer itself.
PORT_TYPE_OVERRIDES = {
    "struct P": "dos_char *",
}
STRUCT_TAGS_NOT_EMITTED = {"P"}


def map_c_type(type_str: str):
    """Map a historical type phrase (e.g. 'unsigned far *', 'char', '')
    to (mapped_str_or_None, note_or_None).  None mapped_str means implicit
    int (caller adds the dos_int + marker comment)."""
    tokens = type_str.replace("*", " * ").split()
    stars = 0
    kept = []
    for t in tokens:
        if t == "*":
            stars += 1
        elif t in DROP_TOKENS:
            continue
        else:
            kept.append(t)
    base_phrase = " ".join(kept)
    note = None
    if base_phrase in PORT_TYPE_OVERRIDES:
        # Supervisor decisions (docs/portable/state-map.md "Porting notes").
        base_phrase = PORT_TYPE_OVERRIDES[base_phrase]
    if base_phrase.startswith("struct ") or base_phrase.startswith("union ") or base_phrase.startswith("enum "):
        mapped = base_phrase  # struct/union/enum tags come from game_structs.h, kept verbatim
    elif base_phrase in BASE_TYPE_MAP:
        mapped = BASE_TYPE_MAP[base_phrase]
    else:
        mapped = base_phrase if base_phrase else None
        if base_phrase:
            note = "unrecognized-type"
    if mapped is None:
        return None, note
    if mapped == "void" and stars == 0:
        return "void", note
    if stars:
        return mapped + " " + ("*" * stars), note
    return mapped, note


# ===========================================================================
# 2. Function-definition scanner
# ===========================================================================

class FuncDef:
    def __init__(self, name, ret_type, params, source_file, notes=None):
        self.name = name
        self.ret_type = ret_type   # mapped string, e.g. "dos_int"
        self.params = params       # list of (mapped_type_str, name)
        self.source_file = source_file
        self.notes = notes or []

    def proto(self):
        if not self.params:
            args = "void"
        else:
            args = ", ".join((t if t == "..." else f"{t} {n}") for t, n in self.params)
        return f"{self.ret_type} {self.name}({args});"


def find_matching_brace(masked: str, open_pos: int) -> int:
    depth = 0
    i = open_pos
    n = len(masked)
    while i < n:
        c = masked[i]
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def find_matching_paren(text: str, open_pos: int) -> int:
    depth = 0
    i = open_pos
    n = len(text)
    while i < n:
        c = text[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


IDENT_RE = re.compile(r"[A-Za-z_]\w*")
IDENT_ONLY_RE = re.compile(r"^[A-Za-z_]\w*$")


def split_top_level(text: str, sep: str):
    """Split `text` on `sep` characters that are not inside () or []."""
    parts = []
    depth = 0
    start = 0
    for i, c in enumerate(text):
        if c in "([":
            depth += 1
        elif c in ")]":
            depth -= 1
        elif c == sep and depth == 0:
            parts.append(text[start:i])
            start = i + 1
    parts.append(text[start:])
    return parts


def parse_kr_decls(trailing: str):
    """Parse K&R parameter declarations ('int a, b; register unsigned c;')
    into {name: mapped_type_str}."""
    result = {}
    for decl in split_top_level(trailing.strip(), ";"):
        decl = decl.strip()
        if not decl:
            continue
        pieces = split_top_level(decl, ",")
        # First piece carries the base type tokens + first declarator.
        first = pieces[0].strip()
        m = re.search(r"([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*$", first)
        if not m:
            continue
        name = m.group(1)
        prefix = first[: m.start()]
        # Count/strip trailing stars that belong to this declarator.
        stripped_prefix = prefix.rstrip()
        stars = 0
        while stripped_prefix.endswith("*"):
            stars += 1
            stripped_prefix = stripped_prefix[:-1].rstrip()
        base_type_tokens = stripped_prefix
        mapped, _note = map_c_type(base_type_tokens + " " + "*" * stars)
        if mapped is None:
            mapped = "dos_int"
        result[name] = mapped
        for extra in pieces[1:]:
            extra = extra.strip()
            m2 = re.search(r"([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*$", extra)
            if not m2:
                continue
            name2 = m2.group(1)
            pre2 = extra[: m2.start()].rstrip()
            stars2 = 0
            while pre2.endswith("*"):
                stars2 += 1
                pre2 = pre2[:-1].rstrip()
            mapped2, _n2 = map_c_type(base_type_tokens + " " + "*" * stars2)
            if mapped2 is None:
                mapped2 = "dos_int"
            result[name2] = mapped2
    return result


def locate_header_text(candidate: str):
    """`candidate` is ALL masked text since the previous top-level '}' (i.e.
    it may contain several unrelated ';'-terminated statements before the
    real function header -- top-level ';' is NOT a reliable boundary by
    itself, because K&R trailing parameter declarations also end in ';' and
    are part of the header, not a separate statement).

    Returns the isolated header text (ANSI: 'RETTYPE NAME(args)'; K&R:
    'RETTYPE NAME(bare,args); type decl; type decl;'), or None if the tail
    of `candidate` doesn't look like a function header at all (struct/union/
    enum body, initializer, ...).
    """
    pieces = split_top_level(candidate, ";")
    if not pieces:
        return None
    last = pieces[-1].strip()
    if last:
        # ANSI-style (or a K&R def with zero trailing decls): the header is
        # whatever follows the last top-level ';' before '{', full stop --
        # any earlier pieces are unrelated prior top-level statements.
        if "(" not in last:
            return None
        return last
    # K&R-with-trailing-decls: the trailing empty piece is the gap between
    # the last decl's ';' and '{'.  Because there is no ';' between a K&R
    # header's ')' and its FIRST parameter declaration, that pair is fused
    # into one piece (e.g. "void f(i)\nint i" -- one piece, not two); only
    # a SECOND-or-later declaration gets its own piece.  Search backward
    # through the remaining pieces for the one whose head (before its own
    # first '(') is a clean return-type+name -- that piece is the fusion
    # of the header and (maybe) its first K&R decl; everything after it is
    # more K&R decls, everything before it is unrelated prior statements.
    for k in range(len(pieces) - 2, -1, -1):
        piece = pieces[k]
        stripped_piece = piece.strip()
        if not stripped_piece or "(" not in stripped_piece:
            continue
        popen = stripped_piece.find("(")
        pclose = find_matching_paren(stripped_piece, popen)
        if pclose == -1:
            continue
        before = stripped_piece[:popen].rstrip()
        if not re.search(r"[A-Za-z_]\w*\s*$", before):
            continue  # no identifier right before '(' -- not a header start
        header_and_name = stripped_piece[: pclose + 1]
        fused_first_decl = stripped_piece[pclose + 1 :]
        trailing_parts = [p for p in [fused_first_decl] + list(pieces[k + 1 : -1]) if p.strip()]
        trailing = (";".join(trailing_parts) + ";") if trailing_parts else ""
        return header_and_name + trailing
        # (If a candidate piece has an identifier-before-paren but turns out
        # not to be a real header once the caller tries to fully parse it,
        # we don't retry further left -- that hasn't been observed in this
        # corpus and would need a real backtracking parser to do safely.)
    return None


def try_parse_function_header(raw_candidate: str):
    """Returns a dict describing the function, or None."""
    candidate = locate_header_text(raw_candidate)
    if candidate is None:
        return None
    stripped = candidate.strip()
    if not stripped:
        return None
    if "=" in stripped:
        return None
    paren_pos = stripped.find("(")
    if paren_pos == -1:
        return None
    close_pos = find_matching_paren(stripped, paren_pos)
    if close_pos == -1:
        return None
    # Identifier immediately before '(' (skip whitespace) is the function name.
    before = stripped[:paren_pos].rstrip()
    m = re.search(r"([A-Za-z_]\w*)\s*$", before)
    if not m:
        return None
    name = m.group(1)
    if name in ("if", "for", "while", "switch", "return", "sizeof", "do", "else"):
        return None
    prefix = before[: m.start()].strip()
    params_text = stripped[paren_pos + 1 : close_pos]
    trailing = stripped[close_pos + 1 :].strip()

    kr_types = {}
    if trailing:
        # Must look like ';'-terminated declarations, not e.g. a K&R body
        # opening brace already consumed elsewhere.  Reject anything that
        # still contains unmatched braces/parens.
        if trailing.count(";") == 0:
            return None
        kr_types = parse_kr_decls(trailing)

    param_names_raw = [p.strip() for p in split_top_level(params_text, ",") if p.strip()]
    is_kr = bool(kr_types) or (
        param_names_raw
        and all(IDENT_ONLY_RE.match(p) for p in param_names_raw)
        and trailing == ""
        and param_names_raw != ["void"]
        and False  # bare names with NO trailing decls at all is ambiguous; handled below
    )

    params = []
    implicit_int_params = []
    if params_text.strip() in ("", "void"):
        params = []
    elif kr_types:
        # K&R form: params_text is the bare name list (order matters), types
        # come from kr_types (matched by NAME, not position -- see
        # OPLVOICE.C's voice_bank_retune_on for why order != decl order).
        for pname in param_names_raw:
            if not IDENT_ONLY_RE.match(pname):
                # Not really K&R after all (e.g. an ANSI type crept in) --
                # bail to the ANSI path below by returning None here so the
                # caller can retry uniformly is overkill; just fall through
                # treating whole params_text as ANSI instead.
                params = None
                break
            t = kr_types.get(pname)
            if t is None:
                t = "dos_int"
                implicit_int_params.append(pname)
            params.append((t, pname))
        if params is None:
            params = []
            kr_types = {}
    if params == [] and params_text.strip() not in ("", "void") and not kr_types:
        # ANSI-style parameter list: "TYPE name, TYPE *name2, ..."
        for piece in split_top_level(params_text, ","):
            piece = piece.strip()
            if not piece or piece == "void":
                continue
            if piece == "...":
                params.append(("...", ""))
                continue
            m2 = re.search(r"([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*$", piece)
            if not m2:
                params.append(("dos_int", piece))
                continue
            pname = m2.group(1)
            ptype_raw = piece[: m2.start()]
            arr = m2.group(2)
            stars = 0
            ptype_raw_stripped = ptype_raw.rstrip()
            while ptype_raw_stripped.endswith("*"):
                stars += 1
                ptype_raw_stripped = ptype_raw_stripped[:-1].rstrip()
            if arr:
                stars += 1  # array parameter decays to pointer
            mapped, _note = map_c_type(ptype_raw_stripped + " " + "*" * stars)
            if mapped is None:
                mapped = "dos_int"
                implicit_int_params.append(pname)
            params.append((mapped, pname))

    ret_mapped, ret_note = map_c_type(prefix)
    notes = []
    if ret_mapped is None:
        ret_mapped = "dos_int"
        if trailing or (param_names_raw and all(IDENT_ONLY_RE.match(p) for p in param_names_raw) and param_names_raw):
            notes.append("K&R: implicit int")
        else:
            notes.append("implicit int")
    elif ret_note:
        notes.append(ret_note)
    if implicit_int_params:
        notes.append("param(s) without explicit type, defaulted dos_int: " + ", ".join(implicit_int_params))

    return {
        "name": name,
        "ret_type": ret_mapped,
        "params": params,
        "notes": notes,
    }


def scan_definitions(text: str, source_label: str):
    masked = mask_text(text)
    n = len(masked)
    i = 0
    decl_start = 0
    defs = []
    while i < n:
        c = masked[i]
        if c == "{":
            candidate = masked[decl_start:i]
            end = find_matching_brace(masked, i)
            if end == -1:
                break
            parsed = try_parse_function_header(candidate)
            if parsed is not None:
                fd = FuncDef(parsed["name"], parsed["ret_type"], parsed["params"], source_label, parsed["notes"])
                defs.append(fd)
            i = end + 1
            decl_start = i
            continue
        i += 1
    return defs


# ===========================================================================
# 3. extern-declaration scanner (for conflict detection)
# ===========================================================================

EXTERN_RE = re.compile(
    r"extern\s+((?:(?:unsigned|signed|const|volatile|struct|union|enum|register|far|near|void|char|int|long|short)\s+)*"
    r"[A-Za-z_]\w*(?:\s*\*)*\s*\*?)\s*\*?\s*([A-Za-z_]\w*)\s*\(([^;()]*(?:\([^;()]*\)[^;()]*)*)\)\s*(?:,|;)"
)


def scan_extern_decls(masked_text: str):
    """Very loose scan for `extern RETTYPE name(params);` and the
    comma-chained `extern RETTYPE name1(), name2(args), ...;` form.  Returns
    a list of (name, rettype_text, params_text)."""
    out = []
    for stmt in re.finditer(r"extern\s+[^;{}]*;", masked_text):
        body = stmt.group(0)
        after_extern = body[len("extern"):].strip()
        # Split "TYPE name1(p1), name2(p2), name3(p3);" into a shared type
        # phrase (everything before the first '(') applied to each item.
        first_paren = after_extern.find("(")
        if first_paren == -1:
            continue
        head = after_extern[:first_paren]
        m = IDENT_RE.search(head[::-1])
        if not m:
            continue
        # Find the return-type prefix shared by the whole statement: it's
        # everything before the FIRST declarator's identifier.
        idm = list(IDENT_RE.finditer(head))
        if not idm:
            continue
        first_name_m = idm[-1]
        ret_prefix = head[: first_name_m.start()]
        rest = after_extern[first_name_m.start():]
        # Now split `rest` on top-level commas into declarators, each of
        # shape `name(...)` or `*name(...)`.
        for decl in split_top_level(rest.rstrip(";").rstrip(), ","):
            decl = decl.strip()
            if not decl:
                continue
            dm = re.match(r"\**\s*([A-Za-z_]\w*)\s*\(([^)]*)\)\s*$", decl)
            if not dm:
                continue
            out.append((dm.group(1), ret_prefix.strip(), dm.group(2).strip()))
    return out


def normalize_param_types(params_text: str):
    """Normalize an extern declaration's parameter text to a tuple of
    mapped type strings, for comparison against a chosen prototype.  Bare
    K&R-forward-decl parens (`()`  or `(void)`) return None (uninformative,
    not a conflict candidate)."""
    t = params_text.strip()
    if t in ("", "void"):
        return None
    out = []
    for piece in split_top_level(t, ","):
        piece = piece.strip()
        if not piece:
            continue
        if piece == "...":
            out.append("...")
            continue
        # extern decl params may or may not carry a name; try to strip a
        # trailing identifier if present (unnamed dominant style seen here
        # is bare types: "unsigned,unsigned,void far*,int").
        m = re.search(r"([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*$", piece)
        type_part = piece
        if m and not IDENT_ONLY_RE.match(piece.strip()) is False and m.group(1) not in (
            "int", "char", "long", "short", "unsigned", "signed", "void", "struct", "union", "enum",
            "far", "near", "const", "volatile", "register",
        ):
            type_part = piece[: m.start()]
            if not type_part.strip():
                type_part = piece  # bare type with no name, e.g. "unsigned"
        mapped, _note = map_c_type(type_part)
        out.append(mapped or "dos_int")
    return tuple(out)


# ===========================================================================
# 4. Service-provided skip set
# ===========================================================================

SERVICE_HEADERS = ["gfx.h", "resource.h", "timer.h", "input.h", "decode.h"]

DECL_RE = re.compile(
    r"^[A-Za-z_][\w ]*?\**\s*\b([A-Za-z_]\w*)\s*\(([^;]*)\)\s*;\s*$", re.MULTILINE
)


def scan_struct_tags(path: Path):
    if not path.exists():
        return set()
    text = path.read_text(encoding="utf-8")
    return set(re.findall(r"\bstruct\s+(\w+)\s*\{", text))


STRUCT_DEF_RE = re.compile(r"\bstruct\s+(\w+)\s*\{([^{}]*)\}\s*;")


def resolve_missing_struct_tags(all_func_defs):
    """Every `struct TAG` mentioned in an emitted prototype must resolve to
    something game_funcs.h can compile against.  game_structs.h (written by
    another agent) covers the historical *.H-derived records; anything else
    is either a small struct defined LOCALLY in its one src/*.C file right
    before use (e.g. src/PUZZLE.C's `struct piece_desc{char a,b;}`, used BY
    VALUE -- an incomplete forward declaration would not compile; or
    src/INTRO.C's `struct E`/`struct P`, found the same way) or, if no
    brace-delimited body for the tag exists anywhere in src/*.C at all
    (pointer-only use, opaque), gets a plain `struct TAG;` forward
    declaration, which is all a pointer-only use needs."""
    known = scan_struct_tags(INCLUDE_DIR / "game_structs.h")
    used = set()
    for fd in all_func_defs:
        for text in [fd.ret_type] + [t for t, _n in fd.params]:
            used.update(re.findall(r"\bstruct\s+(\w+)", text))
    missing = sorted(used - known - STRUCT_TAGS_NOT_EMITTED)

    src_masked = {}
    for path in sorted(SRC_DIR.glob("*.C")):
        src_masked[path.name] = (mask_text(path.read_text(encoding="latin-1")), path.name)

    resolved = []  # (tag, kind, text_lines, note)
    for tag in missing:
        found = None
        for fname, (masked, _fn) in src_masked.items():
            for m in STRUCT_DEF_RE.finditer(masked):
                if m.group(1) == tag:
                    found = (fname, m.group(2))
                    break
            if found:
                break
        if found:
            fname, body = found
            field_types = parse_kr_decls(body if body.strip().endswith(";") else body + ";")
            # parse_kr_decls needs name order too, not just the type map --
            # re-derive declaration order from the body text directly.
            names_in_order = []
            for decl in split_top_level(body.strip().rstrip(";"), ";"):
                for piece in split_top_level(decl, ","):
                    m2 = re.search(r"([A-Za-z_]\w*)\s*(\[[^\]]*\])?\s*$", piece.strip())
                    if m2:
                        names_in_order.append(m2.group(1))
            field_lines = [f"    {field_types.get(n, 'dos_int')} {n};" for n in names_in_order]
            resolved.append((tag, "defined", field_lines,
                              f"src/{fname}: locally-defined struct, not part of game_structs.h's curated set"))
        else:
            resolved.append((tag, "opaque", [],
                              "no brace-delimited definition found anywhere in src/*.C -- used only as an "
                              "opaque pointer (cast from a raw offset); AMBIGUOUS if a future port needs its "
                              "field layout"))
    return resolved


def scan_header_declared_functions(path: Path):
    if not path.exists():
        return {}
    text = mask_text(path.read_text(encoding="utf-8"))
    out = {}
    for m in DECL_RE.finditer(text):
        name = m.group(1)
        if name in ("if", "while", "for", "switch", "sizeof"):
            continue
        out[name] = str(path.name)
    return out


# Sound.h's 8 ASM C-facing entry points (asm/SOUND.ASM, per
# docs/portable/asm-module-inventory.md sec 9 "C-facing entry points").
# sound.h is authored alongside this script (task item 3) and declares
# exactly these, plus opl_detect() as the backend-API replacement for
# src/OPLREG.C's hardware-probing opl_detect() (tu-porting-rules.md sec 5:
# "opl_register_write, opl_detect, port I/O -> sound.h").
SOUND_H_PROVIDED = {
    "sound_tick_entry": "sound.h",
    "sound_backend_select_init": "sound.h",
    "sound_voice_table_reload": "sound.h",
    "sound_voices_reset": "sound.h",
    "sound_voices_disable_all": "sound.h",
    "opl_register_write": "sound.h",
    "stream_control_block_arm": "sound.h",
    "sound_stop_reset": "sound.h",
    "opl_detect": "sound.h",  # src/OPLREG.C's own opl_detect() is replaced outright (port I/O)
}

# Whole historical files that a hand-written portable subsystem replaces
# outright (tu-porting-rules.md sec 5): none of their functions are ever
# called, by name, from a *ported* game.c file, so none get a game_funcs.h
# prototype.  Evidence for each is a grep across src/*.C showing zero
# callers outside the file itself (recorded in funcs-inventory.md).
WHOLESALE_REPLACED_FILES = {
    "VIDEO.C": "portable/gfx (gfx.h) -- every VIDEO.C function is either a gfx.h primitive already, or one of "
               "video_alloc_framebuffer/video_set_text_mode/video_normalize_far_ptr, replaced by "
               "gfx_framebuffer_init()/portable/game/startup.c per tu-porting-rules.md sec 5",
    "RESOURCE.C": "portable/resource (resource.h + decode.h) -- resource_load_record's whole flag/type dispatch, "
                  "including its private helpers sprite_sheet_decode_sequential/sprite_sheet_decode_indexed and "
                  "the DECODE.ASM calls they make, is reimplemented by resource_load_record() per resource.h's own "
                  "header comment; grep confirms no src/*.C file outside RESOURCE.C calls any of its functions",
    "CRITERR.C": "dosio.h -- tu-porting-rules.md sec 4: harderr/hardresume/hardretn call sites are deleted and the "
                 "read-vs-write failure policy is routed through dosio.h instead",
    "STARTUP.C": "portable/game/startup.c (supervisor-written) -- tu-porting-rules.md sec 5 row 3: "
                 "cmdline_parse_args/bios_equipment_probe/video_adapter_detect/sound_backend_probe/"
                 "video_mode_select are explicitly replaced; dos_write_handle2 has no caller outside "
                 "RESOURCE.C (also wholesale-replaced) and STARTUP.C itself, so it is retired with the rest "
                 "of the file (its fputs(text,stderr) semantics, minus the trailing byte, are documented in "
                 "tu-porting-rules.md sec 4 for whoever writes startup.c)",
    "LIB_RAND.C": "portable/compat/cclib.c -- this IS the historical Turbo C rand/srand LCG this script's sibling "
                  "cclib.c reimplements byte-for-byte (state=state*0x015A4E35+1; return (state>>16)&0x7fff); "
                  "keeping both defined would collide with cclib.h's `#define rand cc_rand`",
}

# Single functions replaced within an otherwise normally-ported file.
INDIVIDUAL_REPLACEMENTS = {
    "ui_gfx_alloc": ("PLAYERSL.C", "resource.h resource_staging_init() -- resource.h's own header comment "
                                    "documents the identical ui_gfx_blob/ui_gfx_shadow_a/ui_gfx_shadow_b layout"),
    # src/GAME.C's own `main() { if (video_mode_select()) { boot_init_seed_rand();
    # game_run(); game_shutdown(); video_set_text_mode(); } }` is the historical
    # DOS entry point.  The portable executable's real C `main()` lives in
    # portable/platform/sdl3/main.c (architecture.md: "platform/sdl3/ main(),
    # window/texture, event pump, ...") -- a ported game_funcs.h prototype named
    # `main` would collide with that symbol at link time.  AMBIGUOUS / flagged
    # for Wave 3: whoever ports GAME.C must give this a new name (e.g.
    # `game_bootstrap`); this script drops it rather than emit a colliding
    # `dos_int main(void);` prototype.
    "main": ("GAME.C", "portable/platform/sdl3/main.c owns the real C `main()`; this historical "
                        "main() (the video_mode_select/boot_init_seed_rand/game_run/game_shutdown/"
                        "video_set_text_mode sequence) needs a NEW name when GAME.C is ported -- "
                        "AMBIGUOUS, not resolved by this script"),
}


def build_skip_set():
    skip = {}
    for hname in SERVICE_HEADERS:
        for name, header in scan_header_declared_functions(INCLUDE_DIR / hname).items():
            skip[name] = header
    for name, header in SOUND_H_PROVIDED.items():
        skip[name] = header
    return skip


# ===========================================================================
# 5. ASM PUBLIC scan + hand-curated signatures
#    (docs/portable/asm-module-inventory.md sec 1-9; ground-truth call-site
#    argument counts cross-checked against src/*.C where the inventory doc
#    itself flags ambiguity, e.g. draw_queue_append -- see funcs-inventory.md)
# ===========================================================================

PUBLIC_RE = re.compile(r"^\s*(?:PUBLIC|public)\s+(.+)$", re.MULTILINE)


def scan_asm_publics(path: Path):
    masked = mask_asm_text(path.read_text(encoding="latin-1"))
    names = []
    for m in PUBLIC_RE.finditer(masked):
        for raw in m.group(1).split(","):
            raw = raw.strip()
            if not raw:
                continue
            raw = raw.split()[0]  # drop any trailing comment remnants
            if raw.startswith("_"):
                raw = raw[1:]
            if raw:
                names.append(raw)
    return names


# name -> dict(params=[(type,name),...] or None for "register ABI, no
# prototype", ret='void'/'dos_int', callers=[...] or [] , note=str or None,
# skip=str or None (data label / duplicate alias -- excluded, not a service)
ASM_SIGNATURES = {
    # ---- asm/RECTQ.ASM ----
    "rect_queue_flush": dict(params=[], ret="void",
        note="no C caller found (grep-confirmed); asm-module-inventory.md sec 1 flags it as possibly dead code, "
             "reachable only from other ASM (F_233E's cast animation) -- kept as a checklist entry, not called "
             "from any src/*.C site"),

    # ---- asm/BOARDCOL.ASM ----
    "board_collision_span_or": dict(params=[("dos_int", "x"), ("dos_int", "y"), ("dos_int", "h")], ret="dos_int",
        note="callers: src/GAME.C, src/LEVEL.C"),

    # ---- asm/SPRITES.ASM ----
    "play_window_wipe_clipped": dict(params=[("dos_int", "x1"), ("dos_int", "y1"), ("dos_int", "w"), ("dos_int", "h")],
        ret="void", note="no C caller (ASM-internal, called only from sprite_table_wipe_active)"),
    "sprite_script_frame_driver": dict(params=[], ret="void", note="callers: src/GAME.C, src/LEVEL.C"),
    "sprite_table_wipe_active": dict(params=[], ret="void", note="callers: src/GAME.C, src/LEVEL.C"),
    "board_actors_draw": dict(params=[("dos_int", "y0")], ret="void", note="callers: src/BOARD.C, src/GAME.C, src/LEVEL.C"),

    # ---- asm/SPRDRAW.ASM ----
    "sprite_table_queue_draws": dict(params=[], ret="void", note="caller: src/BOARD.C"),
    "animated_tile_tick": dict(params=[], ret="void",
        note="caller: src/GAME.C; internally not a Turbo-C-shaped routine (BP used as a data register, stack "
             "patched mid-body per asm-module-inventory.md sec 4) but its external interface is a plain 0-arg call"),
    "sprite_record_adjust_draw": dict(params=[("dos_int", "id")], ret="void", note="caller: src/BOARD.C"),

    # ---- asm/ANIMROW.ASM ----
    "anim_step_row_copy": dict(
        params=[("dos_int", "src_row_idx"), ("dos_int", "dst_off"), ("dos_int", "src_off"), ("dos_int", "count"),
                ("dos_int", "dst_row_idx"), ("dos_int", "arg6"), ("dos_int", "row_stride"), ("dos_int", "arg8")],
        ret="void",
        note="caller: src/ANIMSTEP.C (2 sites, `anim_step_row_copy(a/N,b,q/N,d,e/N,f,i,stride)`); "
             "asm-module-inventory.md sec 5 leaves args 6 and 8 (its own '?' entries) unnamed -- AMBIGUOUS, "
             "named arg6/arg8 here pending a Wave-2 disassembly cross-check"),

    # ---- asm/ICONANIM.ASM ----
    "icon_list_animate_draw": dict(params=[], ret="void", note="caller: src/GAME.C"),
    "icon_frame_reset_and_draw": dict(params=[], ret="void",
        note="no C caller (grep-confirmed); asm-module-inventory.md sec 6: a direct-branch shared tail of "
             "icon_list_animate_draw, public but not an independently reachable entry point"),

    # ---- asm/DRAWQ.ASM ----
    "draw_queue_render_highlighted": dict(params=[], ret="void", note="caller: src/BOARD.C"),
    "draw_queue_render": dict(params=[], ret="void", note="caller: src/GAME.C"),

    # ---- asm/DRAWQBUF.ASM ----
    "draw_queue_reset": dict(params=[], ret="void", note="caller: src/BOARD.C"),
    "draw_queue_append": dict(
        params=[("dos_char", "attr"), ("dos_int", "x"), ("dos_int", "y"), ("dos_int", "color"), ("dos_int", "height")],
        ret="void",
        note="callers: src/BOARD.C (5 sites) and asm/SPRDRAW.ASM (ASM-to-ASM); signature taken from src/BOARD.C's "
             "own `extern void draw_queue_append(char, int, int, int, int);` (line 409), which resolves "
             "asm-module-inventory.md sec 8's own flagged ambiguity about the exact parameter count -- the "
             "routine's raw AX return (\"address of the record's final word\" per the ASM header) is never used "
             "through this C-visible `void` signature, also per BOARD.C's own extern decl"),

    # ---- asm/SOUND.ASM C-facing entry points: declared in sound.h, not here ----
    # (sound_tick_entry, sound_backend_select_init, sound_voice_table_reload,
    #  sound_voices_reset, sound_voices_disable_all, opl_register_write,
    #  stream_control_block_arm, sound_stop_reset)
}

# asm/SOUND.ASM's 33 register-ABI, ASM-to-ASM-only routines (never called
# from C -- asm-module-inventory.md sec 9 "Internal-only routines").  Listed
# as commented REGISTER ABI lines, not prototypes.
SOUND_REGISTER_ABI_INTERNAL = [
    ("sound_voice_pump_loop", "F_C1F7", "rescan the voice table in mode 2, dispatch one command per voice"),
    ("sound_voice_table_prime", "F_C232", "(re)prime every configured voice's per-voice table entries"),
    ("sound_voice_service_loop", "F_C27D", "advance each voice's counter, retrigger on note-off"),
    ("sound_command_stream_dispatch", "F_C2EA", "split/dispatch the command byte at [si+voice_stream_cursor_table]"),
    ("sound_control_value_select", "F_C359", "select and submit one value from the sound control state"),
    ("sound_command_value_derive", "F_C3DB", "derive a command value from ES:DI state, update its selector"),
    ("sound_control_block_advance", "F_C440", "advance three two-word slots in the DS:177Ch control-state block"),
    ("sound_secondary_cmd_dispatch", "F_C501", "dispatch one secondary stream command by its AH selector"),
    ("sound_command_flags_update", "F_C549", "update the paired command-control flags (v_hold/v_len) from AL"),
    ("voice_percent_scale_store", "F_C567", "signed scale-and-store for the 1788/178A state pair"),
    ("sound_param_scale4", "F_C59A", "scale a raw 0..63 value by 4, store through g1788"),
    ("f_c5a8", "F_C5A8", "store AL (zero-extended) through SI at voice_dur_table (17A4)"),
    ("sound_table_word_select_store", "F_C5B3", "select a word from the 1832h table by AL, store into state_cursor (17C4)"),
    ("f_c5c6", "F_C5C6", "store AL (zero-extended) through SI at state_text (17DC)"),
    ("voice_command_decode_apply", "F_C5D1", "decode one command byte, update the selected SI-relative note/frequency state"),
    ("voice_enable", "F_C678", "enable a voice: open the speaker gate (mode 0/1) or queue a value for the OPL bank (mode 2)"),
    ("voice_disable", "F_C6B9", "disable a voice, or submit an alternate-backend voice update"),
    ("sound_pit_divisor_program", "F_C706", "write a PIT divisor (mode 0), queue it (mode 2), or emit packed OPL nibbles"),
    ("sound_voices_reset_and_service", "F_C755", "combined reset+immediate-service helper"),
    ("opl_port_write_byte", "F_C8D4", "write one raw byte to the OPL data port"),
    ("sound_tick_step", "F_C8E2", "run one music-stream command via F_C914 when due, age the delay counter"),
    ("sound_stream_command_step", "F_C914", "fetch the next command byte, split into nibble sub-dispatch"),
    ("sound_note_dispatch", "F_C988", "look up a PIT divisor for (note, octave-shift) in notetab, program it"),
    ("sound_stream_delay_decode", "F_C9A4", "decode one ES:[DI+1] command byte into a scaled delay, store into snd_delay"),
    ("sound_ctlblock_command_dispatch", "F_CA03", "dispatch one control-block command by AH selector"),
    ("sound_ctlblock_flags_latch", "F_CA35", "latch/clear the control block's one-shot/length pair from AL"),
    ("stream_percent_scale_store", "F_CA51", "signed percentage scale-and-store -- the 1E84/1E86 twin of F_C567"),
    ("stream_base_value_set", "F_CA83", "scale a raw 0..63 value by 4, store through g1e84"),
    ("stream_note_delay_set", "F_CA91", "store AL (zero-extended) through stream_note_delay (1E92)"),
    ("stream_note_program", "F_CA9B", "look up a divisor in notetab for a shifted note delta, program it"),
    ("speaker_gate_on", "F_CAD0", "open the PC-speaker gate (timer-2 output + speaker enable bits, port 0x61)"),
    ("speaker_gate_off", "F_CADB", "close the PC-speaker gate"),
    ("pit_channel2_set_divisor", "F_CAE6", "write AX's low then high byte to PIT channel 2 (port 0x42)"),
]

# RUNTIME_BLOCK.ASM / DECODE.ASM publics that are excluded outright: either
# a data label (not a callable function) or a bare alias of a gfx_* routine
# with no C caller by that short name (grep-confirmed).
EXCLUDED_ASM_LABELS = {
    "runtime_base": "data label (runtime-slot table base), not a callable function",
    "runtime_block_end": "data label (end-of-block marker), not a callable function",
    "box": "alias of gfx_box (same asm proc, dual PUBLIC label); no C caller by this short name",
    "bar": "alias of gfx_bar; no C caller by this short name",
    "clear": "alias of gfx_clear_rect; no C caller by this short name",
    "fill": "alias of gfx_fill_rect; no C caller by this short name",
    "wipe": "alias of gfx_wipe_rect; no C caller by this short name",
    "blit": "alias of gfx_blit_bitmap; no C caller by this short name",
    "copy": "alias of gfx_copy_rect; no C caller by this short name",
}


# ===========================================================================
# 6. Driver
# ===========================================================================

def gather_src_defs():
    by_file = {}
    for path in sorted(SRC_DIR.glob("*.C")):
        text = path.read_text(encoding="latin-1")
        defs = scan_definitions(text, path.name)
        by_file[path.name] = defs
    return by_file


def gather_extern_conflicts(defs_by_file, chosen_by_name):
    """For every emitted function name, scan ALL src/*.C files' extern
    declarations and flag any typed decl whose param-type tuple disagrees
    with the chosen (definition-derived) prototype."""
    conflicts = {}  # name -> list of (file, rettype_text, params_text)
    all_masked = {}
    for path in sorted(SRC_DIR.glob("*.C")):
        all_masked[path.name] = mask_text(path.read_text(encoding="latin-1"))

    for fname, masked in all_masked.items():
        for name, rettype_text, params_text in scan_extern_decls(masked):
            if name not in chosen_by_name:
                continue
            chosen = chosen_by_name[name]
            chosen_types = tuple(t for t, _n in chosen.params if t != "...")
            decl_types = normalize_param_types(params_text)
            if decl_types is None:
                continue  # uninformative (bare `()`/`(void)`), not a conflict
            decl_types_cmp = tuple(t for t in decl_types if t != "...")
            if decl_types_cmp != chosen_types:
                conflicts.setdefault(name, []).append((fname, rettype_text, params_text))
    return conflicts


def format_group_header(label):
    return f"/* ==== {label} ==== */"


def main():
    check_only = "--check" in sys.argv

    skip_set = build_skip_set()
    defs_by_file = gather_src_defs()

    # Wholesale-replaced files: record which of their functions we drop, for
    # the inventory report, but never emit them.
    wholesale_dropped = {}
    for fname, replacement in WHOLESALE_REPLACED_FILES.items():
        names = [d.name for d in defs_by_file.get(fname, [])]
        wholesale_dropped[fname] = (names, replacement)

    emitted_by_file = {}   # historical file -> [FuncDef]
    chosen_by_name = {}    # name -> FuncDef (for conflict scan)
    skipped_service = []   # (name, file, header)
    skipped_wholesale = [] # (name, file, replacement)
    skipped_individual = []# (name, file, replacement)
    dup_names = {}

    for fname, defs in defs_by_file.items():
        if fname in WHOLESALE_REPLACED_FILES:
            for d in defs:
                skipped_wholesale.append((d.name, fname, WHOLESALE_REPLACED_FILES[fname]))
            continue
        kept = []
        for d in defs:
            if d.name in INDIVIDUAL_REPLACEMENTS:
                repl_file, repl_text = INDIVIDUAL_REPLACEMENTS[d.name]
                skipped_individual.append((d.name, fname, repl_text))
                continue
            if d.name in skip_set:
                skipped_service.append((d.name, fname, skip_set[d.name]))
                continue
            if d.name in chosen_by_name:
                dup_names.setdefault(d.name, [chosen_by_name[d.name].source_file]).append(fname)
                continue  # keep the first definition seen (deterministic: file glob order)
            kept.append(d)
            chosen_by_name[d.name] = d
        if kept:
            emitted_by_file[fname] = kept

    conflicts = gather_extern_conflicts(defs_by_file, chosen_by_name)

    # ---- ASM publics ----
    asm_emitted_by_file = {}     # asm file -> [(name, sig_dict)]
    asm_skipped_service = []     # (name, file, header)
    asm_excluded = []            # (name, file, reason)
    asm_no_signature = []        # (name, file) -- PUBLIC with no curated entry (report, do not emit)
    for path in sorted(ASM_DIR.glob("*.ASM")):
        names = scan_asm_publics(path)
        kept = []
        for name in names:
            if name in EXCLUDED_ASM_LABELS:
                asm_excluded.append((name, path.name, EXCLUDED_ASM_LABELS[name]))
                continue
            if name in skip_set:
                asm_skipped_service.append((name, path.name, skip_set[name]))
                continue
            if any(name == n for n, _addr, _role in SOUND_REGISTER_ABI_INTERNAL):
                continue  # handled as a block under asm/SOUND.ASM below
            if name in ASM_SIGNATURES:
                if name in chosen_by_name:
                    dup_names.setdefault(name, [chosen_by_name[name].source_file]).append(path.name)
                    continue
                sig = ASM_SIGNATURES[name]
                fd = FuncDef(name, sig["ret"], sig["params"], path.name, [sig.get("note")] if sig.get("note") else [])
                kept.append(fd)
                chosen_by_name[name] = fd
            else:
                asm_no_signature.append((name, path.name))
        if kept or path.name == "SOUND.ASM":
            asm_emitted_by_file[path.name] = kept

    all_func_defs = [d for defs in emitted_by_file.values() for d in defs] + \
                     [d for defs in asm_emitted_by_file.values() for d in defs]
    missing_structs = resolve_missing_struct_tags(all_func_defs)

    write_game_funcs_h(emitted_by_file, asm_emitted_by_file, missing_structs)
    write_funcs_inventory(
        skip_set, skipped_service, skipped_wholesale, skipped_individual, dup_names,
        conflicts, asm_skipped_service, asm_excluded, asm_no_signature,
        emitted_by_file, asm_emitted_by_file, missing_structs,
    )

    total_src = sum(len(v) for v in emitted_by_file.values())
    total_asm = sum(len(v) for v in asm_emitted_by_file.values())
    print(f"game_funcs.h: {total_src} src prototypes + {total_asm} asm prototypes = {total_src + total_asm} total")
    print(f"conflicts detected: {len(conflicts)}")
    print(f"duplicate-name definitions across files: {len(dup_names)}")
    if check_only:
        return 0
    return 0


def write_game_funcs_h(emitted_by_file, asm_emitted_by_file, missing_structs):
    lines = []
    lines.append("/* game_funcs.h -- prototypes for every historical function this port must")
    lines.append(" * provide: one per src/*.C function DEFINITION and per C-callable asm/*.ASM")
    lines.append(" * PUBLIC symbol, EXCLUDING functions the portable service headers already")
    lines.append(" * declare (gfx.h, resource.h, timer.h, input.h, sound.h, decode.h) and")
    lines.append(" * functions whose entire historical file is replaced outright by a")
    lines.append(" * hand-written portable subsystem (see the \"excluded\" sections of")
    lines.append(" * docs/portable/funcs-inventory.md for the full rationale/evidence).")
    lines.append(" *")
    lines.append(" * GENERATED FILE -- do not edit by hand.  Regenerate with:")
    lines.append(" *   python tools/portable/inventory/extract_prototypes.py")
    lines.append(" *")
    lines.append(" * Types follow docs/portable/tu-porting-rules.md sec 2 (dos_* aliases,")
    lines.append(" * `T far *` -> `T *`, K&R implicit-int -> dos_int with a marker comment).")
    lines.append(" * `struct NAME` types come from game_structs.h.")
    lines.append(" */")
    lines.append("#ifndef PORTABLE_GAME_FUNCS_H")
    lines.append("#define PORTABLE_GAME_FUNCS_H")
    lines.append("")
    lines.append('#include "dos_types.h"')
    lines.append('#include "game_structs.h"')
    lines.append("")
    lines.append("/* ---- provided by portable services; NOT redeclared here ----")
    lines.append(" * See the \"provided by services\" table in docs/portable/funcs-inventory.md")
    lines.append(" * for the full name -> header mapping (gfx.h, resource.h, timer.h, input.h,")
    lines.append(" * sound.h, decode.h) and for the whole-file replacements (VIDEO.C,")
    lines.append(" * RESOURCE.C, CRITERR.C, STARTUP.C, LIB_RAND.C) and single-function")
    lines.append(" * replacements (PLAYERSL.C's ui_gfx_alloc).")
    lines.append(" */")
    lines.append("")

    if missing_structs:
        lines.append("/* ---- historical struct tags used below but NOT covered by game_structs.h's")
        lines.append(" * curated set (see docs/portable/funcs-inventory.md sec 7 for provenance) ---- */")
        for tag, kind, field_lines, note in missing_structs:
            if kind == "defined":
                lines.append(f"struct {tag} {{  /* {note} */")
                lines.extend(field_lines)
                lines.append("};")
            else:
                lines.append(f"struct {tag};  /* opaque: {note} */")
        lines.append("")

    for fname in sorted(emitted_by_file.keys()):
        lines.append(format_group_header(f"src/{fname}"))
        for d in emitted_by_file[fname]:
            comment = f"  /* {'; '.join(d.notes)} */" if d.notes else ""
            lines.append(d.proto() + comment)
        lines.append("")

    for fname in sorted(asm_emitted_by_file.keys()):
        lines.append(format_group_header(f"asm/{fname}"))
        for d in asm_emitted_by_file[fname]:
            comment = f"  /* {'; '.join(d.notes)} */" if d.notes else ""
            lines.append(d.proto() + comment)
        if fname == "SOUND.ASM":
            lines.append("/* REGISTER ABI -- see docs/portable/asm-module-inventory.md sec 9")
            lines.append(" * (SI = voice index x2, ES:DI = loaded sound resource far pointer);")
            lines.append(" * ASM-to-ASM only, no C caller, no C prototype possible: */")
            for name, addr, role in SOUND_REGISTER_ABI_INTERNAL:
                lines.append(f"/*   {name} ({addr}): {role} */")
        lines.append("")

    lines.append("#endif /* PORTABLE_GAME_FUNCS_H */")
    lines.append("")
    GENFUNCS_OUT.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def write_funcs_inventory(
    skip_set, skipped_service, skipped_wholesale, skipped_individual, dup_names,
    conflicts, asm_skipped_service, asm_excluded, asm_no_signature,
    emitted_by_file, asm_emitted_by_file, missing_structs,
):
    total_src = sum(len(v) for v in emitted_by_file.values())
    total_asm = sum(len(v) for v in asm_emitted_by_file.values())

    out = []
    out.append("# game_funcs.h prototype extraction (Wave 3 umbrella headers)")
    out.append("")
    out.append("Generated by `tools/portable/inventory/extract_prototypes.py` from")
    out.append("`src/*.C` function definitions and `asm/*.ASM` PUBLIC symbols (read-only")
    out.append("scan; re-run any time to refresh). This document and")
    out.append("`portable/include/game_funcs.h` are both regenerated together -- do not")
    out.append("hand-edit either.")
    out.append("")
    out.append(f"**Totals:** {total_src} src/*.C prototypes + {total_asm} asm/*.ASM prototypes "
                f"= {total_src + total_asm} in game_funcs.h.")
    out.append("")

    out.append("## 1. Provided by portable services (skipped, not redeclared)")
    out.append("")
    out.append("Functions the service headers already declare; `game_funcs.h` must not")
    out.append("redeclare them or every ported .c file that includes `game.h` would see a")
    out.append("conflicting (or merely redundant) second prototype.")
    out.append("")
    out.append("| Function | Defined in | Provided by |")
    out.append("|---|---|---|")
    for name, fname, header in sorted(skipped_service):
        out.append(f"| `{name}` | src/{fname} | {header} |")
    for name, fname, header in sorted(asm_skipped_service):
        out.append(f"| `{name}` | asm/{fname} | {header} |")
    out.append("")

    out.append("## 2. Whole historical files replaced outright")
    out.append("")
    out.append("These files' hardware/DOS plumbing is fully superseded by a hand-written")
    out.append("portable subsystem (tu-porting-rules.md sec 5); grep across `src/*.C`")
    out.append("confirms none of their functions is called from any file outside itself,")
    out.append("so none needs a `game_funcs.h` prototype -- they are never going to be")
    out.append("mechanically ported.")
    out.append("")
    for fname, replacement in sorted(WHOLESALE_REPLACED_FILES.items()):
        names = sorted(n for n, f, _r in skipped_wholesale if f == fname)
        out.append(f"- **src/{fname}** -> {replacement}")
        out.append(f"  - functions dropped: {', '.join('`'+n+'`' for n in names) if names else '(none found)'}")
    out.append("")
    if skipped_individual:
        out.append("Single functions replaced within an otherwise normally-ported file:")
        out.append("")
        out.append("| Function | Defined in | Replaced by |")
        out.append("|---|---|---|")
        for name, fname, repl in sorted(skipped_individual):
            out.append(f"| `{name}` | src/{fname} | {repl} |")
        out.append("")

    out.append("## 3. ASM publics excluded as data labels / bare aliases")
    out.append("")
    out.append("| Symbol | ASM file | Reason |")
    out.append("|---|---|---|")
    for name, fname, reason in sorted(asm_excluded):
        out.append(f"| `_{name}` | asm/{fname} | {reason} |")
    out.append("")

    if asm_no_signature:
        out.append("## 3b. ASM publics with no curated signature (NOT emitted -- needs a follow-up pass)")
        out.append("")
        for name, fname in sorted(asm_no_signature):
            out.append(f"- `_{name}` (asm/{fname})")
        out.append("")

    out.append("## 4. Duplicate-name definitions across files")
    out.append("")
    if dup_names:
        out.append("The same function name is DEFINED in more than one place (component")
        out.append("members that keep a per-file name, or a genuine historical name reuse).")
        out.append("`game_funcs.h` keeps the first definition encountered in `src/*.C` glob")
        out.append("order (alphabetical); this is deterministic but arbitrary -- flagged for")
        out.append("human review.")
        out.append("")
        out.append("| Function | Files |")
        out.append("|---|---|")
        for name, files in sorted(dup_names.items()):
            out.append(f"| `{name}` | {', '.join(files)} |")
    else:
        out.append("(none found)")
    out.append("")

    out.append("## 5. Prototype conflicts (definition vs. historical `extern` declarations)")
    out.append("")
    if conflicts:
        out.append("For each function below, at least one `extern` declaration elsewhere in")
        out.append("`src/*.C` gives a *typed* parameter list that disagrees with the type")
        out.append("list derived from the function's own DEFINITION. Per the task rule, the")
        out.append("DEFINITION's types win; the conflicting declarations are listed for")
        out.append("reference (Wave 3 porting agents will hit these as the historical")
        out.append("codebase's own internal inconsistencies, not this script's error).")
        out.append("")
        out.append("| Function | File (def) | Chosen prototype | Conflicting declarations |")
        out.append("|---|---|---|---|")
        for name in sorted(conflicts):
            fd = None
            for fname, defs in emitted_by_file.items():
                for d in defs:
                    if d.name == name:
                        fd = d
                        break
            chosen = fd.proto() if fd else "?"
            decls = "; ".join(f"{f}: `extern {rt} {name}({pt});`" for f, rt, pt in conflicts[name])
            out.append(f"| `{name}` | src/{fd.source_file if fd else '?'} | `{chosen}` | {decls} |")
    else:
        out.append("(none found)")
    out.append("")

    out.append("## 6. CC.LIB / DOS call mapping (task item 4 -- see cclib.c/dosio.c)")
    out.append("")
    out.append("See `portable/include/cclib.h`, `portable/compat/cclib.c`,")
    out.append("`portable/include/dosio.h` and `portable/compat/dosio.c` for the")
    out.append("implementations; the mapping table (which historical call maps to which")
    out.append("portable function, and why) lives in those files' header comments, cross-")
    out.append("referenced here so both halves of task item 4 are discoverable from one")
    out.append("document:")
    out.append("")
    out.append("| Historical call | Sites (src/*.C) | Portable mapping |")
    out.append("|---|---:|---|")
    out.append("| `movmem(src,dst,n)` | 22 | `cclib.h`/`cclib.c`: `cc_movmem(src,dst,n)` (calls `memmove(dst,src,n)` -- a real function, not a macro, because CC.LIB's arg order is src,dst while memmove's is dst,src), `#define movmem cc_movmem` |")
    out.append("| `setmem(dst,n,val)` | 8 | `cclib.h`/`cclib.c`: `cc_setmem(dst,n,val)` (calls `memset(dst,val,n)`), `#define setmem cc_setmem` |")
    out.append("| `memmove` | 19 | direct `<string.h>`, identical semantics |")
    out.append("| `memset` | 2 | direct `<string.h>`, identical semantics |")
    out.append("| `strcpy`/`strlen` | 4 / 3 | direct `<string.h>`, identical semantics |")
    out.append("| `farmalloc(n)` | 10 | `cclib.h`: `#define farmalloc(n) malloc(n)` |")
    out.append("| `farfree(p)` | 22 | `cclib.h`: `#define farfree(p) free(p)` |")
    out.append("| `farcoreleft()` | 2 | retired per tu-porting-rules.md sec 6 (the VIDEO.C mode-downgrade caller is itself wholesale-replaced) |")
    out.append("| `MK_FP(seg,ofs)` / `FP_SEG(fp)` | 7 / 5 | retired per tu-porting-rules.md sec 6 (paragraph arithmetic never survives; callers are PLAYERSL.C's ui_gfx_alloc, individually replaced, and VIDEO.C, wholesale-replaced) |")
    out.append("| `rand()` / `srand(x)` | 8 / 3 (+1 spaced) | `cclib.h`: `#define rand cc_rand` / `#define srand cc_srand`, Turbo C LCG reimplemented in cclib.c |")
    out.append("| `ultoa(v,buf,radix)` | 2 | `cclib.h`/`cclib.c`: `cc_ultoa`, `#define ultoa cc_ultoa` |")
    out.append("| `itoa`/`ltoa` | 0 direct calls found in src/*.C, but declared reachable via docs/current tooling and tu-porting-rules.md sec 4 | implemented in cclib.c for completeness (`cc_itoa`/`cc_ltoa`, `#define itoa cc_itoa` / `#define ltoa cc_ltoa`) |")
    out.append("| `toupper` | 2 | direct `<ctype.h>`, identical semantics |")
    out.append("| `biostime(0,0)` (only in `srand(biostime(...))`, src/GAME.C) | 1 | `dosio.h`: `dosio_bios_ticks()` |")
    out.append("| `harderr`/`hardresume`/`hardretn` | 2/2/2 | deleted; CRITERR.C wholesale-replaced, policy routed through `dosio.h` (sec 2 above) |")
    out.append("| `exit(n)` | 2 | unchanged, `<stdlib.h>` `exit` |")
    out.append("| `inport`/`outport` | 4/0 | src/OPLREG.C's port I/O call sites are inside `opl_detect()`, which is replaced outright by `sound.h`'s backend `opl_detect()` (sec 1 above) -- no portable `inport`/`outport` shim is needed |")
    out.append("")
    out.append("`STRCATF.C`'s `str_concat_far_list` is game code (its name merely echoes")
    out.append("`strcat`), not a CC.LIB call; it gets a normal `game_funcs.h` prototype.")
    out.append("")

    out.append("## 7. `struct` tags used by an emitted prototype but not in game_structs.h")
    out.append("")
    out.append("`game_structs.h` (written separately) curates the structs recovered from")
    out.append("the historical `include/*.H` files.  A few prototypes below reference a")
    out.append("`struct` tag that never appears there; `game_funcs.h` defines (or, for an")
    out.append("opaque-pointer-only tag, forward-declares) each one itself so the header")
    out.append("compiles standalone.  Two flavors, both auto-detected by grepping")
    out.append("`src/*.C` for a `struct TAG { ... };` body:")
    out.append("")
    out.append("| Tag | Kind | Provenance |")
    out.append("|---|---|---|")
    for tag, kind, _field_lines, note in missing_structs:
        out.append(f"| `struct {tag}` | {kind} | {note} |")
    if not missing_structs:
        out.append("(none found)")
    out.append("")

    FUNCS_INVENTORY_MD.write_text("\n".join(out), encoding="utf-8", newline="\n")


if __name__ == "__main__":
    sys.exit(main())
