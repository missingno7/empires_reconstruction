import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from reconstruct import mismatch, read_json


class MatchingCWave108DataTests(unittest.TestCase):
    def test_help_strings_round_trip(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave108.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        self.assertEqual(len(recipe['owners']), 9)
        self.assertEqual(sum(owner['end'] - owner['start'] for owner in recipe['owners']), 250)
        for candidate in recipe['owners']:
            owner = next(r for r in manifest['regions'] if r['id'] == candidate['id'])
            self.assertEqual(owner, candidate)
            data = encode_data(read_json(ROOT / owner['source']), owner['build']['encoder'])
            mismatch(original[owner['start']:owner['end']], data, owner)


if __name__ == '__main__':
    unittest.main()
