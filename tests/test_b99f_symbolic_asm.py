import tempfile
import unittest
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from mz import MZ
from reconstruct import bind_region,compile_sources,mismatch,owned_library_modules,read_json,read_object
class B99FSymbolicAssemblyTests(unittest.TestCase):
 def test_exact_symbolic_module(self):
  m=read_json(ROOT/'layout/manifest.json');o=next(x for x in m['regions'] if x['id']=='F_B99F');self.assertEqual((o['kind'],o['source']),('MATCHING_ASM','asm/F_B99F.ASM'));lock=read_json(ROOT/'layout/toolchain.json');image=(ROOT/'assets/AEPROG.EXE').read_bytes();libs=owned_library_modules(m['regions'],ROOT/'toolchain',lock)
  with tempfile.TemporaryDirectory(dir=ROOT/'build') as t:
   r,_=compile_sources(ROOT,[o],Path(t),ROOT/'toolchain',Path(lock['dosbox_default']),lock);module=read_object((Path(t)/r['F_B99F']['object']).read_bytes());data,proof=bind_region(o,module,MZ.parse(image),m['frames'],m['regions'],libs)
  self.assertEqual(module.publics,[{'name':'_fb99f','segment':'_TEXT','offset':0}]);self.assertEqual(module.fixups,[]);mismatch(image[o['start']:o['end']],data,o);self.assertEqual(len(data),1857);self.assertEqual(proof['load_relocations'],[])
if __name__=='__main__':unittest.main()
