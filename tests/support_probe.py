"""Shared helpers that express wave-test intent through tools/probe_module.py.

The historical wave tests compiled sources through their own private copies of
the compile/bind logic and matched "before"/"after" mutant snippets against
exact whitespace in the current source files. That makes every reformatting
or rename of a source force re-anchoring dozens of tests.

These helpers express the same two intents against the single canonical
prover (tools/probe_module.py `probe()`) instead:
  - `exact(owner_id)`: the current source reproduces the original bytes.
  - `mutant_rejected(owner_id, before, after)`: a specific semantic mutant
    (matched whitespace-insensitively against the owner's current source) is
    rejected by the prover.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from probe_module import probe  # noqa: E402
from reconstruct import read_json  # noqa: E402

_PROBE_DIR = ROOT / 'build/probes/tests'


def _owner_source(owner_id, root=ROOT):
    manifest = read_json(root / 'layout/manifest.json')
    region = next((r for r in manifest['regions'] if r['id'] == owner_id), None)
    if region is None:
        plan = read_json(root / 'layout/production-plan.json')['modules']
        module = plan.get(owner_id) if isinstance(plan, dict) else next(
            (m for m in plan if m['id'] == owner_id), None)
        if module is None:
            raise SystemExit(f'unknown owner {owner_id}')
        return module['sources'][0]
    return region['source']


def _normalize(text):
    """Collapse runs of whitespace to single spaces, keeping a position map
    back to the original text so a match in the normalized text can be
    mapped back to the original span."""
    normalized_chars = []
    index_map = []  # index_map[i] = offset into original text of normalized_chars[i]
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c.isspace():
            j = i
            while j < n and text[j].isspace():
                j += 1
            normalized_chars.append(' ')
            index_map.append(i)
            i = j
        else:
            normalized_chars.append(c)
            index_map.append(i)
            i += 1
    index_map.append(n)  # sentinel: end of text
    return ''.join(normalized_chars), index_map


def _result_key(owner_id, root=ROOT):
    """A member of a grouped multi-source module is compiled and reported as
    the whole module (probe_module.probe follows layout/production-plan.json
    the same way); resolve which key the results dict will use."""
    plan = read_json(root / 'layout/production-plan.json')['modules']
    plan_map = {m['id']: m for m in plan}
    members = {m: module['id'] for module in plan_map.values()
              for m in module.get('members', []) if module.get('sources')}
    return members.get(owner_id, owner_id)


def exact(owner_id, overrides=None, root=ROOT):
    """Assert the prover reports the owner's current source as byte-exact."""
    results = probe([owner_id], overrides, root=root)
    key = _result_key(owner_id, root=root)
    status = results[key]['status']
    assert status == 'EXACT', f'{owner_id}: expected EXACT, got {status!r}'


def mutant_rejected(owner_id, before, after, root=ROOT):
    """Locate `before` in the owner's current source (whitespace-insensitive),
    write the `after` mutant under build/probes/tests/, and assert the prover
    reports a mismatch (i.e. anything other than EXACT) for it."""
    if isinstance(before, bytes):
        before = before.decode('latin-1')
    if isinstance(after, bytes):
        after = after.decode('latin-1')

    source_path = root / _owner_source(owner_id, root=root)
    original_text = source_path.read_text(encoding='latin-1')

    normalized_source, index_map = _normalize(original_text)
    normalized_before, _ = _normalize(before)

    matches = [m for m in re.finditer(re.escape(normalized_before), normalized_source)]
    assert len(matches) == 1, (
        f'{owner_id}: expected exactly one whitespace-insensitive match for the '
        f'mutant "before" snippet, found {len(matches)}')
    match = matches[0]
    start = index_map[match.start()]
    end = index_map[match.end()]

    mutated_text = original_text[:start] + after + original_text[end:]

    _PROBE_DIR.mkdir(parents=True, exist_ok=True)
    mutant_path = _PROBE_DIR / f'{owner_id}_MUTANT.C'
    mutant_path.write_text(mutated_text, encoding='latin-1')

    results = probe([owner_id], {owner_id: mutant_path.relative_to(root).as_posix()}, root=root)
    key = _result_key(owner_id, root=root)
    status = results[key]['status']
    assert status != 'EXACT', f'{owner_id}: mutant unexpectedly reported EXACT'
