"""Factory contracts, conservative CFG, cache identity and bounded promotion."""
from pathlib import Path
import json
import sys
import tempfile
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from runtime_cfg import recursive_cfg, resolve_target, runtime_cfg, target_index
from runtime_source import instrument, quality, oracle_bytes
from check_candidate import diagnose_bytes, outside_scope, CandidateFailure, check
from interface_census import declarations, census
from production_plan import generate, checked_plan
from object_cache import cache_key
from factory_inputs import input_fingerprint
from reconstruct import read_json


class FactoryTests(unittest.TestCase):
    def test_recursive_jump_skips_table_that_decodes_as_code(self):
        cfg=recursive_cfg(bytes.fromhex('eb04b83412c390c3'),[{'offset':0,'names':['entry']}])
        self.assertEqual([i['offset'] for i in cfg['instructions']],[0,6,7])
        self.assertEqual(cfg['unreachable_gaps'],[{'start':2,'end':6,'classification':'UNREACHABLE_UNKNOWN'}])

    def test_loop_is_an_edge_and_fallthrough(self):
        cfg=recursive_cfg(bytes.fromhex('90e2fdc3'),[{'offset':0,'names':['entry']}])
        self.assertIn((1,0,'BRANCH'),[(e['source'],e['target'],e['kind']) for e in cfg['edges']])
        self.assertEqual(len(cfg['instructions']),3)

    def test_call_follows_target_and_fallthrough(self):
        cfg=recursive_cfg(bytes.fromhex('e80100c390c3'),[{'offset':0,'names':['entry']}])
        self.assertEqual([i['offset'] for i in cfg['instructions']],[0,3,4,5])
        self.assertEqual(cfg['direct_calls'][0]['target'],4)

    def test_unknown_indirect_does_not_sweep_table(self):
        cfg=recursive_cfg(bytes.fromhex('ffe09090c3'),[{'offset':0,'names':['entry']}])
        self.assertEqual(len(cfg['instructions']),1)
        self.assertEqual(len(cfg['indirect_sites']),1)

    def test_explicit_data_cannot_be_decoded(self):
        cfg=recursive_cfg(b'\x90\xc3',[{'offset':0,'names':['entry']}],data_ranges=[{'start':0,'end':1}])
        self.assertEqual(cfg['instructions'],[])
        self.assertEqual(cfg['issues'][0]['kind'],'CONTROL_FLOW_ENTERS_DATA')

    def test_exact_public_wins_over_owner_interior(self):
        owners=[{'id':'large','start':512,'end':600}]
        index={20:[{'symbol':'actual_function','classification':'KNOWN_FUNCTION_ENTRY','owner':'small'}]}
        result=resolve_target(20,index,owners,blocks={20})
        self.assertEqual(result['symbol'],'actual_function')
        self.assertEqual(result['classification'],'KNOWN_FUNCTION_ENTRY')

    def test_secondary_public_wins_over_local_block(self):
        result=resolve_target(7,{7:[{'symbol':'secondary','classification':'KNOWN_SECONDARY_PUBLIC'}]},[],blocks={7})
        self.assertEqual(result['classification'],'KNOWN_SECONDARY_PUBLIC')

    def test_real_runtime_is_deterministic_and_bounded(self):
        binary=oracle_bytes(); a=runtime_cfg(binary); b=runtime_cfg(binary)
        self.assertEqual(a,b); self.assertEqual(a['issues'],[])
        self.assertEqual(len(a['roots']),20)
        self.assertFalse(any(60<=i['offset']<404 for i in a['instructions']))
        self.assertTrue(a['indirect_sites'])

    def test_macro_cannot_fake_quality(self):
        with self.assertRaisesRegex(ValueError,'Unapproved'):
            instrument('JMP_NEAR macro target\ndb 90h\nendm\nJMP_NEAR foo\nend')

    def test_assembler_error_maps_back_to_original_source(self):
        from runtime_source import assembly_diagnostics
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as directory:
            work=Path(directory)
            (work/'BUILD.LOG').write_text('**Error** R0000.ASM(17) Undefined symbol: missing\n')
            result=assembly_diagnostics(work,[{'line':12,'instrumented_line':17}])
            self.assertEqual(result[0]['source_line'],12)
            self.assertIn('missing',result[0]['message'])

    def test_named_db_and_dw_cannot_fake_symbolic_progress(self):
        _,rows=instrument('named db 90h\nlabelled: dw 1234h\nend')
        self.assertEqual([r['category'] for r in rows],['raw_unresolved','raw_unresolved'])

    def test_raw_dw_does_not_fake_typed_data(self):
        m={'bytes':2,'ranges':[{'start':0,'end':2,'category':'raw_unresolved','text':'dw 1234h'}]}
        self.assertEqual(quality(m)['raw_unresolved_bytes'],2)
        self.assertEqual(quality(m,evidence={'data_ranges':[{'start':0,'end':2,'evidence':'proven table'}]})['typed_table_data_bytes'],2)

    def test_absolute_memory_diagnostic(self):
        self.assertEqual(diagnose_bytes(bytes.fromhex('8b3ee4c0'),bytes.fromhex('bfe4c0'))['pattern'],'TASM_ABSOLUTE_MEMORY')

    def test_near_jump_diagnostic(self):
        d=diagnose_bytes(bytes.fromhex('e98200'),bytes.fromhex('eb8290'))
        self.assertEqual(d['pattern'],'TASM_SHORTENED_BRANCH'); self.assertEqual(d['first_mismatch'],0)

    def test_edit_boundary_handles_added_lines_but_not_neighbors(self):
        self.assertTrue(outside_scope('a\nb\nc\n','a\nx\ny\nc\n',2,2))
        self.assertFalse(outside_scope('a\nb\nc\n','d\nx\nc\n',2,2))

    def test_unspecified_arguments_are_not_zero(self):
        functions,_,_,_=declarations('extern int f();\nint f(int x) { return x; }','sample.c')
        self.assertEqual([f['argument_count'] for f in functions],[None,1])

    def test_unnamed_argument_type_and_declaration_line_are_preserved(self):
        functions,_,_,_=declarations('extern int f(char);\nvoid g(int x) {}','sample.c')
        self.assertEqual(functions[0]['argument_types'],['char'])
        self.assertEqual(functions[1]['line'],2)

    def test_return_pointer_and_void_conflicts_are_visible(self):
        functions,_,_,_=declarations('extern char near *f();\nchar far *f(int x) { return 0; }\nextern void g();\nint g(void) { return 0; }','sample.c')
        self.assertEqual([f['return_type'] for f in functions],['char near *','char far *','void','int'])

    def test_knr_parameters_are_not_globals_and_types_are_retained(self):
        f,g,_,_=declarations('f(p,n) char *p; unsigned n; { return n; }','knr.c')
        self.assertEqual(g,[])
        self.assertTrue(f[0]['definition'])
        self.assertEqual(f[0]['argument_types'],['char *','unsigned'])

    def test_complex_pointer_declarators_remain_explicit_unknowns(self):
        f,g,_,u=declarations('extern void (*table[])(void); extern void interrupt (*getvect())();','ptr.c')
        self.assertEqual(len(u),2)
        self.assertEqual(g[0]['symbol'],'table')
        self.assertEqual(f[0]['confidence'],'UNKNOWN')

    def test_comma_record_fields_are_all_counted(self):
        _,_,records,_=declarations('struct R { char pad[13]; int a,b; char tail[10]; };','r.c')
        self.assertEqual(records[0]['bytes'],27)
        self.assertEqual([f['offset'] for f in records[0]['fields']],[0,13,15,17])

    def test_pointer_shape_is_per_global_declarator(self):
        _,g,_,_=declarations('extern char *a,b; struct X far *p;','g.c')
        self.assertEqual([x['type'] for x in g],['char *','char','struct X far *'])

    def test_record_fields_provide_layout_evidence(self):
        _,_,records,_=declarations('struct one { char a[9]; int b; char c[16]; };','sample.c')
        self.assertEqual(records[0]['bytes'],27)
        self.assertEqual(records[0]['fields'][1]['offset'],9)

    def test_census_records_real_conflicts_and_shared_layouts(self):
        result=census()
        self.assertTrue(result['conflicts']); self.assertTrue(result['consolidation_candidates'])
        self.assertTrue(any(r['name']=='record27' and r['bytes']==27 for r in result['records']))

    def test_checked_plan_is_complete_and_single_order(self):
        plan=checked_plan(); self.assertEqual(plan,generate())
        self.assertEqual(len(set(plan['object_order'])),len(plan['object_order']))
        runtime=next(m for m in plan['modules'] if m['id']=='RUNTIME_BLOCK')
        self.assertEqual(runtime['source'],'asm/RUNTIME_BLOCK.ASM')
        self.assertEqual(runtime['tool'],'TASM.EXE')
        self.assertEqual(sum(b['logical_end']-b['logical_start'] for b in plan['bss']),37250)

    def test_cache_key_ignores_session_path_but_tracks_flags_and_headers(self):
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as d:
            root=Path(d); (root/'include').mkdir(); (root/'tools').mkdir()
            (root/'tools/reconstruct.py').write_text('driver')
            (root/'a').mkdir(); (root/'b').mkdir()
            for f in ('a/unit.C','b/unit.C'): (root/f).write_text('int f(){}')
            owner={'id':'one','source':'a/unit.C','kind':'MATCHING_C','build':{}}
            key=cache_key(root,owner,{})
            self.assertEqual(key,cache_key(root,dict(owner,source='b/unit.C'),{}))
            self.assertNotEqual(key,cache_key(root,dict(owner,build={'flags_append':'-O'}),{}))
            (root/'include/X.H').write_text('int x;')
            self.assertNotEqual(key,cache_key(root,owner,{}))

    def test_accepted_refresh_cannot_bless_stale_success(self):
        from reconstruction_factory import refresh
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as directory:
            root=Path(directory); (root/'build').mkdir(); (root/'docs').mkdir()
            (root/'build/exe-build-report.json').write_text('{"status":"BUILT","mode":"ACCEPTANCE","fresh_build":true,"input_fingerprint":"old"}')
            with patch('reconstruction_factory.input_fingerprint',return_value='new'):
                with self.assertRaisesRegex(ValueError,'fresh complete acceptance'):
                    refresh(root,accepted=True)

    def test_failed_promotion_invalidates_earlier_acceptance(self):
        from check_candidate import promote
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as d:
            root=Path(d); (root/'build').mkdir()
            receipt=root/'build/candidate-promotion.json'; receipt.write_text('{\"status\":\"ACCEPTED\"}')
            with patch('check_candidate.check',side_effect=CandidateFailure('STALE_INPUTS','changed')):
                with self.assertRaises(CandidateFailure): promote('fake',root)
            self.assertFalse(receipt.exists())

    def test_fast_never_reuses_old_success(self):
        success=ROOT/'build/candidate-fast.json'; success.write_text('{"status":"PASS"}')
        with patch('check_candidate.context',side_effect=CandidateFailure('STALE_INPUTS','changed')):
            with self.assertRaises(CandidateFailure): check('fake')
        self.assertFalse(success.exists())


if __name__=='__main__': unittest.main()
