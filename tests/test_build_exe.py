"""Opt-out-free integration coverage for the canonical structural EXE build."""
import os
from pathlib import Path
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from build_exe import ORIGINAL_SHA256, build, validate_toolchain


def local_linker_is_available():
    try:
        validate_toolchain(ROOT)
    except (OSError, ValueError, KeyError):
        return False
    return Path(os.environ.get('DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')).exists()


class CleanStructuralExeBuildTests(unittest.TestCase):
    @unittest.skipUnless(local_linker_is_available(), 'local pinned Borland toolchain/DOSBox is unavailable')
    def test_fresh_construction_does_not_open_original_fixture(self):
        fixture = (ROOT / 'assets/AEPROG.EXE').resolve()
        read_bytes = Path.read_bytes

        def reject_fixture(path):
            if path.resolve() == fixture:
                raise AssertionError('fixture read during fixture-free construction')
            return read_bytes(path)

        # The original remains on disk for normal verification, but this guard
        # proves the construction path itself neither reads nor copies it.
        with patch.object(Path, 'read_bytes', reject_fixture):
            report = build(ROOT, verify=False)
        self.assertEqual(report['status'], 'BUILT')
        self.assertEqual(report['sha256'], ORIGINAL_SHA256)
        self.assertEqual(report['relocations'], 106)
        self.assertFalse(report['verification']['performed'])
        self.assertEqual(report['verification']['reason'], 'verification not requested')

    @unittest.skipUnless(local_linker_is_available(), 'local pinned Borland toolchain/DOSBox is unavailable')
    def test_fresh_source_to_tlink_build_is_exact(self):
        # ``build`` removes this stale receipt before it creates every new
        # compiler/linker session. It proves no earlier probe report can be
        # accepted as the build result.
        stale = ROOT / 'build/tlink-structural-report.json'
        stale.parent.mkdir(exist_ok=True)
        stale.write_text('{"stale": true}\n')
        report = build(ROOT, verify=True)
        self.assertEqual(report['status'], 'BUILT')
        self.assertEqual(report['sha256'], ORIGINAL_SHA256)
        self.assertEqual(report['unresolved_symbols'], 0)
        self.assertEqual(report['relocations'], 106)
        self.assertTrue(report['verification']['byte_identical'])
        self.assertNotIn('stale', stale.read_text())
        self.assertTrue((ROOT / 'build/AEPROG.EXE').exists())


if __name__ == '__main__':
    unittest.main()
