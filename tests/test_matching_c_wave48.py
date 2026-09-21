"""Fresh proof for F_9D8E and its relocated initialized C data segment."""
import struct
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from exe_data import encode_data
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import (read_json, compile_sources, read_object, bind_region,
                         mismatch, owned_library_modules)


class MatchingCWave48Tests(unittest.TestCase):
    def test_fresh_code_data_text_and_relocation(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        recipe = read_json(ROOT / 'recipes/c/matching-wave48.json')
        code = next(o for o in recipe['owners'] if o['id'] == 'F_9D8E')
        text = next(o for o in recipe['owners'] if o['id'] == 'TEXT_118C')
        data_owner = next(o for o in recipe['owners'] if o['id'] == 'DATA_125D')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        mz = MZ.parse(original)
        lock = read_json(ROOT / 'layout/toolchain.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            receipts, _ = compile_sources(ROOT, [code], Path(temporary), ROOT / 'toolchain',
                                          resolve_runner(lock), lock)
            module = read_object((Path(temporary) / receipts[code['id']]['object']).read_bytes())
            modules[code['id']] = module
            code_bytes, code_proof = bind_region(code, module, mz,
                                                 manifest['frames'], manifest['regions'], modules)
            mismatch(original[code['start']:code['end']], code_bytes, code)
            self.assertEqual(len(code_bytes), 62)
            self.assertEqual(len(code_proof['fixups']), 5)
            text_bytes = encode_data(read_json(ROOT / text['source']), text['build']['encoder'])
            mismatch(original[text['start']:text['end']], text_bytes, text)

            # F_9D8E now declares text118c[] as a local initializer that
            # shares one 229-byte compiled _DATA segment with g125d, rather
            # than the historical split where DATA_125D alone was the C
            # module's entire _DATA segment (tools/reconstruct.compiled_data
            # requires an owner's declared extent to equal the whole
            # segment, and only supports 'external' pointer32 fixups, not
            # this self-segment one). Bind the combined segment directly:
            # the far-pointer fixup inside g125d addresses text118c at the
            # start of the same segment (TEXT_118C's DGROUP offset).
            part = bytearray(module.segment_bytes('_DATA'))
            self.assertEqual(len(part), (text['end'] - text['start']) + (data_owner['end'] - data_owner['start']))
            fixups = module.fixups_in('_DATA')
            self.assertEqual(len(fixups), 1)
            fixup = fixups[0]
            self.assertEqual((fixup['loc'], fixup['width'], fixup['target_kind'], fixup['target']),
                             ('pointer32', 4, 'segment', '_DATA'))
            offset = fixup['offset']
            addend = int.from_bytes(part[offset:offset + 2], 'little')
            segment_addend = int.from_bytes(part[offset + 2:offset + 4], 'little')
            dgroup_offset = text['start'] - 512 - manifest['frames']['DGROUP']
            value = (dgroup_offset + fixup['displacement'] + addend) & 0xFFFF
            segment_value = (manifest['frames']['DGROUP'] // 16 + segment_addend) & 0xFFFF
            part[offset:offset + 4] = struct.pack('<HH', value, segment_value)
            mismatch(original[text['start']:data_owner['end']], bytes(part), data_owner)
            self.assertEqual(bytes(part[:len(text_bytes)]), text_bytes)
            data_bytes = bytes(part[len(text_bytes):])
            mismatch(original[data_owner['start']:data_owner['end']], data_bytes, data_owner)
            self.assertEqual(len(data_bytes), 20)
            relocation = mz.load_offset(text['start']) + offset + 2
            self.assertEqual(relocation, 68758)


if __name__ == '__main__':
    unittest.main()
