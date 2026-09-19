import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]


class SharedDataModuleTests(unittest.TestCase):
    def test_fresh_shared_compilation_and_full_link(self):
        path = ROOT / 'build/shared-module-link-report.json'
        if not path.exists():
            self.skipTest('local shared-module TLINK probe not run')
        report = json.loads(path.read_text())
        self.assertEqual(report['status'], 'CODE_PLACEMENT_EQUAL')
        self.assertEqual(report['code_bytes'], 611)
        self.assertEqual(report['objects_replaced'], 5)
        self.assertEqual(report['objects_added'], 1)
        self.assertEqual(report['data_evidence']['bytes'], 43)
        self.assertEqual(report['errors'], [])
        self.assertEqual(report['downstream_code_divergences'], [])
        self.assertFalse(report['historical_module_proven'])
        text = next(s for s in report['segments'] if s['name'] == '_TEXT')
        self.assertEqual((text['start'], text['length']), (0, 0xFA23))
        # The retained synthetic DATA sizing is deliberately not padded to hide
        # the removed inter-object alignment.
        self.assertFalse(report['byte_comparison']['load_image']['equal'])


if __name__ == '__main__':
    unittest.main()
