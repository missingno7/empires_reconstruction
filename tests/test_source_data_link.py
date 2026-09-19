from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from data_omf import emit_data
from bss_asm import bss_asm_source, bss_slice
from omf import OmfReader
from omf_scaffold import externalize_data_segment
from reconstruct import read_json


class SourceDataLinkTests(unittest.TestCase):
    def test_bss_asm_source_preserves_aliases_and_extent(self):
        source = bss_asm_source(8, {'A': 0, 'B': 0, 'C': 3})
        self.assertIn('A label byte\r\n', source)
        self.assertIn('B label byte\r\n', source)
        self.assertIn('db 3 dup (?)\r\n', source)
        self.assertTrue(source.endswith('db 5 dup (?)\r\n_BSS ends\r\nend\r\n'))

    def test_bss_slice_rebases_canonical_publics(self):
        layout = {'format': 'anchored-bss-layout-v1', 'length': 12,
                  'publics': {'START': 0, 'A': 2, 'B': 5, 'END': 11}}
        self.assertEqual(bss_slice(layout, 2, 6),
                         {'length': 4, 'publics': {'A': 0, 'B': 3}})
        with self.assertRaisesRegex(ValueError, 'extent'):
            bss_slice(layout, 4, 4)

    def test_bss_contribution_plan_covers_canonical_layout(self):
        layout = read_json(ROOT / 'src/data/GAME_BSS.json')
        plan = read_json(ROOT / 'recipes/data/bss-contributions.json')
        self.assertEqual(plan['canonical_layout'], 'src/data/GAME_BSS.json')
        expected_start = 0
        actual_publics = {}
        for contribution in plan['contributions']:
            self.assertEqual(contribution['logical_start'], expected_start)
            sliced = bss_slice(layout, contribution['logical_start'], contribution['logical_end'])
            for name, offset in sliced['publics'].items():
                actual_publics[name] = contribution['logical_start'] + offset
            expected_start = contribution['logical_end']
        self.assertEqual(expected_start, layout['length'])
        self.assertEqual(actual_publics, layout['publics'])

    def test_fixup_crossing_chunk_boundary(self):
        data = bytes(2100)
        obj = emit_data(data, {'source': 0}, [{'offset': 998, 'target': 'target'}])
        module = OmfReader().read(obj)
        self.assertEqual(module.segment_bytes('_DATA'), data)
        self.assertEqual(module.fixups_in('_DATA')[0]['offset'], 998)
        self.assertEqual(module.fixups_in('_DATA')[0]['target'], 'target')
        with self.assertRaisesRegex(ValueError, 'Overlapping'):
            emit_data(data, {}, [{'offset': 998, 'target': 'a'}, {'offset': 999, 'target': 'b'}])

    def test_externalization_preserves_all_real_code_contributions(self):
        path = ROOT / 'build/tlink-structural-report.json'
        if not path.exists():
            self.skipTest('local TLINK probe has not run')
        report = read_json(path)
        work = Path(report['byte_comparison']['candidate']).parent
        count = 0
        for entry in report['relocatable_scaffold']:
            if entry['kind'] != 'owner':
                continue
            data = (work / entry['object']).read_bytes()
            module = OmfReader().read(data)
            if not module.segment_length('_DATA'):
                continue
            result = OmfReader().read(externalize_data_segment(data, '__SOURCE'))
            self.assertEqual(result.segment_bytes('_TEXT'), module.segment_bytes('_TEXT'))
            self.assertEqual(result.segment_length('_DATA'), 0)
            self.assertEqual(len(result.fixups_in('_TEXT')), len(module.fixups_in('_TEXT')))
            count += 1
        self.assertEqual(count, 8)

    def test_full_linked_initialized_data(self):
        path = ROOT / 'build/source-data-link-report.json'
        if not path.exists():
            self.skipTest('local source DATA probe has not run')
        report = read_json(path)
        self.assertEqual(report['status'], 'LINKED')
        self.assertEqual(report['errors'], [])
        self.assertTrue(report['code_contributions_equal'])
        self.assertTrue(report['byte_comparison']['initialized_data']['equal'])
        self.assertTrue(report['byte_comparison']['load_image']['equal'])
        self.assertTrue(report['byte_comparison']['text']['equal'])
        self.assertEqual(report['synthetic_bss_bytes'], 0)
        self.assertEqual(report['partitioned_bss_source_bytes'], 14492)
        self.assertEqual(report['unpartitioned_bss_source_bytes'], 22758)
        self.assertFalse(report['dgroup_scaffold_present'])
        self.assertEqual(report['bss_source']['kind'], 'TASM_SOURCE_CONTRIBUTIONS')
        self.assertTrue(report['bss_source']['binding_evidence_equal'])
        self.assertEqual(report['bss_source']['publics'], 247)
        self.assertEqual(report['bss_source']['initialized_bytes'], 0)
        self.assertEqual(report['bss_source']['group'], 'DGROUP')
        self.assertTrue(report['bss_source']['contribution_plan_equal'])
        self.assertEqual([(item['id'], item['bytes']) for item in report['bss_source']['contributions']],
                         [('F_01CE_BSS_PREFIX', 34), ('ROW_POINTER_TABLE_BSS', 1952),
                          ('RENDER_STATE_BSS', 16), ('RESOURCE25_WORKSPACE_BSS', 672),
                          ('B4374_CONTROL_STATE_BSS', 64), ('G43B4_RECORD_TABLE_BSS', 10000),
                          ('GAMEBSS_LEADING_PREFIX', 1126), ('G6F2A_RECORD_TABLE_BSS', 904),
                          ('GAMEBSS_LEADING_REMAINDER', 20580),
                          ('SLOT_GRID_BSS', 48), ('GAMEBSS_MID_REMAINDER', 8),
                          ('ANIMATION_STATE_BSS', 18), ('GC360_RECORD_TABLE_BSS', 270),
                          ('GAMEBSS_ERR_REMAINDER', 2),
                          ('C470_RECORD_TABLE_BSS', 270), ('GAMEBSS_REMAINDER_PREFIX', 52),
                          ('GC5B2_FLAGS_BSS', 8), ('GAMEBSS_SOUND_PREFIX', 48),
                          ('NOTE_OCTAVE_TABLE_BSS', 96), ('NOTE_INDEX_TABLE_BSS', 96),
                          ('GAMEBSS_VOICE_PREFIX', 890), ('VOICE_POINTER_TABLE_BSS', 44),
                          ('GAMEBSS_REMAINDER', 52)])
        self.assertEqual(report['oracle_copied_initialized_data_bytes'], 0)
        reloc = report['byte_comparison']['mz']
        self.assertEqual(reloc['candidate_fields']['e_crlc'], 106)
        self.assertEqual(reloc['extra_sites'], [])
        self.assertEqual(len(reloc['missing_sites']), 0)
        self.assertTrue(reloc['fields_equal'])
        self.assertTrue(reloc['relocation_pairs_equal'])
        self.assertEqual(report['byte_comparison']['full_file']['differing_bytes_outside_relocation_table'], 0)


if __name__ == '__main__':
    unittest.main()
