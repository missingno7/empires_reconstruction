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
        matching = [item for item in result['levels'] if item['level'] != 'HISTORICAL_LIBRARY']
        self.assertEqual(sum(item['owners'] for item in matching), 347)
        self.assertEqual(result['asm_db_source_files'], 4)
        self.assertGreater(result['asm_db_capsules'][0]['bytes'], 1000)

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
                         {'_g1784', '_g1786', '_g177a', '_g17ac', '_f_c6b9'})

    def test_f880a_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_880A')
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_ASM', 'asm/F_880A.ASM'))
        self.assertEqual(classify_asm_source(ROOT / owner['source']), 'SYMBOLIC_ASM')

    def test_f8bab_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_8BAB')
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_ASM', 'asm/F_8BAB.ASM'))
        self.assertEqual(classify_asm_source(ROOT / owner['source']), 'SYMBOLIC_ASM')

    def test_faa1f_is_symbolic_tasm(self):
        manifest = read_json(ROOT / 'layout/manifest.json')
        owner = next(item for item in manifest['regions'] if item['id'] == 'F_AA1F')
        self.assertEqual((owner['kind'], owner['source']),
                         ('MATCHING_ASM', 'asm/F_AA1F.ASM'))
        self.assertEqual(classify_asm_source(ROOT / owner['source']), 'SYMBOLIC_ASM')


if __name__ == '__main__':
    unittest.main()
