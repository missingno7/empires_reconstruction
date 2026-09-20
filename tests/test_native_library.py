"""Whole-object library packaging must preserve code, fixups and index targets."""
import sys,struct,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from link_support import replace_library_member
from omf import OmfReader
from omf_scaffold import _record

class NativeLibraryTests(unittest.TestCase):
    def test_dictionary_targets_and_member_bytes_survive_member_growth(self):
        lib=Path('toolchain/CC.LIB').read_bytes()
        original=dict(OmfReader().split_library(lib))['RAND']
        # Add a valid inert COMENT before MODEND, forcing archive page movement.
        cursor=0
        while original[cursor] not in (0x8a,0x8b):
            cursor+=3+struct.unpack_from('<H',original,cursor+1)[0]
        native=original[:cursor]+_record(0x88,b'\x00\x00'+b'padding metadata'*8)+original[cursor:]
        packed=replace_library_member(lib,'RAND',native)
        before=OmfReader().split_library(lib);after=OmfReader().split_library(packed)
        self.assertEqual([b for _,b in after],[native if n=='RAND' else b for n,b in before])
        def dictionary_targets(data):
            page=struct.unpack_from('<H',data,1)[0]+3
            starts={};at=page
            for name,blob in OmfReader().split_library(data):
                starts[at//page]=name;at+=((len(blob)+page-1)//page)*page
            pos,count=struct.unpack_from('<IH',data,3);result={}
            for base in range(pos,pos+512*count,512):
                for bucket in data[base:base+37]:
                    if not bucket:continue
                    loc=base+bucket*2;size=data[loc]
                    name=data[loc+1:loc+1+size]
                    number=struct.unpack_from('<H',data,loc+1+size)[0]
                    result[name]=starts[number]
            return result
        self.assertEqual(dictionary_targets(lib),dictionary_targets(packed))

    def test_different_public_identity_is_rejected(self):
        lib=Path('toolchain/CC.LIB').read_bytes()
        other=next(b for name,b in OmfReader().split_library(lib) if name!='RAND')
        with self.assertRaisesRegex(ValueError,'public identity'):
            replace_library_member(lib,'RAND',other)

if __name__=='__main__':unittest.main()
