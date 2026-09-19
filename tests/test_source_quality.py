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
            path.write_text('nop\n')
            self.assertEqual(classify_asm_source(path), 'SYMBOLIC_ASM')
        finally:
            path.unlink(missing_ok=True)

    def test_manifest_inventory_is_complete(self):
        result = report(read_json(ROOT / 'layout/manifest.json'))
        matching = [item for item in result['levels'] if item['level'] != 'HISTORICAL_LIBRARY']
        self.assertEqual(sum(item['owners'] for item in matching), 347)
        self.assertEqual(result['asm_db_source_files'], 58)
        self.assertGreater(result['asm_db_capsules'][0]['bytes'], 1000)


if __name__ == '__main__':
    unittest.main()
