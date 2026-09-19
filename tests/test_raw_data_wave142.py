import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json
from exe_data import encode_data


class RawDataWave142Tests(unittest.TestCase):
    def test_player_name_instruction_is_one_exact_terminated_record(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave142.json')
        evidence = read_json(ROOT / 'docs/matching-wave142-evidence.json')
        owner = recipe['owners'][0]
        current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
        self.assertEqual(current['kind'], 'EXACT_DATA')
        self.assertEqual(current['classification'], 'dialog_text')
        source = read_json(ROOT / owner['source'])
        encoded = encode_data(source, owner['build']['encoder'])
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        self.assertEqual(encoded, original[owner['start']:owner['end']])
        self.assertEqual(len(encoded), 51)
        self.assertEqual(encoded[-1], 0)
        self.assertNotIn(0, encoded[:-1])
        self.assertEqual(evidence['owners'][0]['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
