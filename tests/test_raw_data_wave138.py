import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json
from exe_data import encode_data


class RawDataWave138Tests(unittest.TestCase):
    def test_record_block_and_terminal_pad_are_exact(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave138.json')
        evidence = read_json(ROOT / 'docs/matching-wave138-evidence.json')
        self.assertEqual(len(recipe['owners']), 2)
        self.assertEqual(sum(o['end'] - o['start'] for o in recipe['owners']), 187)
        for owner in recipe['owners']:
            source = read_json(ROOT / owner['source'])
            self.assertEqual(encode_data(source, owner['build']['encoder']),
                             (ROOT / 'assets/AEPROG.EXE').read_bytes()[owner['start']:owner['end']])
            current = next(r for r in manifest['regions'] if r['id'] == owner['id'])
            self.assertEqual(current['kind'], 'EXACT_DATA')
            item = next(i for i in evidence['owners'] if i['id'] == owner['id'])
            self.assertEqual(item['status'], 'EQUAL')


if __name__ == '__main__':
    unittest.main()
