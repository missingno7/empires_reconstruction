import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from mz import MZ
from reconstruct import read_json


class RawDataWave145Tests(unittest.TestCase):
    def test_level_complete_text_is_relocation_free_and_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave145.json')
        evidence = read_json(ROOT / 'docs/matching-wave145-evidence.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        mz = MZ.parse(original)
        owner = recipe['owners'][0]
        current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
        self.assertEqual(current['kind'], 'EXACT_DATA')
        encoded = encode_data(read_json(ROOT / owner['source']), 'ascii-v1')
        self.assertEqual(encoded, original[owner['start']:owner['end']])
        self.assertEqual(len(encoded), evidence['owners'][0]['bytes'])
        load_start = owner['start'] - 512
        load_end = owner['end'] - 512
        self.assertFalse(any(load_start <= r['load_offset'] < load_end for r in mz.relocations))
        prefix = next(r for r in manifest['regions'] if r['id'] == 'RAW_01129F_CONTROL_PREFIX')
        self.assertEqual(prefix['end'], owner['start'])


if __name__ == '__main__':
    unittest.main()
