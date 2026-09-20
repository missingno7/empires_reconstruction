"""Pinned toolchain regression probes for rules cheap grinders must not rediscover."""
from pathlib import Path
import sys
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]; sys.path.insert(0,str(ROOT/'tools'))
from reconstruct import read_json,read_object,compile_sources
from dos_runner import resolve_runner
from tasm_rules import JMP_NEAR, CALL_NEAR

PREFIX="_TEXT segment byte public 'CODE'\n_TEXT ends\n_DATA segment word public 'DATA'\n_DATA ends\n_BSS segment word public 'BSS'\n_BSS ends\nDGROUP group _DATA,_BSS\nassume cs:_TEXT,ds:DGROUP\n_TEXT segment byte public 'CODE'\n"
ASM={
'planar15':'lodsw\nmov bx,ax\nshr bx,1\nshr bx,1\nshr bx,1\nshr bx,1\nmov ah,es:[bx]\nstosb',
'packed21':'lodsw\nmov ch,ah\nmov bx,ax\nrol bx,1\nrol bx,1\nrol bx,1\nrol bx,1\nmov ah,bl\nstosw\nmov al,ch\nmov ah,bh\nstosw',
'numeric':'mov di,word ptr [0c0e4h]',
'absolute':'mov di,word ptr ds:[0c0e4h]',
'near':'jmp near ptr dest\nnop\ndest: ret',
'forcedcall':CALL_NEAR+'\nCALL_NEAR dest\nnop\ndest: ret',
'forced':JMP_NEAR+'\nJMP_NEAR dest\nnop\ndest: ret',
'extrns':'extrn a:near\nextrn b:near\ncall a\ncall b\nret',
'grouped':'extrn a,b:near\ncall a\ncall b\nret',
'far':'extrn a:far\ncall a\nret',
'dgroup':'extrn g:word\nmov ax,offset DGROUP:g\nret',
'orgorder':'extrn a:far\norg 5\ncall a\norg 0\ncall a',
}
C={'nearptr':'int f(char near *p) { return *p; }','farptr':'int f(char far *p) { return *p; }',
   'symbolsi':'void f() {\nasm mov si,ax\n}','rawsi':'void f() {\nasm db 08bh,0f0h\n}'}


class PinnedTasmRulesTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory(prefix='rules-',dir=ROOT/'build'); work=Path(cls.tmp.name)
        owners=[]
        for kind,cases,suffix in [('MATCHING_ASM',ASM,'.ASM'),('MATCHING_C',C,'.C')]:
            for name,text in cases.items():
                path=work/(name+suffix)
                path.write_text(PREFIX+text+'\n_TEXT ends\nend\n' if suffix=='.ASM' else text+'\n')
                owners.append({'id':name,'kind':kind,'source':path.relative_to(ROOT).as_posix(),'build':{}})
        lock=read_json(ROOT/'layout/toolchain.json')
        receipts,_=compile_sources(ROOT,owners,work/'compile',ROOT/'toolchain',resolve_runner(lock),lock)
        cls.modules={name:read_object((work/'compile'/r['object']).read_bytes()) for name,r in receipts.items()}

    @classmethod
    def tearDownClass(cls): cls.tmp.cleanup()

    def test_unrolled_patterns_match_every_historical_body(self):
        from runtime_source import oracle_bytes
        binary=oracle_bytes()
        for name,start,end,width in [('planar15',0x307,0x7b7,15),('packed21',0x89f,0xf2f,21)]:
            pattern=self.modules[name].segment_bytes('_TEXT')
            self.assertEqual(len(pattern),width)
            self.assertFalse(self.modules[name].linker_fixups)
            for at in range(start,end,width):
                self.assertEqual(pattern,binary[at:at+width],hex(at))

    def test_numeric_memory_requires_explicit_segment(self):
        self.assertEqual(self.modules['numeric'].segment_bytes('_TEXT').hex(),'bfe4c0')
        self.assertEqual(self.modules['absolute'].segment_bytes('_TEXT').hex(),'8b3ee4c0')

    def test_tasm_shortens_even_near_ptr(self):
        self.assertEqual(self.modules['near'].segment_bytes('_TEXT').hex(),'eb029090c3')
        self.assertEqual(self.modules['forced'].segment_bytes('_TEXT').hex(),'e9010090c3')
        self.assertEqual(self.modules['forcedcall'].segment_bytes('_TEXT').hex(),'e8010090c3')

    def test_extrn_near_far_and_untyped_group_member(self):
        self.assertEqual(self.modules['extrns'].segment_bytes('_TEXT').hex(),'e80000e80000c3')
        self.assertEqual(self.modules['far'].fixups[0]['loc'],'pointer32')
        self.assertEqual(self.modules['grouped'].segment_bytes('_TEXT').hex(),'2eff160000e80000c3')
        self.assertFalse(self.modules['grouped'].fixups[0]['self_relative'])

    def test_dgroup_frame_is_retained_in_full_fixup_contract(self):
        f=self.modules['dgroup'].linker_fixups[0]
        self.assertEqual((f['frame_kind'],f['frame']),('group','DGROUP'))
        self.assertEqual(f['encoded_addend'],'0000')

    def test_org_controls_fixupp_emission_order(self):
        self.assertEqual([f['offset'] for f in self.modules['orgorder'].linker_fixups],[6,1])
        self.assertEqual(self.modules['orgorder'].segment_bytes('_TEXT').hex(),'9a000000009a00000000')

    def test_near_far_pointer_type_changes_actual_code(self):
        self.assertEqual(self.modules['nearptr'].segment_bytes('_TEXT').hex(),'558bec8b5e048a0798eb005dc3')
        self.assertEqual(self.modules['farptr'].segment_bytes('_TEXT').hex(),'558becc45e04268a0798eb005dc3')

    def test_inline_si_changes_compiler_saves(self):
        self.assertEqual(self.modules['symbolsi'].segment_bytes('_TEXT').hex(),'568bf05ec3')
        self.assertEqual(self.modules['rawsi'].segment_bytes('_TEXT').hex(),'8bf0c3')


if __name__=='__main__': unittest.main()
