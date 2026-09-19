import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from omf import OmfReader
from omf_scaffold import make_text_padding, trim_text_contribution


class OmfScaffoldTests(unittest.TestCase):
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


if __name__ == '__main__':
    unittest.main()
