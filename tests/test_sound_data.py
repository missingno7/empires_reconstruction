from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from data_omf import emit_data
from omf import OmfReader
from reconstruct import read_json
from sound_data import bind_sound_data, compile_sound_data, decode_sound_data


class SoundDataTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.manifest = read_json(ROOT / 'layout/manifest.json')
        cls.owner = next(r for r in cls.manifest['regions']
                         if r['id'] == 'DATA_01139E_SOUND')
        cls.document = read_json(ROOT / cls.owner['source'])
        cls.original = (ROOT / 'raw/01139E-011AC6.bin').read_bytes()
        cls.base = cls.owner['start'] - 512 - cls.manifest['frames']['DGROUP']

    def test_source_round_trips_original_extent(self):
        self.assertEqual(decode_sound_data(self.original, self.base), self.document)
        bound = bind_sound_data(
            self.document,
            lambda owner: (self.base, len(self.original)) if owner == self.owner['id'] else None)
        self.assertEqual(bound, self.original)

    def test_near_pointers_are_relocatable_omf_fixups(self):
        data, refs, publics = compile_sound_data(self.document)
        self.assertEqual(len(data), 1832)
        self.assertEqual(len(refs), 38)
        self.assertEqual({ref['loc'] for ref in refs}, {'offset16'})
        self.assertEqual(data[0xbe:0xc2], bytes(4))
        self.assertEqual(data[0xc4:0x10c], bytes(72))
        module = OmfReader().read(emit_data(data, publics, refs, 'SOUND'))
        fixups = module.fixups_in('_DATA')
        self.assertEqual(len(fixups), 38)
        self.assertEqual({fixup['loc'] for fixup in fixups}, {'offset16'})
        self.assertEqual({fixup['target_kind'] for fixup in fixups}, {'external'})

    def test_structural_boundaries_are_stable(self):
        self.assertEqual(len(self.document['state_words']), 71)
        self.assertEqual(len(self.document['note_divisors']), 24)
        self.assertEqual(self.document['opl_port'], 0x388)
        self.assertEqual(len(self.document['dispatch']), 36)
        self.assertEqual(len(self.document['lookup_prefix']), 13)
        self.assertEqual(len(self.document['tail_words']), 9)


if __name__ == '__main__':
    unittest.main()
