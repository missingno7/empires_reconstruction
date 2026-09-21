#!/usr/bin/env python3
"""Verification tests for tools/portable/datagen.py.

Run with:  python -m unittest tools.portable.test_datagen -v
       or:  python tools/portable/test_datagen.py
"""
import sys
import tempfile
import unittest
from pathlib import Path

THIS_FILE = Path(__file__).resolve()
ROOT = THIS_FILE.parents[2]
if str(THIS_FILE.parent) not in sys.path:
    sys.path.insert(0, str(THIS_FILE.parent))

import datagen as dg  # noqa: E402


class DatagenTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        out_root = Path(cls.tmp.name)
        cls.report, cls.aux = dg.generate(
            out_generated=out_root / 'portable/generated',
            out_docs=out_root / 'docs/portable',
            verbose=False)
        cls.out_root = out_root

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    # -- (a) DATA image length -------------------------------------------

    def test_data_image_length_is_0x3902(self):
        self.assertEqual(len(self.aux['image']), 0x3902)
        self.assertEqual(dg.DATA_LEN, 0x3902)
        self.assertTrue(self.report['data_len_ok'])

    def test_generation_succeeds_without_the_exe_present(self):
        # assets/AEPROG.EXE is an OPTIONAL cross-check only; the JSON
        # recipe sources (decoded through the historical tools/*.py
        # encoders) are the sole byte source. Verified here by temporarily
        # hiding the real path from cross_check_against_exe and re-running
        # the full pipeline -- it must produce byte-identical output.
        real_exists = dg.Path.exists
        try:
            dg.Path.exists = lambda self: False if self == dg.EXE_PATH else real_exists(self)
            out2 = Path(tempfile.mkdtemp())
            report2, aux2 = dg.generate(out2 / 'portable/generated', out2 / 'docs/portable',
                                         verbose=False)
        finally:
            dg.Path.exists = real_exists
        self.assertFalse(report2['exe_cross_check']['checked'])
        self.assertTrue(report2['data_len_ok'])
        self.assertEqual(aux2['image'], self.aux['image'])
        self.assertEqual(report2['game_data']['emitted'], self.report['game_data']['emitted'])

    def test_component_map_covers_data_exactly_once(self):
        components = self.aux['components']
        cursor = 0
        for c in sorted(components, key=lambda c: c['ds_offset']):
            self.assertEqual(c['ds_offset'], cursor,
                              f"gap/overlap before component {c['id']}")
            cursor += c['length']
        self.assertEqual(cursor, dg.DATA_LEN)

    # -- (b) emitted initializer bytes match the historical image --------

    def test_emitted_bytes_match_image_for_a_sample_of_symbols(self):
        image = self.aux['image']
        verify_bytes = self.aux['verify_bytes']
        symbols_by_primary = {s['primary']: s for s in self.aux['symbols']}
        self.assertGreater(len(verify_bytes), 50, 'expected many emitted DATA objects')

        # A struct-shaped component's pointer32/offset16 fields are zeroed
        # in `image` by the encoder (unresolved) but hold a real, link-time
        # pointer *value* in `emitted` -- not a byte pattern this test can
        # predict -- so those specific windows, relative to the component
        # base, are masked out on both sides before comparing.
        ref_windows_by_component = {}
        for c in self.aux['components']:
            windows = []
            for ref in c.get('refs', []):
                width = 4 if ref.get('loc', 'pointer32') == 'pointer32' else 2
                windows.append((ref['offset'], ref['offset'] + width))
            if windows:
                ref_windows_by_component[c['id']] = windows

        # Sample: every 7th emitted DATA symbol (deterministic, covers many
        # formats without comparing all ~100 for speed).
        sample = list(verify_bytes.items())[::7]
        self.assertGreater(len(sample), 5)
        checked = 0
        for primary, emitted in sample:
            sym = symbols_by_primary[primary]
            offset, length = sym['offset'], sym['size']
            expected = bytearray(image[offset:offset + length])
            emitted = bytearray(emitted)
            self.assertEqual(len(emitted), len(expected), f'{primary}: length mismatch')
            comp = next((c for c in self.aux['components'] if c['id'] == sym['component_id']), None)
            if comp is not None:
                comp_local_offset = offset - comp['ds_offset']
                for start, end in ref_windows_by_component.get(comp['id'], []):
                    for i in range(max(start, comp_local_offset), min(end, comp_local_offset + length)):
                        expected[i - comp_local_offset] = 0
                        emitted[i - comp_local_offset] = 0
            self.assertEqual(bytes(emitted), bytes(expected),
                              f'{primary} (DS:{offset:04x}): emitted bytes differ from the DATA '
                              'image outside pointer fields')
            checked += 1
        self.assertGreater(checked, 5)

    def test_ascii_string_symbol_matches_image_exactly(self):
        # A plain ascii-nul-v1 string has no pointer fields, so its emitted
        # bytes must equal the historical image byte-for-byte, no masking.
        image = self.aux['image']
        found = False
        for s in self.aux['symbols']:
            if s['component_id'] is None or s['section'] != 'data':
                continue
            comp = next((c for c in self.aux['components'] if c['id'] == s['component_id']), None)
            if comp is None or comp['format'] != 'ascii-nul-v1':
                continue
            emitted = self.aux['verify_bytes'].get(s['primary'])
            if emitted is None:
                continue
            expected = image[s['offset']:s['offset'] + s['size']]
            self.assertEqual(emitted, expected)
            found = True
            break
        self.assertTrue(found, 'expected at least one emitted ascii-nul-v1 symbol')

    # -- (c) every pointer32/offset16 ref resolves to a named target -----

    def test_every_pointer_ref_resolves_to_a_named_target(self):
        table = self.aux['table']
        components = self.aux['components']
        symbols = self.aux['symbols']
        resolver = dg.PointerResolver(table, symbols)
        total_refs = 0
        for c in components:
            if c['kind'] != 'data':
                continue
            for ref in c.get('refs', []):
                total_refs += 1
                # Must not raise, and must land inside a claimed span with a
                # real primary name.
                sym, local = resolver.resolve(ref['target'], ref.get('addend', 0))
                self.assertIsNotNone(sym['primary'])
                self.assertGreaterEqual(local, 0)
                self.assertLess(local, sym['size'])
        self.assertEqual(total_refs, self.report['pointer_refs_total'])
        self.assertGreater(total_refs, 0)

    def test_game_data_emission_did_not_raise_and_resolved_all_pointers(self):
        # generate() ran emit_game_data() in setUpClass; describe_and_emit_component
        # raises ValueError on any unresolved/misshapen pointer target, so
        # simply reaching here with files on disk is itself part of the
        # proof -- this test just pins the observable counts. Objects are
        # symbol-driven (a component can be split into several objects, or
        # merged with its neighbors, or produce none at all if nothing
        # names any offset in it and something wider swallows it), so the
        # count no longer needs to equal len(components) 1:1; it only needs
        # to be in the right ballpark and every component-count bucket
        # needs to add up against the *component* totals it was drawn from.
        gd = self.report['game_data']
        self.assertGreater(gd['emitted'], 0)
        self.assertEqual(gd['skipped_code_owned'],
                          sum(1 for c in self.aux['components'] if c['kind'] == 'code_owned'))
        self.assertEqual(gd['skipped_toolchain'],
                          sum(1 for c in self.aux['components'] if c['kind'] == 'toolchain_opaque'))

    # -- output files exist and are well-formed ---------------------------

    def test_output_files_written(self):
        gen = self.out_root / 'portable/generated'
        for name in ('game_data.h', 'game_data.c', 'game_state.h', 'game_state.c', 'symbols.json'):
            path = gen / name
            self.assertTrue(path.exists(), f'{name} was not written')
            self.assertGreater(path.stat().st_size, 0, f'{name} is empty')
        state_map = self.out_root / 'docs/portable/state-map.md'
        self.assertTrue(state_map.exists())
        self.assertGreater(state_map.stat().st_size, 0)

    def test_symbols_json_is_well_formed_and_matches_report(self):
        import json
        doc = self.aux['symbols_doc']
        self.assertEqual(doc['data_len'], dg.DATA_LEN)
        self.assertEqual(doc['bss_len'], dg.BSS_LEN)
        self.assertEqual(len(doc['symbols']), len(self.aux['symbols']))
        # round-trips through JSON on disk too
        on_disk = json.loads((self.out_root / 'portable/generated/symbols.json').read_text())
        self.assertEqual(on_disk['data_len'], dg.DATA_LEN)

    def test_no_duplicate_top_level_identifiers_between_data_and_state(self):
        import re
        gen = self.out_root / 'portable/generated'
        names = []
        for fn in ('game_data.h', 'game_state.h'):
            text = (gen / fn).read_text(encoding='utf-8')
            names += re.findall(r'^extern\s+(?:struct\s+\w+\s+)?[\w*\s]+?\b(\w+)(?:\[|;)',
                                 text, re.M)
            names += re.findall(r'^#define\s+(\w+)\s+', text, re.M)
        dupes = {n for n in names if names.count(n) > 1}
        self.assertEqual(dupes, set(), f'duplicate top-level identifiers: {dupes}')

    def test_no_unowned_objects_collide_with_state_ownership(self):
        owned = dg.load_state_ownership()
        for s in self.aux['symbols']:
            if s['owned_by'] is not None:
                self.assertIn(s['owned_by'], owned)
                self.assertEqual(s['definition_site'], 'subsystem-owned')

    # -- deterministic ordering -------------------------------------------

    def test_generation_is_deterministic(self):
        out2 = Path(tempfile.mkdtemp())
        try:
            dg.generate(out2 / 'portable/generated', out2 / 'docs/portable', verbose=False)
            gen1 = self.out_root / 'portable/generated'
            gen2 = out2 / 'portable/generated'
            for name in ('game_data.h', 'game_data.c', 'game_state.h', 'game_state.c',
                         'symbols.json'):
                self.assertEqual((gen1 / name).read_bytes(), (gen2 / name).read_bytes(),
                                  f'{name} is not deterministic across runs')
        finally:
            import shutil
            shutil.rmtree(out2, ignore_errors=True)

    # -- supervisor rules A-H (derivation-warning resolutions) ------------

    def test_zero_remaining_derivation_warnings(self):
        # Every one of the original 11 derivation warnings has a specific
        # supervisor-approved resolution (rules A-H); none of them should
        # still be falling back to a generic "conflict" warning.
        self.assertEqual(self.report['warnings'], 0, self.aux['warnings'])

    def test_rule_a_segment_half_pointers_are_real_pointers_not_split(self):
        # int8_saved_vector / rect_queue_write_ptr / gc0ba / ui_gfx_blob /
        # gc5da: a name 2 bytes after a 4-byte far/interrupt pointer is that
        # pointer's own segment word, absorbed (not a separate object).
        by_primary = {s['primary']: s for s in self.aux['symbols']}
        # rect_queue_write_ptr/ui_gfx_blob are subsystem-owned (portable/gfx
        # /resource define them); their span/pointer-ness still comes from
        # this resolution, but the emitted display type (with the trailing
        # `*`) only exists for the ones this generator actually emits.
        cases = {
            'int8_saved_vector': ('void', 2938, 4, True),
            'rect_queue_write_ptr': ('dos_char', 16580, 4, False),
            'gc0ba': ('dos_char', 49338, 4, True),
            'ui_gfx_blob': ('dos_char', 50634, 4, False),
            'gc5da': ('dos_char', 50650, 4, True),
        }
        for name, (base_type, offset, size, emitted) in cases.items():
            sym = by_primary[name]
            self.assertEqual(sym['offset'], offset, name)
            self.assertEqual(sym['size'], size, name)
            self.assertTrue(sym['is_ptr'], name)
            expected = f'{base_type} *' if emitted else base_type
            self.assertEqual(sym['c_type'], expected, name)
        # The absorbed +2 names never got their own symbol.
        absorbed = {'g0b7c', 'g40c6', 'gc0bc', 'gc5cc', 'gc5dc', 'dialog_backdrop_save_size'}
        primaries = {s['primary'] for s in self.aux['symbols']}
        for name in absorbed:
            self.assertNotIn(name, primaries, f'{name} should have been absorbed, not emitted')
        self.assertGreaterEqual(len(self.aux['extra']['segment_half']), 5)
        # rule A/H: emitted as `void *`/`TYPE *`, never a function-pointer
        # type. int8_saved_vector is DATA (always generated); gc0ba/gc5da
        # are BSS and not subsystem-owned, so they are also emitted by us.
        header = (self.out_root / 'portable/generated/game_data.h').read_text('utf-8')
        self.assertIn('extern void *int8_saved_vector;', header)
        state_header = (self.out_root / 'portable/generated/game_state.h').read_text('utf-8')
        self.assertIn('extern dos_char *gc0ba;', state_header)
        self.assertIn('extern dos_char *gc5da;', state_header)

    def test_rule_b_interior_alias_is_a_field_expression_macro(self):
        # gc563 lands exactly on slot_table[9].text[0]; slot_table stays a
        # full struct c470_record[10], gc563 becomes a #define expression.
        by_primary = {s['primary']: s for s in self.aux['symbols']}
        slot_table = by_primary['slot_table']
        self.assertEqual(slot_table['c_type'], 'struct c470_record[10]')
        self.assertEqual(slot_table['dims'], [10])
        self.assertEqual(slot_table['size'], 270)
        self.assertNotIn('gc563', {s['primary'] for s in self.aux['symbols']})
        reports = self.aux['extra']['interior_alias']
        gc563 = next(r for r in reports if r['name'] == 'gc563')
        self.assertEqual(gc563['expr'], '(slot_table[9].text[0])')
        state_header = (self.out_root / 'portable/generated/game_state.h').read_text('utf-8')
        self.assertIn('#define gc563 (slot_table[9].text[0])', state_header)

    def test_rule_c_multidim_array_clips_outer_dimension_keeps_stride(self):
        # int wig[24][12] only has 288 bytes before the next real symbol;
        # rule C clips the outer dimension to 12 rows, keeps the 12-int
        # (24-byte) row stride HITTEST.C's idx*0x18 indexing relies on.
        by_primary = {s['primary']: s for s in self.aux['symbols']}
        wig = by_primary['wig']
        self.assertEqual(wig['c_type'], 'dos_int[12][12]')
        self.assertEqual(wig['dims'], [12, 12])
        self.assertEqual(wig['size'], 288)
        header = (self.out_root / 'portable/generated/game_data.h').read_text('utf-8')
        self.assertIn('extern dos_int wig[12][12];', header)

    def test_rule_g_jmp_buf_uses_host_setjmp(self):
        by_primary = {s['primary']: s for s in self.aux['symbols']}
        sym = by_primary['game_abort_jmpbuf']
        self.assertEqual(sym['c_type'], 'jmp_buf')
        self.assertEqual(sym['size'], 20)  # historical Turbo C jmp_buf size
        header = (self.out_root / 'portable/generated/game_state.h').read_text('utf-8')
        self.assertIn('#include <setjmp.h>', header)
        self.assertIn('extern jmp_buf game_abort_jmpbuf;', header)
        # Never `jmp_buf name[N]` -- jmp_buf is already an array type in C.
        self.assertNotIn('jmp_buf game_abort_jmpbuf[', header)

    def test_rule_d_and_e_array_rounding(self):
        by_primary = {s['primary']: s for s in self.aux['symbols']}
        # Rule D (floor): gca6d, 23 measured bytes / 2-byte dos_uint -> 11
        # elements (22 bytes), 1 leftover byte left unclaimed.
        gca6d = by_primary['gca6d']
        self.assertEqual(gca6d['c_type'], 'dos_uint[11]')
        self.assertEqual(gca6d['dims'], [11])
        self.assertEqual(gca6d['size'], 22)
        # Rule D special case: g13ef is really a scalar (SLOTMENU.C) even
        # though LEVEL.C spells it as an unsized array.
        g13ef = by_primary['g13ef']
        self.assertEqual(g13ef['dims'], [1])
        self.assertEqual(g13ef['size'], 2)
        self.assertTrue(any('g13ef' in n for n in self.aux['extra']['porting_notes']))
        # Rule E (ceil): ga5e rounds UP to 6 whole 35-byte records (210
        # bytes), documented as overlapping the next object.
        ga5e = by_primary['ga5e']
        self.assertEqual(ga5e['c_type'], 'struct ga5e_entry[6]')
        self.assertEqual(ga5e['dims'], [6])
        self.assertEqual(ga5e['size'], 210)

    def test_rule_f_storage_alias_union(self):
        by_primary = {s['primary']: s for s in self.aux['symbols']}
        union = by_primary['sound_instrument_region']
        self.assertEqual(union['offset'], 0x2fd2)
        self.assertEqual(union['size'], 1924)
        header = (self.out_root / 'portable/generated/game_data.h').read_text('utf-8')
        self.assertIn('#define ui_panel_glyph_records ((struct g2fd2_entry *)(sound_instrument_region))',
                       header.replace('  ', ' '))
        self.assertIn('DATA_012C03_SOUND_INSTRUMENTS_s', header)

    # -- supervisor round 4: sound split + emit_qualifiers -----------------

    def test_sound_component_is_symbol_split(self):
        # DATA_01139E_SOUND must no longer be one opaque struct with
        # `#define sound_enabled DATA_01139E_SOUND`; every SOUND.H name
        # becomes its own typed object, the 38 pointer refs become
        # pointer-typed sound_dispatch_XXXX arrays, and the unnamed gaps
        # become sound_region_XXXX byte arrays -- with zero straddle
        # warnings and the whole 1832-byte span still exactly covered.
        by_primary = {s['primary']: s for s in self.aux['symbols']}
        self.assertNotIn('DATA_01139E_SOUND', by_primary)  # no monolithic object any more
        for name in ('sound_enabled', 'snd_on', 'music_enabled', 'mus_flag', 'snd_flag2',
                     'snd_backend_mode', 'snd_nvoices', 'snd_mode', 'snd_hi', 'opl_port'):
            sym = by_primary[name]
            self.assertIn(sym['c_type'], ('dos_int', 'dos_uint'), name)
            self.assertEqual(sym['dims'], [], name)
            self.assertEqual(sym['component_id'], 'DATA_01139E_SOUND', name)
        for name, count in (('v_b', 4), ('voice_stream_cursor_table', 4),
                            ('voice_stream_base_table', 4), ('v_ctr', 4), ('v_hold', 4),
                            ('v_len', 4), ('voice_rest_table', 4), ('notetab', 12)):
            self.assertEqual(by_primary[name]['dims'], [count], name)
        # The 38 refs: two note-bank pointers (grouped, since neither is
        # individually named) + the 36-entry dispatch table.
        dispatch = by_primary['sound_dispatch_182C']
        self.assertEqual(dispatch['is_ptr'], True)
        self.assertEqual(dispatch['dims'], [2])
        big_dispatch = by_primary['sound_dispatch_1832']
        self.assertEqual(big_dispatch['dims'], [36])
        total_refs = sum(len(s.get('refs', [])) for s in self.aux['symbols']
                         if s['component_id'] == 'DATA_01139E_SOUND')
        self.assertEqual(total_refs, 38)
        # Full coverage, no gaps: every sound sub-symbol's span is
        # contiguous across the whole 1832-byte component.
        sound_syms = sorted((s for s in self.aux['symbols']
                             if s['component_id'] == 'DATA_01139E_SOUND'),
                            key=lambda s: s['offset'])
        cursor = 0x176E
        for s in sound_syms:
            self.assertEqual(s['offset'], cursor, s['primary'])
            cursor += s['size']
        self.assertEqual(cursor, 0x176E + 1832)
        # No pointer ref ever straddled a named object's boundary.
        self.assertEqual([w for w in self.aux['warnings'] if 'straddles' in w], [])
        header = (self.out_root / 'portable/generated/game_data.h').read_text('utf-8')
        self.assertIn('extern dos_int sound_enabled;', header)
        self.assertIn('extern void *sound_dispatch_1832[36];', header)

    def test_sound_request_count_is_dos_int(self):
        # Explicitly asked for by the supervisor: DS:237C, outside the
        # sound component entirely, unaffected by the split.
        by_primary = {s['primary']: s for s in self.aux['symbols']}
        sym = by_primary['sound_request_count']
        self.assertEqual(sym['offset'], 0x237C)
        self.assertEqual(sym['c_type'], 'dos_int')

    def test_emit_qualifiers_applied_to_declaration_and_definition(self):
        # tools/portable/state_ownership.json's emit_qualifiers.timer_ticks
        # = "volatile" must land on both the extern decl and the definition.
        header = (self.out_root / 'portable/generated/game_data.h').read_text('utf-8')
        source = (self.out_root / 'portable/generated/game_data.c').read_text('utf-8')
        self.assertIn('extern volatile dos_ulong timer_ticks;', header)
        self.assertIn('volatile dos_ulong timer_ticks = 0;', source)
        # Nothing else picked up a qualifier it shouldn't have.
        qualifiers = dg.load_emit_qualifiers()
        self.assertEqual(qualifiers, {'timer_ticks': 'volatile'})


if __name__ == '__main__':
    unittest.main()
