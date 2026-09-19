import sys
from pathlib import Path
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from audit_tlink_data import contributions, audit


class DataAuditTests(unittest.TestCase):
    def test_non_normalized_frame_and_empty_segment(self):
        rows = contributions('1333:0002 0000 C=BSS S=_BSS G=DGROUP M=A ACBP=48\n'
                             '0FA3:0094 000F C=DATA S=_DATA G=DGROUP M=R0001.C ACBP=48')
        self.assertEqual(rows[0]['load_start'], 0x13332)
        self.assertEqual(rows[0]['length'], 0)
        self.assertEqual(rows[1]['load_start'], 0xFAC4)

    def test_missing_relocation_at_owner_boundary(self):
        manifest = {'regions': [{'id': 'A', 'start': 512, 'end': 516, 'kind': 'RAW'},
                                {'id': 'B', 'start': 516, 'end': 520, 'kind': 'RAW'}]}
        report = {'relocatable_scaffold': [], 'outputs': {'exe_sha256': 'test'},
                  'byte_comparison': {'mz': {'missing_sites': [4]}}}
        result = audit(manifest, report, '')
        self.assertEqual([o['owner'] for o in result['missing_relocation_owners']], ['B'])


if __name__ == '__main__':
    unittest.main()
