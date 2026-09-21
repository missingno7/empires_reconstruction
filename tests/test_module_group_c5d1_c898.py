"""Fresh shared-compilation proof around the symbolic F_C755 boundary.

C_C5D1_C706 and C_C77A_C898 (recovery/recipes/modules/*.json) are C-candidate
recipes for members of the sound driver 0xC3A0..0xCD5C. That whole driver is
now proven to be one hand-written TASM module, asm/SOUND.ASM
(layout/production-plan.json module M_C1A0_CB48; see
docs/current/asm-provenance.json for why Turbo C cannot reproduce it: every
framed routine uses SI/DI without saving them). Because production ownership
moved from these individual C sources to asm/SOUND.ASM,
tools/probe_module_group.py's own metadata guard now (correctly) refuses
these recipes with 'Recipe differs from established source/flags': the
manifest regions' 'source' field no longer names each recipe's own C file.
That guard exists to catch stale probes, and this is a real, expected
divergence, not a bug to route around it for.

What still holds, and is what this test now checks directly (bypassing
probe_module_group.probe(), whose extra manifest-consistency check no longer
applies here): the retired C candidates, compiled together exactly as before,
still reproduce the original executable's bytes for every member extent.
That is an audit fact about the source text and the original binary, not a
claim about current production ownership.
"""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (bind_region, compile_sources, mismatch, owned_library_modules,
                         project_path, read_json, read_object)


def probe_retired_c_candidate(recipe_path):
    """Reduced, manifest-ownership-agnostic version of probe_module_group.probe()."""
    recipe = read_json(recipe_path)
    manifest = read_json(ROOT / 'layout/manifest.json')
    original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
    lock = read_json(ROOT / 'layout/toolchain.json')
    modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
    owners_by_id = {o['id']: o for o in manifest['regions']}
    selected = [owners_by_id[s['owner']] for s in recipe['sources']]
    assert all(a['end'] == b['start'] for a, b in zip(selected, selected[1:])), 'owners must be contiguous'

    def source_bytes(rel_path):
        path = project_path(ROOT, rel_path)
        if not path.exists():
            path = project_path(ROOT, 'recovery/' + rel_path)
        return path.read_bytes()

    combined = b'\r\n'.join(source_bytes(s['path']) for s in recipe['sources'])
    with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
        work = Path(temporary)
        (work / 'combined.C').write_bytes(combined)
        candidate = {'id': recipe['id'], 'kind': 'MATCHING_C',
                     'source': (work / 'combined.C').relative_to(ROOT).as_posix(),
                     'build': {'flags_append': recipe['flags_append']}}
        receipts, _ = compile_sources(ROOT, [candidate], work, ROOT / 'toolchain',
                                      resolve_runner(lock), lock)
        module = read_object((work / receipts[recipe['id']]['object']).read_bytes())
    publics = module.publics_in(recipe['segment'])
    expected_names = [s['public'] for s in recipe['sources']]
    assert [p['name'] for p in publics] == expected_names, 'combined module public order differs'
    text_bytes = module.segment_length(recipe['segment'])
    assert text_bytes == selected[-1]['end'] - selected[0]['start'], 'combined module emitted extent size differs'
    mz = MZ.parse(original)
    fixups_checked = 0
    for owner in selected:
        data, proof = bind_region(owner, module, mz, manifest['frames'], manifest['regions'], modules)
        mismatch(original[owner['start']:owner['end']], data, owner)
        fixups_checked += len(proof['fixups'])
    return {'status': 'EQUAL', 'source_units_combined': len(selected),
           'text_bytes': text_bytes, 'fixups_checked': fixups_checked}


class C5D1C898ModuleGroupTests(unittest.TestCase):
    def test_c_prefix_shares_one_exact_object(self):
        report = probe_retired_c_candidate(ROOT / 'recovery/recipes/modules/C_C5D1_C706.json')
        self.assertEqual(report['status'], 'EQUAL')
        self.assertEqual((report['source_units_combined'], report['text_bytes'],
                          report['fixups_checked']), (4, 388, 0))

    def test_c_suffix_shares_one_exact_object(self):
        report = probe_retired_c_candidate(ROOT / 'recovery/recipes/modules/C_C77A_C898.json')
        self.assertEqual(report['status'], 'EQUAL')
        self.assertEqual((report['source_units_combined'], report['text_bytes'],
                          report['fixups_checked']), (5, 346, 13))


if __name__ == '__main__':
    unittest.main()
