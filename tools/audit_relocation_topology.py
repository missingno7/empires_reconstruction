"""Audit relocation-table direction (ascending/descending) per module.

Established structural rule (verified on the exact build): the EXE's 106
relocations are written by TLINK in the order of each object's FIXUPP
subrecords, object by object in link order. Turbo C's native object writer
emits FIXUPP subrecords in DESCENDING offset order; our generated DATA object
(tools/data_omf.py) emits descending on purpose; TASM-produced objects
(hand-written assembler modules, Turbo C `-B` units, and C units containing an
inline `asm` statement, which all restart the source through the assembler)
emit ASCENDING.

This module classifies each production-plan module's *observed* relocation
direction (read from assets/AEPROG.EXE via tools/mz.py) and *predicts* a
direction from its production route (tool/flags/source), then reports every
mismatch as evidence about historical provenance: a TASM module whose
relocations descend was, before reconstruction, a native Turbo C unit; a C
unit compiled with -B (or containing inline asm) whose relocations descend
was, likewise, not genuinely reassembled the way its flags suggest.
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))

from mz import MZ  # noqa: E402
from omf import OmfReader  # noqa: E402

PLAN_PATH = ROOT / "layout/production-plan.json"
EXE_PATH = ROOT / "assets/AEPROG.EXE"
OUT_JSON = ROOT / "docs/current/relocation-topology.json"
OUT_MD = ROOT / "docs/current/relocation-topology.md"

RULE_STATEMENT = (
    "The EXE's 106 relocations are written by TLINK in the order of each "
    "object's FIXUPP subrecords, object by object in link order. Turbo C's "
    "native object writer emits FIXUPP subrecords in DESCENDING source-offset "
    "order; the generated DATA object (tools/data_omf.py) emits descending on "
    "purpose; TASM-produced objects -- hand-written assembler modules, Turbo C "
    "units compiled with '-B', and C units containing an inline `asm` "
    "statement (all of which restart the source through the assembler) -- "
    "emit ASCENDING. MUSIC (M_DDD9_DF98) is historically descending, so its "
    "original was a pure Turbo C unit; the TASM module reproduces it with "
    "descending ORG contributions."
)


def classify_direction(offsets: list[int]) -> str:
    """Classify a sequence of relocation offsets in table order."""
    if len(offsets) == 0:
        return "none"
    if len(offsets) == 1:
        return "single"
    ascending = all(a < b for a, b in zip(offsets, offsets[1:]))
    descending = all(a > b for a, b in zip(offsets, offsets[1:]))
    if ascending:
        return "ascending"
    if descending:
        return "descending"
    return "mixed"


def source_has_inline_asm(source_path: Path) -> bool:
    if not source_path.exists():
        return False
    text = source_path.read_text(errors="ignore")
    # Strip comments and string literals first: a comment such as "no inline asm"
    # or a path like asm/MUSIC.ASM must not count as an asm statement.
    text = re.sub(r"/\*.*?\*/|//[^\n]*", " ", text, flags=re.S)
    text = re.sub(r'"(?:\\.|[^"\\])*"', '""', text)
    return re.search(r"(^|[;{}\s])asm\b", text) is not None


def predict_direction(module: dict) -> tuple[str | None, str]:
    """Return (predicted_direction_or_None, reason)."""
    tool = module.get("tool")
    flags = module.get("flags", [])
    sources = module.get("sources") or ([module["source"]] if "source" in module else [])

    if tool == "TASM.EXE":
        return "ascending", "tool is TASM.EXE"
    if tool == "TCC.EXE":
        if "-B" in flags:
            return "ascending", "TCC.EXE with -B (restarts through the assembler)"
        for src in sources:
            if source_has_inline_asm(ROOT / src):
                return "ascending", f"TCC.EXE source {src} contains an `asm` statement"
        return "descending", "TCC.EXE, no -B, no inline asm"
    return None, f"unrecognized tool {tool!r}"


def load_module_relocations(plan: dict, exe_relocations: list[dict]):
    """For each module, collect relocation load_offsets falling inside it."""
    results = []
    for module in plan["modules"]:
        start, end = module["start"], module["end"]
        lo, hi = start - 512, end - 512
        offsets_in_module = [r["load_offset"] for r in exe_relocations
                              if lo <= r["load_offset"] < hi]
        results.append((module, offsets_in_module))
    return results


def classify_generated_objects(plan: dict, exe_relocations: list[dict]):
    """DATA/BSS objects and other library-pulled objects not in `modules`."""
    module_objects = {m["object"] for m in plan["modules"] if "object" in m}
    data_objects = {d["object"]: d for d in plan.get("data", [])}
    bss_objects = {b["object"]: b for b in plan.get("bss", [])}
    other = [o for o in plan["object_order"]
             if o not in module_objects and o not in data_objects and o not in bss_objects]
    return data_objects, bss_objects, other


def audit(plan_path=PLAN_PATH, exe_path=EXE_PATH):
    plan = json.loads(plan_path.read_text())
    exe_data = exe_path.read_bytes()
    mz = MZ.parse(exe_data)

    module_rows = []
    mismatches = []
    for module, offsets in load_module_relocations(plan, mz.relocations):
        observed = classify_direction(offsets)
        predicted, reason = predict_direction(module)
        row = {
            "module": module["id"],
            "tool": module.get("tool"),
            "flags": module.get("flags", []),
            "source": module.get("sources") or module.get("source"),
            "relocation_count": len(offsets),
            "relocation_offsets": offsets,
            "observed_direction": observed,
            "predicted_direction": predicted,
            "prediction_reason": reason,
            "category": "production-module",
        }
        if predicted is not None and observed in ("ascending", "descending") \
                and observed != predicted:
            row["mismatch"] = True
            implication = (
                f"{module['id']} is produced via {reason}, predicting "
                f"{predicted} relocations, but the EXE shows {observed} "
                f"relocations. This is evidence the ORIGINAL object for this "
                f"module was NOT produced the way its current production "
                f"route implies: "
            )
            if predicted == "ascending" and observed == "descending":
                implication += (
                    "a TASM/assembler-restart route producing descending "
                    "output means the ORIGINAL unit was compiled directly by "
                    "Turbo C (native descending FIXUPP order), not hand- or "
                    "TASM-assembled."
                )
            else:
                implication += (
                    "a plain-TCC route (no -B, no inline asm) producing "
                    "ascending output means the ORIGINAL unit was, in fact, "
                    "assembled (TASM, -B, or inline asm), contradicting the "
                    "current production route's implied provenance."
                )
            row["implication"] = implication
            mismatches.append(row)
        else:
            row["mismatch"] = False
        module_rows.append(row)

    data_objects, bss_objects, other_objects = classify_generated_objects(plan, mz.relocations)

    # Build per-object address ranges for generated (DATA/BSS) objects.
    generated_rows = []
    # DATA objects: locate by object_order position isn't enough; we only
    # have module boundaries for code modules. DATA/BSS regions live between
    # code modules per data_placements/bss entries, which carry logical
    # offsets relative to their own cluster, not EXE-file offsets directly.
    # We report them by declared intent only (tools/data_omf.py comment) plus
    # whatever their id's own bss logical_start/end gives us, when present.
    for obj, entry in data_objects.items():
        generated_rows.append({
            "object": obj,
            "id": entry.get("id"),
            "category": "generated-data",
            "predicted_direction": "descending",
            "prediction_reason": "tools/data_omf.py emits DATA fixups descending on purpose",
            "note": "No EXE-relative module span recorded for DATA objects in "
                    "production-plan.json; direction is asserted by the "
                    "generator's own design comment, not independently "
                    "re-observed here.",
        })
    for obj, entry in bss_objects.items():
        generated_rows.append({
            "object": obj,
            "id": entry.get("id"),
            "category": "generated-bss",
            "predicted_direction": None,
            "prediction_reason": "BSS objects declare storage only (no LEDATA); "
                                  "typically carry no fixups",
        })
    for obj in other_objects:
        generated_rows.append({
            "object": obj,
            "id": None,
            "category": "library (CC.LIB)",
            "predicted_direction": None,
            "prediction_reason": "pulled from the pinned CC.LIB by TLINK, not a "
                                  "production-plan module",
        })

    # Fresh-compile verification, if a build/production-*/compile/WORK exists.
    fresh_compile = audit_fresh_compile(plan)

    # Every TASM module's historical direction, so a descending TASM module
    # (a native-Turbo-C fingerprint, per the rule) is visible even when it is
    # already flagged as a mismatch above.
    tasm_descending_candidates = [
        {"module": row["module"], "observed_direction": row["observed_direction"],
         "relocation_count": row["relocation_count"]}
        for row in module_rows
        if row["tool"] == "TASM.EXE" and row["observed_direction"] == "descending"
    ]

    result = {
        "format": "empires-relocation-topology-v1",
        "rule": RULE_STATEMENT,
        "exe": str(exe_path.relative_to(ROOT)),
        "plan": str(plan_path.relative_to(ROOT)),
        "total_exe_relocations": len(mz.relocations),
        "modules": module_rows,
        "generated_and_library_objects": generated_rows,
        "mismatches": mismatches,
        "fresh_compile_verification": fresh_compile,
        "tasm_descending_candidates": tasm_descending_candidates,
    }
    return result


def audit_fresh_compile(plan: dict):
    """Check raw FIXUPP order in a fresh build/production-*/compile/WORK, if any."""
    candidates = sorted(
        (p for p in (ROOT / "build").glob("production-*/compile/WORK") if p.is_dir()),
        key=lambda p: p.stat().st_mtime, reverse=True,
    )
    if not candidates:
        return {"found": False, "note": "No build/production-*/compile/WORK directory present."}
    work_dir = candidates[0]
    object_module = {m["object"]: m for m in plan["modules"] if "object" in m}
    reader = OmfReader()
    rows = []
    agreements = 0
    disagreements = 0
    for obj_path in sorted(work_dir.glob("*.OBJ")):
        module = object_module.get(obj_path.name)
        if module is None:
            continue
        try:
            om = reader.read_file(obj_path)
        except Exception as exc:  # pragma: no cover - defensive
            rows.append({"object": obj_path.name, "module": module["id"], "error": str(exc)})
            continue
        # Raw record order: fixups list preserves the order records were
        # read (module.fixups is the "legacy binding view", but it's built
        # in FIXUPP-record encounter order, i.e. raw order).
        offsets = [f["offset"] for f in om.fixups]
        raw_direction = classify_direction(offsets)
        predicted, reason = predict_direction(module)
        agrees_with_prediction = (predicted is None or raw_direction in ("none", "single")
                                   or raw_direction == predicted)
        if raw_direction in ("ascending", "descending") and predicted is not None:
            if raw_direction == predicted:
                agreements += 1
            else:
                disagreements += 1
        rows.append({
            "object": obj_path.name,
            "module": module["id"],
            "raw_fixup_count": len(offsets),
            "raw_direction": raw_direction,
            "predicted_direction": predicted,
            "agrees_with_prediction": agrees_with_prediction,
        })
    return {
        "found": True,
        "work_dir": str(work_dir.relative_to(ROOT)),
        "objects_checked": len(rows),
        "agreements": agreements,
        "disagreements": disagreements,
        "rows": rows,
    }


def compiler_path_description(tool, flags, sources) -> str:
    """Human-readable compiler path for a module's (tool, flags, sources)."""
    if tool == "TASM.EXE":
        return "TASM.EXE (hand-written/generated assembler module)"
    if tool == "TCC.EXE":
        if "-B" in flags:
            return "Turbo C -B (compiler restarts the unit through TASM)"
        for src in sources:
            if source_has_inline_asm(ROOT / src):
                return (f"Turbo C inline-asm restart (source {src} contains "
                         "an `asm` statement, restarting through TASM)")
        return "direct Turbo C (TCC.EXE, no -B, no inline asm)"
    return f"unrecognized tool {tool!r}"


def ledata_records(data: bytes) -> list[dict]:
    """Walk raw OMF records for LEDATA/LIDATA record boundaries.

    tools/omf.py's OmfReader.read merges LEDATA/LIDATA payloads into a single
    per-segment byte array and does not retain individual record boundaries,
    so this duplicates the minimal LNAMES/SEGDEF16/LEDATA16/LIDATA16 parsing
    needed to report each record's (segment, offset, length) in encounter
    order -- exactly what `inspect` needs to show and OmfReader cannot.
    """
    reader = OmfReader()
    lnames: list[str] = []
    segment_names: list[str] = []
    records = []
    for kind, body in reader.records(data):
        if kind == reader.LNAMES:
            at = 0
            while at < len(body):
                length = body[at]
                lnames.append(body[at + 1:at + 1 + length].decode("latin1"))
                at += 1 + length
        elif kind == reader.SEGDEF16:
            acbp = body[0]
            at = 1
            if (acbp >> 5) == 0:  # absolute segment: frame + offset follow
                at += 3
            at += 2  # segment length
            name_index, at = reader._index(body, at)
            name = (lnames[name_index - 1] if 0 < name_index <= len(lnames)
                    else f"?{name_index}")
            segment_names.append(name)
        elif kind in (reader.LEDATA16, reader.LIDATA16):
            segment_index, at = reader._index(body, 0)
            offset = struct.unpack_from("<H", body, at)[0]
            at += 2
            if kind == reader.LEDATA16:
                length = len(body) - at
            else:
                length = 0
                pos = at
                while pos < len(body):
                    chunk, pos = reader._expand_iterated_block(body, pos)
                    length += len(chunk)
            name = (segment_names[segment_index - 1]
                    if 0 < segment_index <= len(segment_names)
                    else f"?{segment_index}")
            records.append({
                "kind": "LEDATA" if kind == reader.LEDATA16 else "LIDATA",
                "segment": name, "offset": offset, "length": length,
            })
    return records


def fixup_thread_flags(data: bytes) -> list[dict]:
    """Raw FIXUPP16 subrecord scan, one entry per subrecord in encounter
    order, recording whether its frame/target used a predefined THREAD
    subrecord (F0 record) rather than an explicit method+index in the
    subrecord itself. tools/omf.py's reader resolves threads into concrete
    method/index on ObjectModule.fixups but does not retain whether a thread
    was used, so this duplicates the minimal byte walk to recover it.
    """
    reader = OmfReader()
    flags = []
    for kind, body in reader.records(data):
        if kind != reader.FIXUPP16:
            continue
        at = 0
        while at < len(body):
            if body[at] & 0x80:
                at += 2  # LOCAT (2 bytes)
                fixdat = body[at]
                at += 1
                frame_bit = bool(fixdat >> 7)
                frame_field = (fixdat >> 4) & 7
                target_bit = bool((fixdat >> 3) & 1)
                no_displacement = (fixdat >> 2) & 1
                if not frame_bit:
                    if frame_field in (0, 1, 2):
                        _, at = reader._index(body, at)
                    elif frame_field == 3:
                        at += 2
                if not target_bit:
                    _, at = reader._index(body, at)
                if not no_displacement:
                    at += 2
                flags.append({"frame_via_thread": frame_bit,
                              "target_via_thread": target_bit})
            else:
                thread = body[at]
                at += 1
                is_frame = (thread >> 6) & 1
                method = (thread >> 2) & 7
                if not (is_frame and method in (4, 5, 6)):
                    _, at = reader._index(body, at)
    return flags


def _load_plan_and_regions(root=ROOT):
    manifest = json.loads((root / "layout/manifest.json").read_text())
    plan = {m["id"]: m for m in json.loads((root / "layout/production-plan.json")
                                            .read_text())["modules"]}
    regions = {r["id"]: r for r in manifest["regions"]}
    return manifest, plan, regions


def compile_module_object(module_id, source_override=None, as_c_flags=None, root=ROOT):
    """Compile one plan module through the pinned toolchain and return its
    raw .OBJ bytes plus the resolved owner dict and staged source path.

    Mirrors tools/probe_module.probe's single-owner construction (whole-module
    concatenation for a shared multi-source module, --as-c override) since
    probe() itself only returns a byte-comparison verdict, not the compiled
    object -- `inspect` needs the object to report LEDATA/FIXUPP records.
    """
    sys.path.insert(0, str(root / "tools"))
    from reconstruct import compile_sources, read_json  # noqa: E402 (local: heavy deps)

    lock = read_json(root / "layout/toolchain.json")
    manifest, plan, regions = _load_plan_and_regions(root)
    members = {m: module["id"] for module in plan.values()
               for m in module.get("members", []) if module.get("sources")}
    concat = root / "build/probes/_shared"

    owner_id, override = module_id, source_override
    if owner_id in members or (owner_id in plan and plan[owner_id].get("sources")):
        module = plan[members.get(owner_id, owner_id)]
        sources = ([override] if (override and len(module["sources"]) == 1)
                   else module["sources"])
        concat.mkdir(parents=True, exist_ok=True)
        staged = concat / (module["id"] + ".C")
        staged.write_bytes(b"\r\n".join((root / src).read_bytes() for src in sources))
        owner_id, override = module["id"], staged.relative_to(root).as_posix()

    if owner_id not in plan and owner_id not in regions:
        raise SystemExit(f"unknown module {owner_id}")
    owner = dict(plan.get(owner_id) or regions[owner_id])
    if as_c_flags is not None:
        owner["kind"] = "MATCHING_C"
        owner["build"] = {**owner["build"], "flags_append": as_c_flags}
    if owner.get("sources"):
        bindings = dict(owner["build"].get("bindings", {}))
        for member in owner.get("members", []):
            for symbol, binding in regions.get(member, {}).get("build", {}) \
                    .get("bindings", {}).items():
                bindings.setdefault(symbol, binding)
        owner["build"] = {**owner["build"], "span": max(1, len(owner.get("publics", []))),
                          "bindings": bindings}
    compiled = {**owner, "source": override} if override else owner

    (root / "build").mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory(dir=root / "build") as temporary:
        work = Path(temporary)
        receipts, _ = compile_sources(root, [compiled], work, root / "toolchain", None, lock)
        obj_bytes = (work / receipts[owner["id"]]["object"]).read_bytes()
    return obj_bytes, owner, compiled["source"]


def inspect_module(module_id, source_override=None, as_c_flags=None, root=ROOT):
    """Compile `module_id` (optionally overriding its source and/or forcing a
    C candidate via `as_c_flags`) and report its compiler path, LEDATA
    records, FIXUPP subrecords (raw order and direction), the module's
    historical EXE relocations, and a verdict comparing the two.
    """
    sys.path.insert(0, str(root / "tools"))
    from reconstruct import read_object
    import probe_module

    manifest, plan, regions = _load_plan_and_regions(root)
    module = plan.get(module_id) or regions.get(module_id)
    if module is None:
        raise SystemExit(f"unknown module {module_id}")

    if as_c_flags is not None:
        sources = module.get("sources") or ([module["source"]] if "source" in module else [])
        if source_override:
            sources = [source_override]
        fake = {"tool": "TCC.EXE", "flags": as_c_flags.split(), "sources": sources}
        rule_predicted, rule_reason = predict_direction(fake)
        path = f"Turbo C candidate (--as-c {as_c_flags!r}); {rule_reason}"
    else:
        rule_predicted, rule_reason = predict_direction(module)
        path = compiler_path_description(module.get("tool"), module.get("flags", []),
                                          module.get("sources") or
                                          ([module["source"]] if "source" in module else []))

    obj_bytes, owner, compiled_source = compile_module_object(
        module_id, source_override, as_c_flags, root)
    om = read_object(obj_bytes)
    ledata = ledata_records(obj_bytes)
    thread_flags = fixup_thread_flags(obj_bytes)
    raw_fixups = []
    for fixup, threads in zip(om.fixups, thread_flags):
        raw_fixups.append({**fixup, **threads})
    for fixup, linker in zip(raw_fixups, om.linker_fixups):
        fixup["frame_kind"] = linker["frame_kind"]
        fixup["frame"] = linker["frame"]
    raw_offsets = [f["offset"] for f in raw_fixups]
    raw_direction = classify_direction(raw_offsets)

    exe_data = (root / "assets/AEPROG.EXE").read_bytes()
    mz = MZ.parse(exe_data)
    lo, hi = owner["start"] - 512, owner["end"] - 512
    historical = [r for r in mz.relocations if lo <= r["load_offset"] < hi]
    hist_offsets = [r["load_offset"] for r in historical]
    hist_direction = classify_direction(hist_offsets)

    # The probe's own byte comparison, for reference: bind_region resolves
    # every fixup to a concrete address, which needs binding metadata this
    # standalone probe of an alternate (e.g. --as-c) candidate may not carry
    # (undeclared externals) -- so it is reported but not used as the code
    # verdict. Reused instead: probe_module's own technique of masking
    # unresolved fixup-field bytes out of a *segment-level* comparison, run
    # here directly against the compiled unit's own code segment vs. the
    # historical region -- this is what actually answers "same code" without
    # requiring full-link address resolution.
    overrides = {module_id: source_override} if source_override else None
    as_c = {module_id: as_c_flags} if as_c_flags is not None else None
    probe_results = probe_module.probe([module_id], overrides, root=root, as_c=as_c)
    key = next(iter(probe_results))
    pr = probe_results[key]

    segment = owner.get("build", {}).get("segment", "_TEXT")
    try:
        emitted = om.segment_bytes(segment)
    except Exception:
        emitted = b""
    original_region = exe_data[owner["start"]:owner["end"]]
    masked = {f["offset"] + i for f in om.fixups_in(segment) for i in range(f["width"])}
    first_diff = next((i for i, (a, b) in enumerate(zip(original_region, emitted))
                       if a != b and i not in masked), None)
    if first_diff is None and len(original_region) == len(emitted):
        code_status = "EXACT"
    elif first_diff is None:
        code_status = f"LENGTH {len(emitted)} != {len(original_region)}"
    else:
        code_status = f"MISMATCH at +0x{first_diff:X}"
    code_exact = code_status == "EXACT"

    # The verdict follows the same rule the whole-plan audit applies (compare
    # the ROUTE's predicted direction, not the raw fresh-compile direction,
    # against the historical EXE run): a candidate can compile byte-exact
    # while still standing on a mis-predicted/mislabeled route -- that is
    # exactly what SAME_CODE_WRONG_TOPOLOGY is for. `raw_direction` (the
    # fresh object's own observed order) is reported alongside for direct
    # comparison, but the verdict itself mirrors `audit()`'s mismatch logic.
    if not code_exact:
        verdict = f"CODE_MISMATCH ({code_status})"
    elif not historical:
        verdict = "NO_RELOCATIONS"
    elif hist_direction in ("none", "single"):
        verdict = "CODE_AND_TOPOLOGY_MATCH"
    elif raw_direction in ("ascending", "descending"):
        # The fresh object's measured FIXUPP order is what TLINK will emit; the
        # route prediction is only a fallback when the order is unobservable.
        verdict = ("CODE_AND_TOPOLOGY_MATCH" if raw_direction == hist_direction
                   else "SAME_CODE_WRONG_TOPOLOGY")
    elif rule_predicted is None or rule_predicted == hist_direction:
        verdict = "CODE_AND_TOPOLOGY_MATCH"
    else:
        verdict = "SAME_CODE_WRONG_TOPOLOGY"

    return {
        "module": module_id,
        "resolved_owner": owner["id"],
        "compiled_source": compiled_source,
        "compiler_path": path,
        "rule_predicted_direction": rule_predicted,
        "rule_prediction_reason": rule_reason,
        "ledata_records": ledata,
        "fixupp_count": len(raw_fixups),
        "fixupp_subrecords": raw_fixups,
        "raw_fixup_offsets": raw_offsets,
        "raw_direction": raw_direction,
        "historical_relocations": historical,
        "historical_direction": hist_direction,
        "code_status": code_status,
        "probe_bind_status": pr["status"],
        "verdict": verdict,
    }


def render_inspect(result: dict) -> str:
    lines = []
    lines.append(f"# inspect {result['module']}")
    lines.append("")
    lines.append(f"Resolved owner: {result['resolved_owner']}")
    lines.append(f"Compiled source: {result['compiled_source']}")
    lines.append(f"Compiler path: {result['compiler_path']}")
    lines.append(f"Rule-predicted direction: {result['rule_predicted_direction']} "
                 f"({result['rule_prediction_reason']})")
    lines.append("")
    lines.append(f"LEDATA/LIDATA records ({len(result['ledata_records'])}):")
    for rec in result["ledata_records"]:
        lines.append(f"  {rec['kind']} {rec['segment']}+0x{rec['offset']:X} len={rec['length']}")
    lines.append("")
    lines.append(f"FIXUPP subrecords ({result['fixupp_count']}), raw encounter order:")
    for f in result["fixupp_subrecords"]:
        thread_note = []
        if f.get("frame_via_thread"):
            thread_note.append("frame-via-thread")
        if f.get("target_via_thread"):
            thread_note.append("target-via-thread")
        thread_note = f" [{', '.join(thread_note)}]" if thread_note else ""
        lines.append(
            f"  {f['segment']}+0x{f['offset']:X} loc={f['loc']} "
            f"target={f['target_kind']}:{f['target']} "
            f"frame={f.get('frame_kind')}:{f.get('frame')}{thread_note}")
    lines.append(f"  raw offsets: {result['raw_fixup_offsets']}")
    lines.append(f"  raw direction: {result['raw_direction']}")
    lines.append("")
    lines.append(f"Historical EXE relocations, table order ({len(result['historical_relocations'])}):")
    for r in result["historical_relocations"]:
        lines.append(f"  load_offset=0x{r['load_offset']:X}")
    lines.append(f"  historical direction: {result['historical_direction']}")
    lines.append("")
    lines.append(f"Own-segment masked byte comparison (code verdict basis): {result['code_status']}")
    lines.append(f"probe_module.probe bind_region status (reference only): {result['probe_bind_status']}")
    lines.append(f"VERDICT: {result['verdict']}")
    lines.append("")
    return "\n".join(lines)


def render_markdown(result: dict) -> str:
    lines = []
    lines.append("# Relocation Topology Audit")
    lines.append("")
    lines.append("## Rule")
    lines.append("")
    lines.append(result["rule"])
    lines.append("")
    lines.append(f"Total EXE relocations: {result['total_exe_relocations']}")
    lines.append("")
    lines.append("## Per-module table")
    lines.append("")
    lines.append("| Module | Tool | Flags | Relocs | Observed | Predicted | Mismatch |")
    lines.append("|---|---|---|---|---|---|---|")
    for row in result["modules"]:
        flags = " ".join(row["flags"]) if row["flags"] else ""
        lines.append(
            f"| {row['module']} | {row['tool']} | {flags} | "
            f"{row['relocation_count']} | {row['observed_direction']} | "
            f"{row['predicted_direction']} | {'YES' if row['mismatch'] else ''} |"
        )
    lines.append("")
    lines.append("## Generated and library objects")
    lines.append("")
    lines.append("| Object | Category | Predicted | Note |")
    lines.append("|---|---|---|---|")
    for row in result["generated_and_library_objects"]:
        lines.append(
            f"| {row['object']} | {row['category']} | {row['predicted_direction']} | "
            f"{row.get('note', row.get('prediction_reason',''))} |"
        )
    lines.append("")
    lines.append("## Mismatches")
    lines.append("")
    if not result["mismatches"]:
        lines.append("None.")
    else:
        for row in result["mismatches"]:
            lines.append(f"### {row['module']}")
            lines.append("")
            lines.append(f"- Tool/route: {row['tool']} {row['flags']} ({row['prediction_reason']})")
            lines.append(f"- Observed: {row['observed_direction']}; predicted: {row['predicted_direction']}")
            lines.append(f"- Offsets: {row['relocation_offsets']}")
            lines.append(f"- Implication: {row['implication']}")
            lines.append("")
    lines.append("## Fresh compile verification")
    lines.append("")
    fc = result["fresh_compile_verification"]
    if not fc["found"]:
        lines.append(fc["note"])
    else:
        lines.append(f"Checked against `{fc['work_dir']}` ({fc['objects_checked']} objects, "
                      f"{fc['agreements']} agree with prediction, {fc['disagreements']} disagree).")
        lines.append("")
        lines.append("| Object | Module | Raw fixups | Raw direction | Predicted | Agrees |")
        lines.append("|---|---|---|---|---|---|")
        for row in fc["rows"]:
            if "error" in row:
                lines.append(f"| {row['object']} | {row['module']} | ERROR | {row['error']} | | |")
                continue
            lines.append(
                f"| {row['object']} | {row['module']} | {row['raw_fixup_count']} | "
                f"{row['raw_direction']} | {row['predicted_direction']} | "
                f"{'yes' if row['agrees_with_prediction'] else 'NO'} |"
            )
    lines.append("")
    lines.append("## TASM modules with descending historical relocations")
    lines.append("")
    lines.append(
        "Every TASM-tooled module whose historical EXE relocations are "
        "descending -- the native-Turbo-C fingerprint -- regardless of "
        "whether it is already listed above as a mismatch, so re-C "
        "candidates stay visible:"
    )
    lines.append("")
    if not result["tasm_descending_candidates"]:
        lines.append("None.")
    else:
        lines.append("| Module | Relocations |")
        lines.append("|---|---|")
        for row in result["tasm_descending_candidates"]:
            lines.append(f"| {row['module']} | {row['relocation_count']} |")
    lines.append("")
    lines.append("## What mismatches imply for re-C work")
    lines.append("")
    lines.append(
        "A module flagged as a mismatch above is direct object-topology "
        "evidence about its ORIGINAL (pre-reconstruction) authorship, "
        "independent of any register-ABI or instruction-shape argument: "
        "descending relocations under a TASM/assembler-restart production "
        "route point to an original native Turbo C unit (a re-C candidate "
        "worth pursuing even if instruction-shape evidence looks ambiguous); "
        "ascending relocations under a plain-TCC route point to an original "
        "hand- or TASM-assembled unit (re-C should not be attempted, or should "
        "be treated with low confidence if it appears to succeed)."
    )
    lines.append("")
    return "\n".join(lines)


def cmd_inspect(argv):
    parser = argparse.ArgumentParser(
        prog="audit_relocation_topology.py inspect",
        description="Compile one plan module and report its compiler path, "
                    "LEDATA/FIXUPP records, and a code+topology verdict "
                    "against the historical EXE.")
    parser.add_argument("module_id")
    parser.add_argument("--source", help="override compiled source path (project-relative)")
    parser.add_argument("--as-c", nargs="?", const="", default=None, metavar="FLAGS",
                        help="compile as a C candidate for an assembler-owned "
                             "region (optional extra compiler flags, e.g. -B)")
    args = parser.parse_args(argv)
    result = inspect_module(args.module_id, source_override=args.source, as_c_flags=args.as_c)
    print(render_inspect(result))
    return 0


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    if argv and argv[0] == "inspect":
        return cmd_inspect(argv[1:])

    result = audit()
    OUT_JSON.write_text(json.dumps(result, indent=1))
    OUT_MD.write_text(render_markdown(result))
    print(f"Wrote {OUT_JSON}")
    print(f"Wrote {OUT_MD}")
    print(f"Modules: {len(result['modules'])}, mismatches: {len(result['mismatches'])}")
    for row in result["mismatches"]:
        print(f"  MISMATCH {row['module']}: observed={row['observed_direction']} "
              f"predicted={row['predicted_direction']}")
    print(f"TASM modules with descending historical relocations "
          f"(native-Turbo-C fingerprint): "
          f"{[row['module'] for row in result['tasm_descending_candidates']]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
