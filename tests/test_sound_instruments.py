from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from reconstruct import read_json
from sound_instruments import compile_sound_instruments, decode_sound_instruments


class SoundInstrumentTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.original = (ROOT / 'raw/012C03-013386.bin').read_bytes()
        cls.source = read_json(ROOT / 'src/data/DATA_012C03_SOUND_INSTRUMENTS.json')

    def test_source_round_trips_original_extent(self):
        self.assertEqual(decode_sound_instruments(self.original), self.source)
        self.assertEqual(compile_sound_instruments(self.source), self.original)

    def test_verified_layout(self):
        self.assertEqual(len(self.source['slot_glyph_pairs_tail']), 17)
        self.assertEqual(len(self.source['voice_operator_offsets']), 18)
        self.assertEqual(len(self.source['voice_disabled']), 18)
        self.assertEqual(len(self.source['voice_connection']), 18)
        self.assertEqual(len(self.source['instrument_records']), 33)
        self.assertTrue(all(len(record) == 28 for record in self.source['instrument_records']))
        self.assertEqual(self.source['terminator_words'], [-1, -1])

    def test_record_fields_preserve_signed_values(self):
        values = [value for record in self.source['instrument_records'] for value in record]
        self.assertIn(-10, values)
        self.assertIn(-1, values)


if __name__ == '__main__':
    unittest.main()
