import sys
import tempfile
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from mz import MZ
from dos_runner import resolve_runner
from reconstruct import bind_region, compile_sources, mismatch, owned_library_modules, read_json, read_object
class B99FSymbolicAssemblyTests(unittest.TestCase):
 def test_exact_symbolic_module(self):
  # F_B99F was recovered as exact C (see docs/current/exact-c-recovery.md) and
  # now belongs to the grouped multi-source module C_AF45_C15E declared in
  # layout/production-plan.json; compile the whole module's concatenated
  # sources, the way the production build does, instead of F_B99F standalone.
  # Pin the deterministic DOSBox runner explicitly rather than letting the
  # host auto-select a backend (an MS-DOS Player backend can legitimately
  # emit a different byte or two for some switch-table layouts).
  manifest=read_json(ROOT/'layout/manifest.json')
  owner=next(x for x in manifest['regions'] if x['id']=='F_B99F')
  self.assertEqual((owner['kind'],owner['source']),('MATCHING_C','src/LEVEL.C'))
  plan=read_json(ROOT/'layout/production-plan.json')['modules']
  group=next(m for m in plan if m['id']=='C_AF45_C15E')
  self.assertIn('F_B99F', group['members'])
  original=(ROOT/'assets/AEPROG.EXE').read_bytes()
  lock=read_json(ROOT/'layout/toolchain.json')
  modules=owned_library_modules(manifest['regions'],ROOT/'toolchain',lock)
  with tempfile.TemporaryDirectory(dir=ROOT/'build') as temporary:
   work=Path(temporary)
   concat=work/f"{group['id']}.C"
   concat.write_bytes(b'\r\n'.join((ROOT/s).read_bytes() for s in group['sources']))
   compile_owner={'id':group['id'],'kind':'MATCHING_C','source':concat.relative_to(ROOT).as_posix(),
                  'build':{'flags_append':group['build'].get('flags_append','')}}
   receipts,_=compile_sources(ROOT,[compile_owner],work/'compiler',ROOT/'toolchain',
                              resolve_runner(lock),lock)
   module=read_object((work/'compiler'/receipts[group['id']]['object']).read_bytes())
   data,_=bind_region(owner,module,MZ.parse(original),manifest['frames'],manifest['regions'],modules)
  mismatch(original[owner['start']:owner['end']],data,owner)
if __name__=='__main__':unittest.main()
