import json
import unittest
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import read_json


class B99FSymbolicCallInventoryTests(unittest.TestCase):
    def test_inventory_matches_direct_call_sites_and_owner_extents(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        inventory = read_json(ROOT / 'docs/f_b99f-symbolic-call-inventory.json')
        owner = next(item for item in manifest['regions'] if item['id'] == inventory['owner'])
        image = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        load_start = owner['start'] - 512
        regions = manifest['regions']
        # The historical inventory (docs/, not editable here) records target
        # publics by their pre-rename address-based names. Translate through
        # docs/current/symbol-names.json (new name -> {original}) so the
        # comparison tracks the 254 symbol renames from this session.
        symbol_names = read_json(ROOT / 'docs/current/symbol-names.json')['names']
        old_to_new_public = {'_' + info['original']: '_' + new
                             for new, info in symbol_names.items()}
        self.assertEqual(inventory['text_bytes'], owner['end'] - owner['start'])
        self.assertEqual(inventory['omf_fixups'], 0)
        self.assertEqual(len(inventory['calls']), 55)
        for call in inventory['calls']:
            with self.subTest(offset=call['offset']):
                at = owner['start'] + call['offset']
                self.assertEqual(image[at], 0xe8)
                relative = int.from_bytes(image[at + 1:at + 3], 'little', signed=True)
                target = (load_start + call['offset'] + 3 + relative) & 0xffff
                self.assertEqual(target, call['target_load_offset'])
                target_owner = next(item for item in regions
                                    if item['start'] - 512 <= target < item['end'] - 512)
                self.assertEqual(target_owner['id'], call['target_owner'])
                expected_public = old_to_new_public.get(call['target_public'], call['target_public'])
                self.assertEqual(target_owner.get('build', {}).get('public'), expected_public)


if __name__ == '__main__':
    unittest.main()
