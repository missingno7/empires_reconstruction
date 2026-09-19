"""Opt-in integration regression for the two DOS execution hosts."""
import os
from pathlib import Path
import subprocess
import sys
import unittest


@unittest.skipUnless(os.environ.get('EMPIRES_RUN_DOS_RUNNER_PARITY') == '1',
                     'set EMPIRES_RUN_DOS_RUNNER_PARITY=1 with both runner paths to run')
class RunnerParityIntegrationTests(unittest.TestCase):
    def test_complete_structural_build_matches_between_hosts(self):
        root = Path(__file__).resolve().parents[1]
        player = os.environ.get('MSDOS_PLAYER')
        self.assertTrue(player and Path(player).is_file(), 'MSDOS_PLAYER must identify msdos.exe')
        command = [sys.executable, 'tools/probe_runner_parity.py', '--msdos-player', player]
        if os.environ.get('DOSBOX'):
            command += ['--dosbox', os.environ['DOSBOX']]
        subprocess.run(command, cwd=root, check=True, timeout=300)


if __name__ == '__main__':
    unittest.main()
