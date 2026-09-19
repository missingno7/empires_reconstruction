from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from data_omf import emit_data
from omf import OmfReader
from reconstruct import read_json
from typed_data import bind_typed_data, compile_typed_data


class TypedDataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = read_json(ROOT / 'layout/manifest.json')
        cls.frame = cls.manifest['frames']['DGROUP']
        cls.by_id = {owner['id']: owner for owner in cls.manifest['regions']}
        cls.owners = [owner for owner in cls.manifest['regions']
                      if owner.get('build', {}).get('encoder') == 'typed-data-v1']

    def resolve(self, target):
        if target == 'GAME_BSS':
            return max(owner['end'] for owner in self.manifest['regions']) - 512 - self.frame, self.frame // 16
        owner = self.by_id[target]
        return owner['start'] - 512 - self.frame, self.frame // 16

    def test_all_typed_sources_rebuild_their_original_extents(self):
        self.assertEqual(len(self.owners), 8)
        for owner in self.owners:
            with self.subTest(owner=owner['id']):
                source = read_json(ROOT / owner['source'])
                actual = bind_typed_data(source, self.resolve)
                original = (ROOT / 'assets/AEPROG.EXE').read_bytes()[owner['start']:owner['end']]
                self.assertEqual(actual, original)

    def test_symbolic_fields_become_real_omf_fixups(self):
        expected = {
            'DATA_01075A_FILE_ERROR_CONTROL': 1,
            'DATA_010924_MENU_DESCRIPTORS': 20,
            'DATA_01129F_LEVEL_CONTROL': 1,
            'DATA_011F25_KEYBOARD_CONTROL': 1,
            'DATA_011F86_USER_CONTROL': 1,
        }
        for owner in self.owners:
            source = read_json(ROOT / owner['source'])
            data, refs, publics = compile_typed_data(source)
            module = OmfReader().read(emit_data(data, publics, refs, owner['id']))
            self.assertEqual(len(module.fixups_in('_DATA')), expected.get(owner['id'], 0))
            self.assertEqual({fixup['loc'] for fixup in module.fixups_in('_DATA')},
                             {'pointer32'} if owner['id'] in expected else set())

    def test_no_canonical_raw_owner_remains(self):
        self.assertEqual([owner['id'] for owner in self.manifest['regions']
                          if owner['kind'] == 'RAW'], [])


if __name__ == '__main__':
    unittest.main()
