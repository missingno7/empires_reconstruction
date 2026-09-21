"""Generated DOS toolchain inputs are canonical CRLF text, and the historical
commands run unchanged through the default MS-DOS Player runner.

Turbo C, TASM and TLINK receive every text file through reconstruct.dos_text
(latin-1, explicit CRLF, no DOS EOF byte), exactly as the DOSBox batch path
staged them. The compile path itself is the historical one: Turbo C performs
its own TASM handoff for -B and inline-asm units; nothing is split, rewritten
or delayed by the driver.
"""
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from dos_runner import resolve_runner
from probe_runner_parity import normalize_volatile_omf_comments
from reconstruct import compile_sources, dos_text, read_json, sha


def msdos_player():
    try:
        runner = resolve_runner(read_json(ROOT / 'layout/toolchain.json'), backend='msdos-player')
    except ValueError:
        return None
    return runner if (ROOT / 'toolchain/TASM.EXE').exists() else None


class DosTextTests(unittest.TestCase):
    def test_dos_text_is_deterministic_crlf(self):
        self.assertEqual(dos_text(b'a\nb\r\nc\rd\n'), b'a\r\nb\r\nc\r\nd\r\n')
        self.assertEqual(dos_text('x\n'), b'x\r\n')
        self.assertEqual(dos_text(b'\xe9\n'), b'\xe9\r\n')
        self.assertEqual(dos_text(dos_text(b'a\nb\n')), b'a\r\nb\r\n')
        self.assertNotIn(b'\x1a', dos_text(b'end\n'))

    @unittest.skipUnless(msdos_player(), 'MS-DOS Player and the pinned toolchain are required')
    def test_staged_inputs_are_crlf_and_historical_commands_are_stable(self):
        runner = msdos_player()
        lock = read_json(ROOT / 'layout/toolchain.json')
        plan = read_json(ROOT / 'layout/production-plan.json')
        # The largest -B unit: Turbo C hands its assembly to TASM itself.
        module = max((m for m in plan['modules'] if m['tool'] == 'TCC.EXE' and 'source' in m
                      and '-B' in m['flags']), key=lambda m: m['end'] - m['start'])
        objects = set()
        for _ in range(3):
            with tempfile.TemporaryDirectory(dir=ROOT / 'build') as temporary:
                work = Path(temporary)
                receipts, _ = compile_sources(ROOT, [module], work, ROOT / 'toolchain', runner, lock)
                self.assertEqual(receipts[module['id']]['command'].split()[-2:], ['-B', 'R0000.C'])
                staged = [p for p in (work / 'WORK').iterdir() if p.suffix.upper() in ('.C', '.H', '.ASM')]
                self.assertTrue(staged)
                for path in staged:
                    data = path.read_bytes()
                    self.assertEqual(data, dos_text(data), f'{path.name} is not canonical CRLF text')
                # TASM stamps the source time into an OMF comment; erase only that field.
                objects.add(sha(normalize_volatile_omf_comments((work / receipts[module['id']]['object']).read_bytes())))
        self.assertEqual(len(objects), 1, 'the historical -B handoff must be deterministic on this runner')


if __name__ == '__main__':
    unittest.main()
