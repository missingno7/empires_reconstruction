#!/usr/bin/env python3
"""Migration inventory scanner for src/*.C (Wave 2 prep).

Read-only: scans the historical `src/*.C` translation units and emits
`docs/portable/tu-inventory.md` + `docs/portable/tu-inventory.json`.

Usage:  python tools/portable/inventory/scan_tus.py
        (run from anywhere; paths are resolved relative to the repo root,
        which is taken to be two levels above this file's directory's
        grandparent -- i.e. tools/portable/inventory/../../.. )

This script does not modify anything under src/, asm/, include/. It only
writes docs/portable/tu-inventory.{md,json}.
"""
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
SRC_DIR = REPO_ROOT / "src"
DOCS_OUT_MD = REPO_ROOT / "docs" / "portable" / "tu-inventory.md"
DOCS_OUT_JSON = REPO_ROOT / "docs" / "portable" / "tu-inventory.json"

REGISTER_PSEUDO_VARS = [
    "AX", "BX", "CX", "DX", "SI", "DI", "BP", "SP",
    "DS", "ES", "SS", "CS", "IP", "FLAGS",
    "AL", "AH", "BL", "BH", "CL", "CH", "DL", "DH",
]
REGISTER_RE = re.compile(r"_(" + "|".join(REGISTER_PSEUDO_VARS) + r")\b")

INTRINSIC_TOKENS = ["__int__", "__inportb__", "__outportb__", "__sti__", "__cli__", "__emit__"]

DOS_BIOS_CALLS = [
    "open", "read", "write", "close", "lseek", "biostime",
    "harderr", "hardresume", "hardretn", "getvect", "setvect",
    "exit", "srand", "rand",
    # Not in the task's literal list, but the same category (CC.LIB direct
    # port I/O) -- added after tracing the callgraph-startup.md boot path
    # found src/OPLREG.C:opl_detect() calling `inport`/`outport` directly
    # (4 sites), which the original marker set missed entirely.
    "inport", "outport",
]

ASM_RE = re.compile(r"\basm\b")
INTERRUPT_RE = re.compile(r"\binterrupt\b")
FAR_RE = re.compile(r"\bfar\b")
NEAR_RE = re.compile(r"\bnear\b")
MK_FP_RE = re.compile(r"\bMK_FP\b")
FP_SEG_RE = re.compile(r"\bFP_SEG\b")
FARMALLOC_RE = re.compile(r"\bfarmalloc\b")
FARCORELEFT_RE = re.compile(r"\bfarcoreleft\b")
FARFREE_RE = re.compile(r"\bfarfree\b")
SETJMP_RE = re.compile(r"\bsetjmp\b")
LONGJMP_RE = re.compile(r"\blongjmp\b")

DOS_CALL_RES = {name: re.compile(r"\b" + name + r"\s*\(") for name in DOS_BIOS_CALLS}

# Files/subsystems where a DOS/BIOS-shaped call is the *implementation site*
# (defines the extern / owns the hardware access) rather than a caller of a
# portable abstraction that happens to share a libc-ish name. Hand-curated
# from docs/current/portability-boundaries.md's per-boundary "Current
# source"/"Public API" citations (section numbers in comments below).
IMPLEMENTATION_SITES = {
    "RESOURCE.C": {"open", "read", "write", "close", "lseek", "farmalloc"},  # portability-boundaries.md §6, §10
    "STARTUP.C": {"exit"},  # dos_write_handle2 / video_mode_select gate, §1, §10
    "CRITERR.C": {"harderr", "hardresume", "hardretn"},  # §9
    "KEYIRQ.C": {"getvect", "setvect"},  # §3
    "LIB_RAND.C": {"srand", "rand"},  # RNG seeding/consumption is this file's whole job
}

# Hand-curated portable-interface mapping for files whose role is already
# established by docs/current/portability-boundaries.md and
# docs/portable/architecture.md. "review" means: marker(s) present but this
# script did not have a confident 1:1 mapping -- a human should assign it
# during Wave 2 triage.
INTERFACE_MAP = {
    "VIDEO.C": ["gfx"],
    "STARTUP.C": ["gfx", "sound", "dosio"],  # video_adapter_detect/video_mode_select, sound_backend_probe, dos_write_handle2
    "KEYBOARD.C": ["input"],
    "KEYIRQ.C": ["input"],
    "TIMER.C": ["timer"],
    "RESOURCE.C": ["resource", "dosio"],
    "RESCACHE.C": ["resource"],
    "CRITERR.C": ["dosio"],
    "OPLINIT.C": ["sound"],
    "OPLREG.C": ["sound"],
    "OPLVOICE.C": ["sound"],
    "MUSIC.C": ["sound"],
    "SNDFXTGL.C": ["sound"],
    "SNDREQ.C": ["sound", "timer"],
    "LIB_RAND.C": ["rng"],  # rng is not one of the six subsystem headers -- see notes: portable/compat concern, Turbo C rand()/srand() algorithm must be reproduced bit-exactly if determinism matters
    "PLAYERSL.C": ["gfx", "resource"],  # MK_FP/FP_SEG region carve (§7) + resource load + setjmp target (§9)
    "BOARD.C": ["timer", "gfx"],  # farmalloc + longjmp(2) + gfx_* callers
    "LEVEL.C": ["timer", "gfx"],  # farmalloc + longjmp(1)
    "GAME.C": ["timer", "input", "dosio", "rng"],  # setjmp target, boot_init_seed_rand sequencing, biostime (boot) + srand
    "PLRLDPUB.C": ["resource"],
    "HINTDLG.C": ["resource"],  # resource_load_record_alloc/farfree of a hint-text blob
    "INTRO.C": ["resource"],  # farfree of t1/t2 loaded via resource_load_record_alloc (lines 292-293/352)
    "PUZZLE.C": ["rng"],  # rand() only for tile rotation; the `farfree` marker hit is a comment mention (line 204), not a call site
    "ROUNDEND.C": ["rng"],  # rand() only for round-end animation choice
    "VOXCHAN.C": ["sound"],
    "VOXSLOAD.C": ["sound", "resource"],
    "FONT.C": ["gfx"],  # sprite-sheet/font-header walk (ES:SI) feeding text_draw_wrapped/dialog_line_height
    "SHBMPHIT.C": ["gfx"],  # F_1F17: shadow-bitmap collision probe against `vram`, called from FONT/GAME/LEVEL
    # RECTTAB.C: asm-bodied record-table compaction over game-object records
    # (record_table_root, called from BOARD.C/GAME.C) -- performance-critical
    # hand asm over ordinary DGROUP game data, NOT a DOS/hardware service; it
    # does not map onto any of the six portable interfaces.
    "RECTTAB.C": ["none-game-logic"],
}


def kr_style_defs(text: str) -> int:
    """Heuristic count of K&R-style (old, non-ANSI) function definitions.

    A K&R definition is a header line `name(arg1, arg2)` with a *bare*
    identifier list (no types in the parens) whose parameter TYPES are
    declared on following lines before the opening brace, e.g.:

        gfx_box(x, y, w, h)
        int x, y, w, h;
        {
            ...
        }

    This is a heuristic (regex, no real parser) -- intended for a rough
    per-file count, not a byte-exact fact. Flagged as such in the report.
    """
    lines = text.split("\n")
    count = 0
    i = 0
    n = len(lines)
    header_re = re.compile(r"^[A-Za-z_][\w]*\s*\([^;{}]*\)\s*$")
    control_kw = re.compile(r"^(if|for|while|switch|return|else|do)\b")
    while i < n:
        stripped = lines[i].strip()
        if (
            stripped
            and header_re.match(stripped)
            and not control_kw.match(stripped)
            and "{" not in stripped
        ):
            j = i + 1
            decl_lines = []
            hit_brace = False
            while j < n and (j - i) <= 20:
                s = lines[j].strip()
                if "{" in s:
                    hit_brace = True
                    break
                if s:
                    decl_lines.append(s)
                j += 1
            if hit_brace and decl_lines and all(s.endswith(";") for s in decl_lines):
                count += 1
                i = j
                continue
        i += 1
    return count


def scan_file(path: Path) -> dict:
    text = path.read_text(encoding="latin-1")
    lines = text.split("\n")
    line_count = len(lines) if text.endswith("\n") else len(lines)

    has_asm = bool(ASM_RE.search(text))
    has_interrupt = bool(INTERRUPT_RE.search(text))
    reg_matches = sorted(set(m.group(0) for m in REGISTER_RE.finditer(text)))
    intrinsics = sorted(set(tok for tok in INTRINSIC_TOKENS if tok in text))
    far_count = len(FAR_RE.findall(text))
    near_count = len(NEAR_RE.findall(text))
    has_mk_fp = bool(MK_FP_RE.search(text))
    has_fp_seg = bool(FP_SEG_RE.search(text))
    has_farmalloc = bool(FARMALLOC_RE.search(text))
    has_farcoreleft = bool(FARCORELEFT_RE.search(text))
    has_farfree = bool(FARFREE_RE.search(text))
    has_setjmp = bool(SETJMP_RE.search(text))
    has_longjmp = bool(LONGJMP_RE.search(text))

    dos_calls_found = sorted(name for name, rx in DOS_CALL_RES.items() if rx.search(text))

    kr_count = kr_style_defs(text)

    fname = path.name
    impl_sites = IMPLEMENTATION_SITES.get(fname, set())
    # "direct HW markers": things that make the TU irreducibly hardware/DOS
    # facing on their own (per docs/portable/architecture.md integer/ABI
    # discipline and docs/current/portability-boundaries.md §1-9).
    direct_hw_markers = (
        has_asm
        or has_interrupt
        or bool(reg_matches)
        or bool(intrinsics)
        or has_setjmp
        or has_longjmp
        or any(c in impl_sites for c in ("getvect", "setvect", "harderr", "hardresume", "hardretn"))
        or "inport" in dos_calls_found
        or "outport" in dos_calls_found
    )
    # "service markers": calls into a DOS/BIOS/far-heap service that must be
    # replaced by a portable service, but don't by themselves make the *file*
    # hardware-shaped (e.g. a game-logic file calling farmalloc for a buffer).
    service_markers = (
        has_farmalloc or has_farcoreleft or has_farfree or has_mk_fp or has_fp_seg
        or bool(dos_calls_found)
    )

    if direct_hw_markers:
        classification = "HW"
    elif service_markers:
        classification = "MIXED"
    else:
        classification = "PURE"

    interfaces = INTERFACE_MAP.get(fname, [])
    if classification in ("HW", "MIXED") and not interfaces:
        interfaces = ["review"]

    return {
        "file": f"src/{fname}",
        "line_count": line_count,
        "has_asm_statements": has_asm,
        "has_interrupt_functions": has_interrupt,
        "register_pseudo_vars": reg_matches,
        "intrinsics": intrinsics,
        "far_count": far_count,
        "near_count": near_count,
        "has_mk_fp": has_mk_fp,
        "has_fp_seg": has_fp_seg,
        "has_farmalloc": has_farmalloc,
        "has_farcoreleft": has_farcoreleft,
        "has_farfree": has_farfree,
        "has_setjmp": has_setjmp,
        "has_longjmp": has_longjmp,
        "dos_bios_calls": dos_calls_found,
        "kr_style_def_count_heuristic": kr_count,
        "classification": classification,
        "portable_interfaces": interfaces,
    }


def render_markdown(rows: list) -> str:
    out = []
    out.append("# Translation-unit migration inventory\n")
    out.append(
        "Generated by `tools/portable/inventory/scan_tus.py` from `src/*.C` "
        "(read-only scan; re-run any time to refresh). Machine-readable form: "
        "`docs/portable/tu-inventory.json`.\n"
    )
    out.append(
        "Columns: **Lines** (line count); **asm** (contains `asm` statements); "
        "**intr** (contains `interrupt`-qualified functions); **reg** (Turbo C "
        "register pseudo-variables, e.g. `_AX`/`_ES`); **intrin** "
        "(`__int__`/`__inportb__`/`__outportb__`/`__sti__` family); "
        "**far/near** (qualifier counts, `far`/`near`); **MK_FP/FP_SEG** "
        "(far-pointer macros); **farmalloc-fam** (`farmalloc`/`farcoreleft`/"
        "`farfree`); **setjmp/longjmp**; **DOS/BIOS calls** (from "
        "`open/read/write/close/lseek/biostime/harderr/hardresume/hardretn/"
        "getvect/setvect/exit/srand/rand`); **K&R defs** (heuristic count of "
        "old-style K&R function definitions -- regex-based, not byte-exact); "
        "**Class** (PURE/HW/MIXED); **Interfaces** (portable interface(s) an "
        "HW/MIXED file should map to: resource/gfx/timer/input/sound/dosio; "
        "`review` = not confidently assigned by this script).\n"
    )

    n_pure = sum(1 for r in rows if r["classification"] == "PURE")
    n_hw = sum(1 for r in rows if r["classification"] == "HW")
    n_mixed = sum(1 for r in rows if r["classification"] == "MIXED")
    total_lines = sum(r["line_count"] for r in rows)
    out.append(
        f"**Totals:** {len(rows)} files, {total_lines} lines. "
        f"PURE={n_pure}, HW={n_hw}, MIXED={n_mixed}.\n"
    )

    out.append(
        "| File | Lines | asm | intr | reg | intrin | far | near | MK_FP/FP_SEG "
        "| farmalloc-fam | setjmp/longjmp | DOS/BIOS calls | K&R defs | Class | Interfaces |"
    )
    out.append(
        "|---|---:|:---:|:---:|:---:|:---:|---:|---:|:---:|:---:|:---:|---|---:|:---:|---|"
    )
    for r in rows:
        reg = ",".join(r["register_pseudo_vars"]) if r["register_pseudo_vars"] else ""
        intrin = ",".join(r["intrinsics"]) if r["intrinsics"] else ""
        mkfp = "Y" if (r["has_mk_fp"] or r["has_fp_seg"]) else ""
        farfam = "Y" if (r["has_farmalloc"] or r["has_farcoreleft"] or r["has_farfree"]) else ""
        sjlj = "/".join(
            filter(None, ["setjmp" if r["has_setjmp"] else "", "longjmp" if r["has_longjmp"] else ""])
        )
        dosb = ",".join(r["dos_bios_calls"]) if r["dos_bios_calls"] else ""
        ifaces = ",".join(r["portable_interfaces"]) if r["portable_interfaces"] else ""
        out.append(
            f"| {r['file']} | {r['line_count']} | "
            f"{'Y' if r['has_asm_statements'] else ''} | "
            f"{'Y' if r['has_interrupt_functions'] else ''} | "
            f"{reg} | {intrin} | {r['far_count']} | {r['near_count']} | {mkfp} | "
            f"{farfam} | {sjlj} | {dosb} | {r['kr_style_def_count_heuristic']} | "
            f"{r['classification']} | {ifaces} |"
        )

    out.append("")
    out.append("## Notes / caveats")
    out.append(
        "- `far`/`near` counts are raw keyword-occurrence counts (word-boundary "
        "regex over the whole file, including comments); they are a rough size "
        "signal, not evidence by themselves -- most files use `far` pointers "
        "pervasively for ordinary game data and are still PURE game logic "
        "(mechanical `far`->plain-pointer conversion per "
        "docs/current/portability-boundaries.md §7)."
    )
    out.append(
        "- `K&R defs` is a regex heuristic (header line with a bare identifier "
        "list, followed by `;`-terminated declaration lines, followed by `{`) "
        "and is not a byte-exact parse; treat it as a rough per-file count."
    )
    out.append(
        "- Classification rule: **HW** if the file contains any irreducibly "
        "hardware/DOS-facing marker (`asm` statement, `interrupt` function, "
        "register pseudo-variable, `__int__`/`__inportb__`/`__outportb__`/"
        "`__sti__`, `setjmp`/`longjmp`, or a `getvect`/`setvect`/`harderr`/"
        "`hardresume`/`hardretn` call site). **MIXED** if it has no direct "
        "marker but does call a DOS/BIOS-shaped service that must be replaced "
        "(`farmalloc` family, `MK_FP`/`FP_SEG`, or one of "
        "`open/read/write/close/lseek/biostime/exit/srand/rand`). Otherwise "
        "**PURE**."
    )
    out.append(
        "- `Interfaces` is hand-curated for files whose role is established in "
        "`docs/current/portability-boundaries.md`; `review` marks an HW/MIXED "
        "file this script could not confidently place -- resolve during Wave 2 "
        "triage, not by guessing here. Two files use tags outside the six "
        "listed portable headers: `rng` (LIB_RAND.C, PUZZLE.C, ROUNDEND.C, and "
        "one of GAME.C's tags) marks `rand`/`srand`/`biostime`-seed call sites "
        "that need a *bit-exact* Turbo C RNG reimplementation for determinism, "
        "not a DOS/hardware service -- natural home is `portable/compat/`, not "
        "one of the six subsystem headers; `none-game-logic` (RECTTAB.C) marks "
        "a file whose `asm` is hand-tuned game-object bookkeeping "
        "(`record_table_*` over `record_table_root`, called from BOARD.C/"
        "GAME.C) with no DOS/hardware boundary at all -- it still needs "
        "careful byte-for-byte-behavior C translation, just not a portable "
        "service mapping."
    )
    out.append("")
    return "\n".join(out)


def main() -> int:
    if not SRC_DIR.is_dir():
        print(f"error: {SRC_DIR} not found", file=sys.stderr)
        return 1

    c_files = sorted(SRC_DIR.glob("*.C"))
    if not c_files:
        print(f"error: no *.C files under {SRC_DIR}", file=sys.stderr)
        return 1

    rows = [scan_file(p) for p in c_files]

    DOCS_OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    DOCS_OUT_JSON.write_text(json.dumps(rows, indent=2) + "\n", encoding="utf-8")

    md = render_markdown(rows)
    DOCS_OUT_MD.write_text(md, encoding="utf-8")

    print(f"scanned {len(rows)} files -> {DOCS_OUT_MD}, {DOCS_OUT_JSON}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
