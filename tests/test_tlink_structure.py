import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class TlinkStructureTests(unittest.TestCase):
    def test_latest_probe_records_natural_prefix_and_first_divergence(self):
        path = ROOT / 'build/tlink-structural-report.json'
        if not path.exists():
            self.skipTest('local TLINK structural probe has not been run')
        report = read_json(path)
        self.assertEqual(report['status'], 'MAP_AVAILABLE')
        comparison = report['code_comparison']
        self.assertGreaterEqual(comparison['actual_code_row_count'], 339)
        if report.get('mode', '').startswith('library_toupper'):
            self.assertIsNone(comparison['first_divergence'])
            replacements = report.get('library_replacements', [])
            for owner_id in ('LIB_STRLEN', 'LIB_RAND'):
                replacement = next((item for item in replacements if item['owner'] == owner_id), None)
                if replacement is not None:
                    self.assertEqual(replacement['expected_start'], replacement['actual']['offset'])
                    self.assertEqual(replacement['expected_length'], replacement['actual']['length'])
            toupper = report['library_comparison']['actual_toupper']
            self.assertIsNotNone(toupper)
            self.assertEqual(toupper['length'], report['library_comparison']['expected_length'])
            self.assertEqual(toupper['offset'], report['library_comparison']['expected_start'])
            self.assertEqual(report['segments'][0]['length'], 0xFA23)
            self.assertEqual(report['segments'][1]['start'], 0xFA30)
            if (report.get('options', {}).get('scaffold_dgroup') or
                    report.get('mode', '').endswith('_and_dgroup_scaffold')):
                stack = next(segment for segment in report['segments']
                             if segment['name'] == '_STACK')
                self.assertEqual(stack['start'], 0x1C500)
                self.assertEqual(report['link']['unresolved_count'], 0)
                self.assertEqual(report['link'].get('errors', []), [])
        else:
            divergence = comparison['first_divergence']
            if report.get('mode') == 'library_toupper_promotion':
                self.assertIsNone(divergence)
                self.assertNotEqual(report['library_comparison']['actual_toupper']['offset'],
                                    report['library_comparison']['expected_start'])
            else:
                self.assertEqual(divergence['owner'], 'F_F9BE')
                self.assertNotEqual(divergence['expected_start'], divergence['actual_start'])
        self.assertTrue(any(item['kind'] == 'alignment_padding'
                            for item in report['relocatable_scaffold']))
        byte_comparison = report.get('byte_comparison', {})
        if byte_comparison.get('available'):
            self.assertIn('mz', byte_comparison)
            self.assertIn('load_image', byte_comparison)
            self.assertIn('full_file', byte_comparison)
            self.assertIn('first_difference', byte_comparison['full_file'])


if __name__ == '__main__':
    unittest.main()
