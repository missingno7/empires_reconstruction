import tempfile
import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader
from reconstruct import compile_sources, read_json, read_object, sha


class MatchingCWave147Tests(unittest.TestCase):
    def test_rand_source_matches_pinned_library_object(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave147.json')
        evidence = read_json(ROOT / 'docs/matching-wave147-evidence.json')
        owner = recipe['owners'][0]
        current = next(item for item in manifest['regions'] if item['id'] == owner['id'])
        self.assertEqual(current['kind'], 'MATCHING_C')
        self.assertEqual(current['source'], 'src/LIB_RAND.C')
        self.assertEqual(evidence['owners'][0]['fixups'], 8)
        library = dict(OmfReader().split_library((ROOT / 'toolchain/CC.LIB').read_bytes()))['RAND']
        library_module = OmfReader().read(library, 'RAND')
        self.assertEqual(library_module.segment_length('_TEXT'), 57)
        self.assertEqual(library_module.segment_length('_DATA'), 4)
        self.assertEqual(library_module.segment_length('_BSS'), 0)
        self.assertEqual(library_module.publics,
                         [{'name': '_srand', 'segment': '_TEXT', 'offset': 0},
                          {'name': '_rand', 'segment': '_TEXT', 'offset': 17}])
        self.assertEqual(library_module.externals, ['LXMUL@'])
        self.assertEqual(len(library_module.fixups), 8)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            lock = read_json(ROOT / 'layout/toolchain.json')
            receipts, _ = compile_sources(ROOT, [current], Path(temporary), ROOT / 'toolchain',
                                           Path(lock['dosbox_default']), lock)
            candidate = read_object((Path(temporary) / receipts['LIB_RAND']['object']).read_bytes())
            for segment in ('_TEXT', '_DATA', '_BSS'):
                if segment in library_module.segments:
                    self.assertEqual(candidate.segment_bytes(segment), library_module.segment_bytes(segment))
                self.assertEqual(candidate.segment_length(segment) or 0,
                                 library_module.segment_length(segment) or 0)
            self.assertEqual(candidate.publics, library_module.publics)
            self.assertEqual(candidate.externals, library_module.externals)
            self.assertEqual(candidate.fixups, library_module.fixups)


if __name__ == '__main__':
    unittest.main()
