"""An alias only closes a historical blocker for the same owned code extent."""
import contextlib
import copy
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from inventory_linkage import capture


class LinkageInventoryTests(unittest.TestCase):
    def test_exact_extent_alias_and_rejected_near_matches(self):
        with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
            root = Path(temporary)
            upstream = root / 'upstream'
            source = upstream / 'controls/correspondence'
            source.mkdir(parents=True)
            (root / 'layout').mkdir()
            (root / 'docs').mkdir()
            row = {'id': 'OLD', 'extent_bytes': 49, 'reason': 'PROVIDER_REFUSED',
                   'undecided': [{'symbol': '__ctype', 'offset': 23}]}
            (source / 'held-by-provider.json').write_text(json.dumps({'held': [row]}))
            (source / 'OLD.json').write_text(json.dumps({'extent': {
                'file_offset': 100, 'length': 49, 'sha256': 'expected'}}))
            owner = {'id': 'NEW', 'kind': 'MATCHING_C', 'start': 100, 'end': 149,
                     'expected_sha256': 'expected', 'matching_status': 'EQUAL'}
            def check(candidate):
                (root / 'layout/manifest.json').write_text(json.dumps({'regions': [candidate]}))
                with contextlib.redirect_stdout(io.StringIO()):
                    return capture(upstream, root)
            report = check(owner)
            self.assertEqual(report['held_candidates'], 0)
            self.assertEqual(report['resolved_extent_aliases'][0]['owner'], 'NEW')
            for change in ({'start': 99}, {'end': 150}, {'expected_sha256': 'different'},
                           {'matching_status': 'DIFFERS'}, {'kind': 'RAW'}, {'kind': 'EXACT_DATA'}):
                report = check(dict(owner, **change))
                self.assertEqual(report['held_candidates'], 1)
                self.assertEqual(report['unresolved_fixup_sites'], 1)
                self.assertEqual(report['resolved_extent_aliases'], [])
