import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import ZERO_PAD_FORMAT, decode_data, encode_data


class ZeroPadDataTests(unittest.TestCase):
    def test_round_trip(self):
        source = {'format': ZERO_PAD_FORMAT, 'length': 7}
        data = encode_data(source, ZERO_PAD_FORMAT)
        self.assertEqual(data, b'\0' * 7)
        self.assertEqual(decode_data(data, ZERO_PAD_FORMAT), source)

    def test_rejects_nonzero(self):
        with self.assertRaises(ValueError):
            decode_data(b'\0\1', ZERO_PAD_FORMAT)


if __name__ == '__main__':
    unittest.main()
