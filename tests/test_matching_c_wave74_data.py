"""Fresh proof for the recovered DATA_01BA self-referential word."""
from pathlib import Path
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from reconstruct import read_json, mismatch
class MatchingCWave74DataTests(unittest.TestCase):
    def test_recovered_word(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave74.json')
        owner = next(r for r in manifest['regions'] if r['id'] == 'DATA_01BA')
        self.assertEqual(next(r for r in recipe['owners'] if r['id'] == 'DATA_01BA'), owner)
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        data = encode_data(read_json(ROOT / owner['source']), owner['build']['encoder'])
        mismatch(original[owner['start']:owner['end']], data, owner)
        self.assertEqual(data, bytes.fromhex('00 00'))
if __name__ == '__main__': unittest.main()
