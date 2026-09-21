import sys
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from reconstruct import read_json
from support_probe import exact
class B99FSymbolicAssemblyTests(unittest.TestCase):
 def test_exact_symbolic_module(self):
  # F_B99F was recovered as exact C (see docs/current/exact-c-recovery.md) and
  # now belongs to the grouped multi-source module C_AF45_C15E declared in
  # layout/production-plan.json; the canonical prover (tools/probe_module.py,
  # via tests/support_probe.py) compiles the whole module's concatenated
  # sources for a member owner, the way the production build does, instead
  # of F_B99F standalone.
  manifest=read_json(ROOT/'layout/manifest.json')
  owner=next(x for x in manifest['regions'] if x['id']=='F_B99F')
  self.assertEqual((owner['kind'],owner['source']),('MATCHING_C','src/LEVEL.C'))
  plan=read_json(ROOT/'layout/production-plan.json')['modules']
  group=next(m for m in plan if m['id']=='C_AF45_C15E')
  self.assertIn('F_B99F', group['members'])
  exact('F_B99F')
if __name__=='__main__':unittest.main()
