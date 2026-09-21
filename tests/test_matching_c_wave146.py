import unittest
import tempfile
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader
from dos_runner import resolve_runner
from reconstruct import compile_sources, read_json, read_object, sha


class MatchingCWave146Tests(unittest.TestCase):
    def test_strlen_source_matches_pinned_library_object(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave146.json')
        evidence = read_json(ROOT / 'docs/matching-wave146-evidence.json')
        owner = recipe['owners'][0]
        current = next(item for item in manifest['regions'] if item['id'] == owner['id'])
        self.assertEqual(current['kind'], 'KNOWN_TOOLCHAIN_LIBRARY')
        self.assertEqual(current['source'], 'toolchain/CC.LIB')
        self.assertEqual(current['build']['library_module'], 'STRLEN')
        self.assertEqual(current['build']['module_sha256'],
                         '1d7eea610c2ec8e95322277394fee72c44a323df8941cb8ddc9ce6f45f3dfe00')
        self.assertEqual(evidence['owners'][0]['fixups'], 0)
        library = dict(OmfReader().split_library((ROOT / 'toolchain/CC.LIB').read_bytes()))['STRLEN']
        library_module = OmfReader().read(library, 'STRLEN')
        library_text = library_module.segment_bytes('_TEXT')
        self.assertEqual(library_text, bytes.fromhex(
            '55 8b ec 56 57 fc c4 7e 04 32 c0 b9 ff ff f2 ae '
            '8b c1 f7 d0 48 eb 00 5f 5e 5d c3'))
        self.assertEqual(sha(library_text), current['expected_sha256'])
        self.assertEqual(library_module.publics, [{'name': '_strlen', 'segment': '_TEXT', 'offset': 0}])
        self.assertEqual(library_module.fixups, [])
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            lock = read_json(ROOT / 'layout/toolchain.json')
            receipts, _ = compile_sources(ROOT, [owner], Path(temporary), ROOT / 'toolchain',
                                           resolve_runner(lock), lock)
            candidate = read_object((Path(temporary) / receipts['LIB_STRLEN']['object']).read_bytes())
            self.assertEqual(candidate.segment_bytes('_TEXT'), library_text)
            self.assertEqual(candidate.segment_length('_TEXT'), 27)
            self.assertEqual(candidate.publics, library_module.publics)
            self.assertEqual(candidate.fixups, [])


if __name__ == '__main__':
    unittest.main()
