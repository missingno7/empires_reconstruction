"""Blocked byte ownership survives queue regrouping."""
import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from reconstruction_factory import candidates, blocked_ranges
from runtime_cfg import recursive_cfg
from check_candidate import block, snapshot_bytes
from reconstruct import sha
import base64

class QueueFailureTests(unittest.TestCase):
    def test_legacy_failure_survives_neighbor_recovery(self):
        cfg=recursive_cfg(bytes.fromhex('9090909090c3'),[{'offset':0,'names':['test']}])
        m={'ranges':[{'start':n,'end':n+1,'line':n+1,'category':'raw_unresolved'} for n in range(6)],'publics':[],'fixups':[],'source_sha256':'test'}
        state={'RUNTIME_BLOCK:0002-0004':{'status':'BLOCKED_SUPERVISOR','reason':'example'}}
        for prefix in (0,1,2):
            for row in m['ranges']:
                row['category']='symbolic_instruction' if row['start']<prefix else 'raw_unresolved'
            cards=candidates(m,cfg,state)
            failed=[c for c in cards if c['difficulty']=='SUPERVISOR']
            self.assertEqual([c['range'] for c in failed],[[2,4]])
            self.assertEqual(failed[0]['blocked_attempts'],['RUNTIME_BLOCK:0002-0004'])
            self.assertFalse(any(c['difficulty']=='CHEAP' and c['range'][0]<4 and c['range'][1]>2 for c in cards))
        m['ranges']=[{'start':0,'end':6,'line':1,'category':'raw_unresolved'}]
        self.assertEqual(candidates(m,cfg,state)[0]['difficulty'],'SUPERVISOR')

    def test_snapshot_preserves_lf_crlf_and_mixed_endings(self):
        for data in (b'a\nb\n',b'a\r\nb\r\n',b'a\r\nb\n'):
            baseline={'source_snapshot':'a\nb\n','source_sha256':sha(data),
                      'source_snapshot_base64':base64.b64encode(data).decode()}
            self.assertEqual(snapshot_bytes(baseline),data)
        for data in (b'a\nb\n',b'a\r\nb\r\n'):
            self.assertEqual(snapshot_bytes({'source_snapshot':'a\nb\n','source_sha256':sha(data)}),data)
        with self.assertRaises(ValueError): snapshot_bytes({'source_snapshot':'a\n','source_sha256':'wrong'})

    def test_invalid_block_extent_is_rejected(self):
        with self.assertRaises(ValueError): blocked_ranges({'bad':{'status':'BLOCKED_SUPERVISOR'}})

    def test_block_restores_scope_and_excludes_unrelated_failure(self):
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as directory:
            root=Path(directory)
            for name in ('asm','build','recipes/runtime'): (root/name).mkdir(parents=True,exist_ok=True)
            (root/'asm/RUNTIME_BLOCK.ASM').write_text('before\nchanged\nafter\n')
            (root/'build/candidate-fast.json').write_text(json.dumps({'candidate':'different-task','message':'not relevant'}))
            card={'source':'asm/RUNTIME_BLOCK.ASM','source_lines':[2,2],'range':[2,4]}
            baseline={'source_snapshot':'before\noriginal\nafter\n','source_sha256':sha(b'before\noriginal\nafter\n')}
            with patch('check_candidate.context',return_value=(card,baseline)),patch('reconstruction_factory.refresh'):
                block('RUNTIME_BLOCK:0002-0004','test failure',root)
            state=json.loads((root/'recipes/runtime/task-state.json').read_text())['RUNTIME_BLOCK:0002-0004']
            self.assertEqual(state['range'],[2,4])
            self.assertNotIn('message',state['failure'])
            self.assertEqual((root/'asm/RUNTIME_BLOCK.ASM').read_text(),baseline['source_snapshot'])
            self.assertEqual((root/state['saved_source']).read_text(),'before\nchanged\nafter\n')

if __name__=='__main__': unittest.main()
