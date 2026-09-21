"""Opt-out-free integration coverage for the canonical structural EXE build."""
import os
import re
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from build_exe import ORIGINAL_SHA256, build, validate_toolchain


def local_linker_is_available():
    try:
        validate_toolchain(ROOT)
        sys.path.insert(0, str(ROOT / 'tools'))
        from dos_runner import resolve_runner
        from reconstruct import read_json
        resolve_runner(read_json(ROOT / 'layout/toolchain.json'))
    except (OSError, ValueError, KeyError):
        return False
    return True


class CleanStructuralExeBuildTests(unittest.TestCase):
    @unittest.skipUnless(local_linker_is_available(), 'local pinned Borland toolchain/DOSBox is unavailable')
    def test_fresh_construction_does_not_open_original_fixture(self):
        fixture = (ROOT / 'assets/AEPROG.EXE').resolve()
        read_bytes = Path.read_bytes

        def reject_fixture(path):
            if path.resolve() == fixture:
                raise AssertionError('fixture read during fixture-free construction')
            return read_bytes(path)

        # The original remains on disk for normal verification, but this guard
        # proves the construction path itself neither reads nor copies it.
        with patch.object(Path, 'read_bytes', reject_fixture):
            report = build(ROOT, verify=False)
        self.assertEqual(report['status'], 'BUILT')
        self.assertEqual(report['sha256'], ORIGINAL_SHA256)
        self.assertEqual(report['relocations'], 106)
        self.assertFalse(report['verification']['performed'])
        self.assertEqual(report['verification']['reason'], 'verification not requested')

    @unittest.skipUnless(local_linker_is_available(), 'local pinned Borland toolchain/DOSBox is unavailable')
    def test_fresh_source_to_tlink_build_is_exact(self):
        # ``build`` removes this stale receipt before it creates every new
        # compiler/linker session. It proves no earlier probe report can be
        # accepted as the build result.
        stale = ROOT / 'build/exe-build-report.json'
        stale.parent.mkdir(exist_ok=True)
        stale.write_text('{"stale": true}\n')
        with patch('omf_scaffold.trim_text_contribution', side_effect=AssertionError('compiler output must not be trimmed')), patch('omf_scaffold.externalize_data_segment', side_effect=AssertionError('native DATA must not be externalized')):
            report = build(ROOT, verify=True)
        self.assertEqual(report['status'], 'BUILT')
        self.assertEqual(report['sha256'], ORIGINAL_SHA256)
        self.assertEqual(report['unresolved_symbols'], 0)
        self.assertEqual(report['link_invocations'], 1)
        self.assertTrue(report['fresh_build'])
        self.assertEqual(report['mode'], 'ACCEPTANCE')
        self.assertEqual(report['relocations'], 106)
        self.assertTrue(report['verification']['byte_identical'])
        self.assertEqual(report['text_extent_policy'], 'CHECK_ONLY_NO_TRIMMING')
        self.assertEqual(report['pre_link_corrections']['text_trimming'], [])
        self.assertEqual(report['pre_link_corrections']['library_segment_replacements'], [])
        from omf import OmfReader
        native_rand = (Path(report['session']) / 'compile' / report['compiled_objects']['LIB_RAND']).read_bytes()
        packaged = OmfReader().split_library((Path(report['session']) / 'TC/LIB/CC.LIB').read_bytes())
        self.assertEqual(sum(blob == native_rand for _, blob in packaged), 1)
        plan = __import__('json').loads((ROOT / 'layout/production-plan.json').read_text())
        self.assertEqual(report['untouched_compiler_objects'], len(plan['modules']))
        self.assertEqual(report['synthetic_code_padding_objects'], 0)
        self.assertEqual(len(report['natural_code_alignment']), 4)
        transformed = report['compiler_object_transformations']
        self.assertEqual(transformed, [])
        self.assertEqual(report['pre_link_corrections']['data_externalization'], [])
        separated = {x['owner'] for x in report['pre_link_corrections']['data_externalization']}
        self.assertNotIn('F_56C6', separated)
        self.assertNotIn('F_A658', separated)
        self.assertNotIn('F_A28D', separated)
        self.assertNotIn('F_D5BA', separated)
        plan = __import__('json').loads((ROOT / 'layout/production-plan.json').read_text())
        for ident in ('F_A28D', 'F_D5BA', 'F_9D8E', 'C_75F3_7856'):
            raw = Path(report['session']) / 'compile' / report['compiled_objects'][ident]
            module = next(m for m in plan['modules'] if m['id'] == ident)
            staged = Path(report['session']) / 'WORK' / module['object']
            self.assertEqual(raw.read_bytes(), staged.read_bytes(), ident + ' object was rewritten')
        from omf import OmfReader
        for ident, symbol in [('F_56C6', '_g139d'), ('F_A658', '_menu_empty_record')]:
            # Shared record storage stays external even when the member is compiled
            # inside a grouped module.
            owner = next(m['id'] for m in plan['modules'] if ident in m['members'])
            obj = Path(report['session']) / 'compile' / report['compiled_objects'][owner]
            parsed = OmfReader().read(obj.read_bytes())
            self.assertEqual(parsed.segment_length('_DATA'), 0)
            self.assertIn(symbol, parsed.externals)
        shared_path = Path(report['session']) / 'compile' / report['compiled_objects']['C_75F3_7856']
        shared = OmfReader().read(shared_path.read_bytes())
        self.assertEqual(shared.segment_bytes('_DATA')[:7], bytes([4, 0, 4, 0, 0, 0, 0]))
        self.assertEqual(shared.segment_length('_DATA'), 50)
        pubs = {p['name']: p['offset'] for p in shared.publics_in('_DATA')}
        self.assertEqual({k:pubs[k] for k in ('_gb80','_energy_meter','_gb83','_gb85')},
                         {'_gb80':0,'_energy_meter':2,'_gb83':3,'_gb85':5})
        dialog_path = Path(report['session']) / 'compile' / report['compiled_objects']['F_9D8E']
        dialog = OmfReader().read(dialog_path.read_bytes())
        self.assertEqual(dialog.segment_length('_DATA'), 229)
        pubs = {p['name']: p['offset'] for p in dialog.publics_in('_DATA')}
        self.assertEqual(pubs['_text118c'], 0)
        self.assertEqual(pubs['_g125d'], 209)
        regions = {r['id']: r for r in __import__('json').loads((ROOT / 'layout/manifest.json').read_text())['regions']}

        def recovered_member(ident):
            # A recovered function may be compiled inside a grouped module; its own
            # extent and source come from the manifest region, the tool from the plan.
            module = next(m for m in plan['modules'] if m['id'] == ident or ident in m['members'])
            region = regions[ident] if module['id'] != ident else module
            source = region.get('source') or module['source']
            text = (ROOT / source).read_text()
            if len(module['members']) > 1:
                # Only this member's section of the merged module file.
                banner = re.compile(r'/\* ---- ' + re.escape(ident) + r' \(original code at 0x[0-9A-F]+\) ---- \*/')
                match = banner.search(text)
                if match:
                    following = re.search(r'/\* ---- [A-Z0-9_]+ \(original code at', text[match.end():])
                    text = text[match.start():match.end() + following.start()] if following else text[match.start():]
            return {**region, 'tool': module['tool'], 'flags': module['flags'], 'source': source, 'text': text}

        recovered = recovered_member('F_AB66')
        self.assertEqual(recovered['tool'], 'TCC.EXE')
        self.assertEqual(recovered['source'], 'src/SLOTMENU.C')  # F_AB66 is a section of the slot-menu module
        self.assertEqual(recovered['end'] - recovered['start'], 385)
        self.assertNotRegex(recovered['text'], r'(?im)^\s*asm\b')
        selection = recovered_member('F_880A')
        self.assertEqual(selection['tool'], 'TCC.EXE')
        self.assertEqual(selection['end'] - selection['start'], 557)
        self.assertNotIn('F_8C04', {m['id'] for m in plan['modules']})
        self.assertNotRegex(selection['text'], r'(?im)^\s*asm\b')
        menu = recovered_member('F_7964')
        self.assertEqual(menu['tool'], 'TCC.EXE')
        self.assertEqual(menu['end'] - menu['start'], 664)
        self.assertNotIn('F_7DD3', {m['id'] for m in plan['modules']})
        self.assertNotRegex(menu['text'], r'(?im)^\s*asm\b')
        tables = recovered_member('F_B40F')
        self.assertEqual(tables['tool'], 'TCC.EXE')
        self.assertEqual(tables['end'] - tables['start'], 236)
        self.assertNotRegex(tables['text'], r'(?im)^\s*asm\b')
        state = recovered_member('F_AA1F')
        self.assertEqual(state['tool'], 'TCC.EXE')
        self.assertEqual(state['end'] - state['start'], 327)
        self.assertNotRegex(state['text'], r'(?im)^\s*asm\b')
        board = recovered_member('F_B7F9')
        self.assertEqual(board['tool'], 'TCC.EXE')
        self.assertEqual(board['end'] - board['start'], 366)
        self.assertNotRegex(board['text'], r'(?im)^\s*asm\b')
        parser = recovered_member('F_4F96')
        self.assertEqual(parser['tool'], 'TCC.EXE')
        self.assertIn('-B', parser['flags'])
        self.assertEqual(parser['end'] - parser['start'], 299)
        self.assertNotRegex(parser['text'], r'(?im)^\s*asm\b')
        for ident, size in [('F_B122', 693), ('F_28AC', 218), ('F_25B3', 761), ('F_8BAB', 1275), ('F_B99F', 1857), ('M_988F_98CB', 121), ('M_DAD7_DB35', 137), ('M_CB5C_CD23', 641), ('F_699E', 380), ('F_643A', 240), ('F_652A', 66), ('F_4F63', 51)]:
            recovered = recovered_member(ident)
            self.assertEqual(recovered['tool'], 'TCC.EXE')
            self.assertEqual(recovered['end'] - recovered['start'], size)
            self.assertNotRegex(recovered['text'], r'(?im)^\s*asm\b')
        self.assertNotIn('stale', stale.read_text())
        self.assertTrue((ROOT / 'build/AEPROG.EXE').exists())


if __name__ == '__main__':
    unittest.main()
