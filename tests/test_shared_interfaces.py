"""Shared record/dialog headers stay the single declaration of their layouts."""
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from interface_census import declarations
from reconstruct import read_json


def active_sources():
    plan = read_json(ROOT / 'layout/production-plan.json')
    return sorted({s for m in plan['modules'] if m['tool'] == 'TCC.EXE'
                   for s in ([m['source']] if 'source' in m else m.get('sources', []))})


class SharedHeaderTests(unittest.TestCase):
    def layout(self, header, tag):
        _, _, records, _ = declarations((ROOT / 'include' / header).read_text(), 'include/' + header)
        return next(r for r in records if r['name'] == tag)

    def test_record_and_dialog_layouts(self):
        record = self.layout('C470.H', 'c470_record')
        self.assertEqual(record['bytes'], 27)
        offsets = {f['name']: f['offset'] for f in record['fields']}
        self.assertEqual({k: offsets[k] for k in ('value', 'flags', 'sound', 'music', 'option', 'pending', 'state', 'round_progress')},
                         {'value': 9, 'flags': 11, 'sound': 13, 'music': 15, 'option': 17, 'pending': 19, 'state': 21, 'round_progress': 22})
        dialog = self.layout('DIALOG.H', 'dialog')
        self.assertEqual(dialog['bytes'], 20)
        offsets = {f['name']: f['offset'] for f in dialog['fields']}
        self.assertEqual({k: offsets[k] for k in ('title', 'sub', 'text', 'initial', 'cx', 'lines')},
                         {'title': 2, 'sub': 6, 'text': 7, 'initial': 11, 'cx': 12, 'lines': 18})

    def test_active_sources_do_not_redeclare_shared_layouts(self):
        for source in active_sources():
            text = (ROOT / source).read_text()
            for symbol in ('slot_table', 'slot_transfer_table'):
                if re.search(r'\b%s\b' % symbol, text):
                    self.assertIn('#include "C470.H"', text, source + ' uses the record table without the shared header')
            for symbol in ('dialog_draw', 'dialog_run', 'dialog_layout'):
                if re.search(r'\b%s\s*\(' % symbol, text):
                    self.assertIn('#include "DIALOG.H"', text, source + ' calls a dialog routine without the shared header')
        self.assertFalse((ROOT / 'include/RECORD27.H').exists())


if __name__ == '__main__':
    unittest.main()
