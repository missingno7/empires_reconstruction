import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader
from omf_scaffold import make_text_padding, remove_public, trim_text_contribution, make_dgroup_scaffold, _records


class OmfScaffoldTests(unittest.TestCase):
    def test_large_dgroup_records_preserve_all_bytes_and_publics(self):
        data = bytes(range(256)) * 55
        publics = {'_data_%04d' % i: ('_DATA', i * 17) for i in range(400)}
        blob = make_dgroup_scaffold(data, 37000, publics)
        self.assertTrue(all(len(body) <= 1003 for _, body in _records(blob)))
        module = OmfReader().read(blob)
        self.assertEqual(module.segment_bytes('_DATA'), data)
        self.assertEqual(module.segment_length('_BSS'), 37000)
        self.assertEqual({p['name']: (p['segment'], p['offset']) for p in module.publics}, publics)

    def test_padding_is_a_relocatable_text_contribution(self):
        module = OmfReader().read(make_text_padding(7), 'PAD.OBJ')
        self.assertEqual(module.segment_length('_TEXT'), 7)
        self.assertEqual(module.segment_bytes('_TEXT'), b'\0' * 7)

    def test_trim_preserves_prefix_and_fixup_free_extent(self):
        candidates = sorted((ROOT / 'build').glob('tlink-structural-*/compile/WORK/R0009.OBJ'))
        if not candidates:
            self.skipTest('structural TLINK probe has not produced a compiler object')
        source = candidates[-1].read_bytes()
        trimmed = trim_text_contribution(source, 6571)
        module = OmfReader().read(trimmed, 'RUNTIME_BLOCK.OBJ')
        self.assertEqual(module.segment_length('_TEXT'), 6571)
        self.assertEqual(module.segment_bytes('_TEXT'), OmfReader().read(source).segment_bytes('_TEXT')[:6571])

    def test_remove_public_preserves_object_bytes(self):
        candidates = sorted((ROOT / 'build').glob('tlink-structural-*/WORK/R0209.OBJ'))
        candidates = [path for path in candidates
                      if '_getkey' in {public['name'] for public in OmfReader().read(path.read_bytes()).publics}]
        if not candidates:
            self.skipTest('structural TLINK probe has not produced the duplicate-public object')
        source = candidates[-1].read_bytes()
        original = OmfReader().read(source)
        trimmed = OmfReader().read(remove_public(source, '_getkey'))
        self.assertEqual(trimmed.segment_bytes('_TEXT'), original.segment_bytes('_TEXT'))
        self.assertEqual(trimmed.fixups, original.fixups)
        self.assertNotIn('_getkey', {public['name'] for public in trimmed.publics})


if __name__ == '__main__':
    unittest.main()
