import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json
from exe_data import encode_data


class RawDataWave143Tests(unittest.TestCase):
    def test_dialog_prefix_is_separate_from_unresolved_tail(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave143.json')
        evidence = read_json(ROOT / 'docs/matching-wave143-evidence.json')
        owner = recipe['owners'][0]
        current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
        self.assertEqual(current['kind'], 'EXACT_DATA')
        encoded = encode_data(read_json(ROOT / owner['source']), 'ascii-nul-v1')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        self.assertEqual(encoded, original[owner['start']:owner['end']])
        self.assertEqual(len(encoded), 99)
        tail = next(r for r in manifest['regions'] if r['id'] == 'DATA_01075A_FILE_ERROR_CONTROL')
        self.assertEqual(tail['kind'], 'EXACT_DATA')
        self.assertEqual(tail['end'] - tail['start'], 20)
        self.assertEqual(evidence['raw_tail']['bytes'], 20)


if __name__ == '__main__':
    unittest.main()
