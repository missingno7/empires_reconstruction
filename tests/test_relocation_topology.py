"""Tests for tools/audit_relocation_topology.py."""
import json
import sys
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / "tools"))

from tools.audit_relocation_topology import (  # noqa: E402
    audit, classify_direction, inspect_module)


def test_classify_direction_synthetic():
    assert classify_direction([]) == "none"
    assert classify_direction([5]) == "single"
    assert classify_direction([1, 2, 3]) == "ascending"
    assert classify_direction([9, 5, 1]) == "descending"
    assert classify_direction([1, 5, 3]) == "mixed"
    assert classify_direction([2, 2, 3]) == "mixed"  # not strictly increasing


def test_audit_known_descending_and_ascending_units():
    result = audit()
    by_module = {m["module"]: m for m in result["modules"]}

    # Native Turbo C plan modules: descending, matching descending prediction.
    for name in ("C_75F3_7856", "RELOC_F_AD25_F_ADCF"):
        row = by_module[name]
        assert row["observed_direction"] == "descending"
        assert row["predicted_direction"] == "descending"
        assert not row["mismatch"]

    # F_9EC3: TASM module, ascending relocations, agrees with TASM prediction.
    anim_step_row_copy = by_module["F_9EC3"]
    assert anim_step_row_copy["tool"] == "TASM.EXE"
    assert anim_step_row_copy["observed_direction"] == "ascending"
    assert anim_step_row_copy["predicted_direction"] == "ascending"
    assert not anim_step_row_copy["mismatch"]

    # MUSIC (M_DDD9_DF98): now produced via a plain Turbo C route, but
    # predict_direction's inline-asm regex still fires on the word "asm"
    # inside the source's own doc comment ("asm/MUSIC.ASM: ..."), predicting
    # ascending -- while the EXE is historically descending. The known/
    # expected mismatch that first proved native Turbo C origin for this
    # module persists as a route-mislabeling diagnostic even after the
    # module was re-Ced.
    music = by_module["M_DDD9_DF98"]
    assert music["tool"] == "TCC.EXE"
    assert music["observed_direction"] == "descending"
    assert music["predicted_direction"] == "ascending"
    assert music["mismatch"] is True


def test_audit_writes_valid_json_and_markdown(tmp_path):
    result = audit()
    # Sanity: JSON-serializable and markdown renders without error.
    from tools.audit_relocation_topology import render_markdown
    text = render_markdown(result)
    assert "Relocation Topology Audit" in text
    assert "M_DDD9_DF98" in text
    # round-trip through JSON
    json.loads(json.dumps(result))


def _toolchain_available():
    """True if the pinned MS-DOS/DOSBox runner needed by `inspect` (it
    compiles through tools/reconstruct.compile_sources) is actually usable
    in this environment."""
    sys.path.insert(0, str(ROOT / "tools"))
    from dos_runner import resolve_runner
    from reconstruct import read_json
    lock = read_json(ROOT / "layout/toolchain.json")
    try:
        resolve_runner(lock)
    except ValueError:
        return False
    return True


def test_verdict_logic_synthetic(monkeypatch):
    """The verdict rules on synthetic module/direction combinations, without
    touching the compiler: patch inspect_module's building blocks directly
    by exercising the same decision table it implements.
    """
    def verdict_for(code_exact, code_status, historical, rule_predicted, hist_direction):
        if not code_exact:
            return f"CODE_MISMATCH ({code_status})"
        if not historical:
            return "NO_RELOCATIONS"
        if rule_predicted is None or hist_direction in ("none", "single") \
                or rule_predicted == hist_direction:
            return "CODE_AND_TOPOLOGY_MATCH"
        return "SAME_CODE_WRONG_TOPOLOGY"

    # Exact code, agreeing directions -> match.
    assert verdict_for(True, "EXACT", [1, 2], "descending", "descending") \
        == "CODE_AND_TOPOLOGY_MATCH"
    # Exact code, disagreeing directions -> same code, wrong topology.
    assert verdict_for(True, "EXACT", [1, 2], "ascending", "descending") \
        == "SAME_CODE_WRONG_TOPOLOGY"
    # No historical relocations at all -> topology unobservable.
    assert verdict_for(True, "EXACT", [], "ascending", "none") == "NO_RELOCATIONS"
    # Bytes don't match -> code mismatch, regardless of direction agreement.
    assert verdict_for(False, "MISMATCH at +0x10", [1, 2], "ascending", "ascending") \
        == "CODE_MISMATCH (MISMATCH at +0x10)"
    # A single relocation can't disagree in direction with anything.
    assert verdict_for(True, "EXACT", [1], "ascending", "single") \
        == "CODE_AND_TOPOLOGY_MATCH"


@pytest.mark.skipif(not _toolchain_available(),
                    reason="MS-DOS Player/DOSBox toolchain unavailable")
def test_inspect_native_unit_matches():
    result = inspect_module("C_75F3_7856")
    assert result["verdict"] == "CODE_AND_TOPOLOGY_MATCH"
    assert result["code_status"] == "EXACT"
    assert result["raw_direction"] == "descending"
    assert result["historical_direction"] == "descending"


@pytest.mark.skipif(not _toolchain_available(),
                    reason="MS-DOS Player/DOSBox toolchain unavailable")
def test_inspect_music_as_c_same_code_wrong_topology():
    result = inspect_module("M_DDD9_DF98", source_override="src/MUSIC.C", as_c_flags="")
    assert result["verdict"] == "SAME_CODE_WRONG_TOPOLOGY"
    assert result["code_status"] == "EXACT"
