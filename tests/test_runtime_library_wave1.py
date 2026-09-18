"""Fresh binding checks for the six newly identified CC.LIB modules."""
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from reconstruct import (read_json, owned_library_modules, bind_region,
                         mismatch)
from mz import MZ


class RuntimeLibraryWave1Tests(unittest.TestCase):
    def test_fresh_library_modules_match(self):
        recipe = read_json(ROOT / 'recipes/c/matching-wave39-libraries.json')
        original = (ROOT / 'assets/AEPROG.EXE').read_bytes()
        lock = read_json(ROOT / 'layout/toolchain.json')
        manifest = read_json(ROOT / 'layout/manifest.json')
        modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain', lock)
        for owner in recipe['owners']:
            data, _ = bind_region(owner, modules[owner['id']], MZ.parse(original),
                                  manifest['frames'], manifest['regions'], modules)
            mismatch(original[owner['start']:owner['end']], data, owner)
