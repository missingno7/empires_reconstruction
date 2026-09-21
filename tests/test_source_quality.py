from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from report_source_quality import classify_source, classify_asm_source, report
from reconstruct import read_json


class SourceQualityTests(unittest.TestCase):
    def test_syntactic_levels(self):
        path = ROOT / 'build/source-quality-fixture.c'
        try:
            path.write_text('void f() {\n    asm db 90h;\n}\n')
            self.assertEqual(classify_source(path), 'ASM_DB_CAPSULE')
            path.write_text('void f() {\n    asm mov ax,bx;\n}\n')
            self.assertEqual(classify_source(path), 'C_WITH_SYMBOLIC_INLINE_ASM')
            path.write_text('void f() {}\n')
            self.assertEqual(classify_source(path), 'MECHANICAL_C')
            path.write_text('db 90h\n')
            self.assertEqual(classify_asm_source(path), 'ASM_DB_CAPSULE')
            path.write_text('JMP_NEAR macro target\n    db 0e9h\n'
                            '    dw target-$-2\nendm\n')
            self.assertEqual(classify_asm_source(path), 'SYMBOLIC_ASM')
            path.write_text('nop\n')
            self.assertEqual(classify_asm_source(path), 'SYMBOLIC_ASM')
        finally:
            path.unlink(missing_ok=True)

    def test_manifest_inventory_is_complete(self):
        result = report(read_json(ROOT / 'layout/manifest.json'))
        self.assertEqual(result['format'], 'empires-source-quality-v2')
        matching = [item for item in result['levels'] if item['level'] != 'HISTORICAL_LIBRARY']
        # Grouped multi-source modules reduced distinct matching owners from 347 to 345;
        # RUNTIME_BLOCK now contributes 3 nonzero RUNTIME_* owner rows (raw_unresolved
        # is 0) instead of 1 ASM_DB_CAPSULE row: 345 - 1 + 3 = 347.
        self.assertEqual(sum(item['owners'] for item in matching), 347)
        self.assertEqual(result['asm_db_source_files'], 0)
        capsule_level = next(item for item in result['levels']
                             if item['level'] == 'ASM_DB_CAPSULE')
        self.assertEqual((capsule_level['bytes'], capsule_level['owners']), (0, 0))
        self.assertEqual(result['asm_db_capsules'], [])
        raw_unresolved = next(item for item in result['levels']
                              if item['level'] == 'RUNTIME_RAW_UNRESOLVED')
        self.assertEqual(raw_unresolved['bytes'], 0)
        runtime_levels = {item['level']: item['bytes'] for item in result['levels']
                          if item['level'].startswith('RUNTIME_')}
        self.assertEqual(sum(runtime_levels.values()), 6571)

    def test_f4b0c_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_4B0C')
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_ASM', 'asm/F_4B0C.ASM'))
        self.assertEqual(classify_asm_source(ROOT / owner['source']), 'SYMBOLIC_ASM')

    def test_f4eeb_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_4EEB')
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(classify_asm_source(ROOT / owner['source']), 'SYMBOLIC_ASM')

    def test_fc755_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_C755')
        self.assertEqual(owner['kind'], 'MATCHING_ASM')
        self.assertEqual(classify_asm_source(ROOT / owner['source']), 'SYMBOLIC_ASM')
        self.assertEqual(set(owner['build']['bindings']),
                         {'_g1784', '_g1786', '_g177a', '_g17ac', '_voice_disable'})

    def test_f880a_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_880A')
        # F_880A was recovered as exact C; see docs/current/exact-c-recovery.md.
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_C', 'src/DIALOG.C'))
        self.assertEqual(classify_source(ROOT / owner['source']), 'MECHANICAL_C')

    def test_f8bab_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_8BAB')
        # F_8BAB was recovered as exact C; see docs/current/exact-c-recovery.md.
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_C', 'src/PUZZLE.C'))
        self.assertEqual(classify_source(ROOT / owner['source']), 'MECHANICAL_C')

    def test_faa1f_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_AA1F')
        # F_AA1F was recovered as exact C; see docs/current/exact-c-recovery.md.
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_C', 'src/SLOTMENU.C'))
        self.assertEqual(classify_source(ROOT / owner['source']), 'MECHANICAL_C')


if __name__ == '__main__':
    unittest.main()
