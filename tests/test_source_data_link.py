from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from data_omf import emit_data
from omf import OmfReader
from omf_scaffold import externalize_data_segment
from reconstruct import read_json


class SourceDataLinkTests(unittest.TestCase):
    def test_fixup_crossing_chunk_boundary(self):
        data = bytes(2100)
        obj = emit_data(data, {'source': 0}, [{'offset': 998, 'target': 'target'}])
        module = OmfReader().read(obj)
        self.assertEqual(module.segment_bytes('_DATA'), data)
        self.assertEqual(module.fixups_in('_DATA')[0]['offset'], 998)
        self.assertEqual(module.fixups_in('_DATA')[0]['target'], 'target')
        with self.assertRaisesRegex(ValueError, 'Overlapping'):
            emit_data(data, {}, [{'offset': 998, 'target': 'a'}, {'offset': 999, 'target': 'b'}])

    def test_externalization_preserves_all_seven_real_code_contributions(self):
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
        self.assertEqual(count, 7)

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
        self.assertEqual(report['synthetic_bss_bytes'], 37250)
        self.assertEqual(report['oracle_copied_initialized_data_bytes'], 0)
        reloc = report['byte_comparison']['mz']
        self.assertEqual(reloc['candidate_fields']['e_crlc'], 72)
        self.assertEqual(reloc['extra_sites'], [])
        self.assertEqual(len(reloc['missing_sites']), 34)


if __name__ == '__main__':
    unittest.main()
