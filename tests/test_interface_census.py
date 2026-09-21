import sys, unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from interface_census import declarations, object_declarations, type_size, census


def globals_by_symbol(text, path, symbol):
    _, g, _, _ = declarations(text, path)
    return [x for x in g if x['symbol'] == symbol]


class FarPointerInNearObject(unittest.TestCase):
    """`far * near` is a far pointer VALUE stored in a near object: size 4, not 2."""

    def test_far_pointer_near_object(self):
        rows = globals_by_symbol('extern char far * near gb31;\n', 'x.C', 'gb31')
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['pointer_shape'], 'far')
        self.assertEqual(rows[0]['storage'], 'near')
        self.assertEqual(rows[0]['size'], 4)

    def test_near_array_is_not_a_pointer(self):
        rows = globals_by_symbol('extern char near g1356[];\n', 'x.C', 'g1356')
        self.assertEqual(len(rows), 1)
        self.assertIsNone(rows[0]['pointer_shape'])
        self.assertEqual(rows[0]['storage'], 'near')

    def test_near_scalar_is_not_a_pointer(self):
        rows = globals_by_symbol('extern char near current_drive;\n', 'x.C', 'current_drive')
        self.assertEqual(len(rows), 1)
        self.assertIsNone(rows[0]['pointer_shape'])
        self.assertEqual(rows[0]['storage'], 'near')
        self.assertEqual(rows[0]['size'], 1)

    def test_type_size_uses_pre_star_qualifier(self):
        self.assertEqual(type_size('char far * near'), 4)
        self.assertEqual(type_size('char near *'), 2)
        self.assertEqual(type_size('char near'), 1)


class FunctionPointerDeclarators(unittest.TestCase):
    def test_interrupt_pointer_with_storage_qualifier(self):
        rows = globals_by_symbol('extern void interrupt (* near int9_saved_vector)(void);\n', 'x.C', 'int9_saved_vector')
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual(row['pointer_shape'], 'function')
        self.assertEqual(row['code_model'], 'far')
        self.assertEqual(row['storage'], 'near')
        self.assertEqual(row['size'], 4)
        self.assertTrue(row['interrupt'])

    def test_interrupt_pointer_no_storage_qualifier(self):
        rows = globals_by_symbol('extern void interrupt (*int9_saved_vector)();\n', 'x.C', 'int9_saved_vector')
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['pointer_shape'], 'function')
        self.assertEqual(rows[0]['size'], 4)

    def test_function_returning_function_pointer(self):
        f, g, r, u = declarations('extern void interrupt (*getvect())();\n', 'x.C')
        self.assertTrue(any(fn['symbol'] == 'getvect' for fn in f))
        self.assertFalse(any('getvect' in str(row.get('declaration', '')) for row in u))

    def test_array_of_function_pointers(self):
        rows = globals_by_symbol('extern void (*g12a1[])(void);\n', 'x.C', 'g12a1')
        self.assertEqual(len(rows), 1)
        row = rows[0]
        self.assertEqual(row['pointer_shape'], 'function')
        self.assertEqual(row['array'], '[]')
        self.assertEqual(row['code_model'], 'near')
        self.assertEqual(row['size'], 2)


class AnonymousStruct(unittest.TestCase):
    def test_anonymous_struct_array(self):
        text = 'extern struct { int w0; int w2; } a1271[];\n'
        f, g, r, u = declarations(text, 'x.C')
        self.assertFalse(u, u)
        rows = [x for x in g if x['symbol'] == 'a1271']
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['array'], '[]')
        self.assertIsNone(rows[0]['pointer_shape'])
        self.assertEqual(len(r), 1)
        record = r[0]
        self.assertTrue(record['name'].startswith('<anonymous>@'))
        offsets = {field['name']: (field['offset'], field['bytes']) for field in record['fields']}
        self.assertEqual(offsets['w0'], (0, 2))
        self.assertEqual(offsets['w2'], (2, 2))
        self.assertEqual(record['bytes'], 4)

    def test_anonymous_struct_tag_is_file_scoped(self):
        text = 'extern struct { int w0; int w2; } a1271[];\n'
        _, _, r1, _ = declarations(text, 'a.C')
        _, _, r2, _ = declarations(text, 'b.C')
        self.assertNotEqual(r1[0]['name'], r2[0]['name'])


class StringInitializers(unittest.TestCase):
    def test_string_literal_with_commas_and_escapes(self):
        text = 'char text118c[] = "Sorry, better luck!\\r\\rReset.";\n'
        f, g, r, u = declarations(text, 'x.C')
        self.assertFalse(u, u)
        rows = [x for x in g if x['symbol'] == 'text118c']
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['type'], 'char')
        self.assertEqual(rows[0]['array'], '[]')

    def test_struct_brace_initializer_still_yields_global(self):
        text = ('struct R125D { int a; char *b; };\n'
                'struct R125D g125d = { 1, 0, text118c, { 0,0xff,0xff } };\n')
        f, g, r, u = declarations(text, 'x.C')
        rows = [x for x in g if x['symbol'] == 'g125d']
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['type'], 'struct R125D')

    def test_line_numbers_preserved_around_blanked_literal(self):
        text = 'char a[] = "x, y; z";\nchar after_marker;\n'
        f, g, r, u = declarations(text, 'x.C')
        rows = [x for x in g if x['symbol'] == 'after_marker']
        self.assertEqual(rows[0]['line'], 2)


class EndToEnd(unittest.TestCase):
    def test_census_regressions(self):
        result = census()
        gc0cc_rows = [g for g in result['globals'] if g['symbol'] == 'int9_saved_vector']
        self.assertEqual(len(gc0cc_rows), 1)
        decls = gc0cc_rows[0]['declarations']
        # KEYIRQ.C merged the two IRQ1 files into one prologue; the handler
        # (F_699E.C) keeps its own declaration.
        self.assertGreaterEqual(len(decls), 2)
        self.assertTrue(all(d['pointer_shape'] == 'function' and d.get('interrupt') for d in decls), decls)

        gb31_rows = [g for g in result['globals'] if g['symbol'] == 'gb31']
        self.assertEqual(len(gb31_rows), 1)
        for d in gb31_rows[0]['declarations']:
            self.assertEqual(d['pointer_shape'], 'far')
            self.assertEqual(d['size'], 4)

        unsupported_symbols = [u['declaration'] for u in result['unsupported']]
        self.assertFalse(any('a1271' in s for s in unsupported_symbols))
        self.assertFalse(any('text118c' in s for s in unsupported_symbols))
        self.assertFalse(any(s.strip() == 'near' for s in unsupported_symbols))
        self.assertFalse(any('int9_saved_vector' in s for s in unsupported_symbols))


if __name__ == '__main__':
    unittest.main()
