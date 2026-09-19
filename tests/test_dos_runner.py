import sys
from pathlib import Path
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from dos_runner import resolve_runner

class RunnerSelectionTests(unittest.TestCase):
    def test_msdos_is_preferred_when_present(self):
        root = Path(__file__).resolve().parents[1]
        fake = root / 'build' / 'runner-test-msdos.exe'
        fake.parent.mkdir(exist_ok=True); fake.write_bytes(b'MZ')
        try:
            runner = resolve_runner({'runner': {'default': 'msdos-player', 'fallback': 'dosbox'}}, executable=fake)
            self.assertEqual(runner.backend, 'msdos-player')
        finally: fake.unlink(missing_ok=True)

    def test_explicit_missing_msdos_fails_without_silent_switch(self):
        with self.assertRaisesRegex(ValueError, 'unavailable'):
            resolve_runner({'runner': {'default': 'msdos-player', 'fallback': 'dosbox'}}, backend='msdos-player', executable='Z:/missing/msdos.exe')

if __name__ == '__main__': unittest.main()
