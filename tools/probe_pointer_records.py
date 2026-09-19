"""Link canonical pointer records and strings with TLINK, without the game oracle."""
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

from exe_data import encode_data
from mz import MZ
from omf_scaffold import make_dgroup_scaffold
from pointer_records import bind_records, compile_records, records_object
from probe_tlink_layout import link_errors
from reconstruct import ROOT, read_json, sha, write_json


def run():
    lock = read_json(ROOT / 'layout/toolchain.json')
    linker = ROOT / 'toolchain/TLINK.EXE'
    if sha(linker.read_bytes()) != next(p['sha256'] for p in lock['linkers'] if p['path'] == 'TLINK.EXE'):
        raise ValueError('TLINK identity differs from pinned historical linker')
    manifest = read_json(ROOT / 'layout/manifest.json')
    owners = {o['id']: o for o in manifest['regions']}
    owner = owners['DATA_011D90_RECORDS']
    document = read_json(ROOT / owner['source'])
    _, refs = compile_records(document)
    work = Path(tempfile.mkdtemp(prefix='pointer-link-', dir=ROOT / 'build')).resolve()
    shutil.copyfile(linker, work / 'TLINK.EXE')
    names = []
    for index, target in enumerate(dict.fromkeys(r['target'] for r in refs)):
        source_owner = owners[target]
        data = encode_data(read_json(ROOT / source_owner['source']), source_owner['build']['encoder'])
        name = f'D{index:04}.OBJ'
        (work / name).write_bytes(make_dgroup_scaffold(data, 0, {target: ('_DATA', 0)}, f'D{index:04}'))
        names.append(name)
    (work / 'TABLE.OBJ').write_bytes(records_object(document, owner['id']))
    (work / 'LINK.RSP').write_text('/s ' + '+'.join(names + ['TABLE.OBJ']) + ',OUT.EXE,OUT.MAP,', encoding='ascii')
    (work / 'GO.BAT').write_bytes(b'@echo off\r\ntlink @LINK.RSP > LINK.LOG\r\n')
    config = work / 'run.conf'
    config.write_text('[sdl]\noutput=texture\n[mixer]\nnosound=true\n[autoexec]\n'
                      f'mount c "{work}"\nc:\ncall GO.BAT\nexit\n')
    dosbox = os.environ.get('DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')
    subprocess.run([dosbox, '-conf', str(config), '--noprimaryconfig', '-noconsole', '-exit'],
                   cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120, check=True)
    log = (work / 'LINK.LOG').read_text(errors='replace')
    map_text = (work / 'OUT.MAP').read_text(errors='replace')
    errors = link_errors(log, map_text)
    if errors:
        raise ValueError(errors)
    publics = {name.upper(): (int(offset, 16), int(frame, 16))
               for frame, offset, name in re.findall(r'^\s*([0-9A-F]+):([0-9A-F]+)\s+(\S+)\s*$', map_text, re.M)}
    linked = (work / 'OUT.EXE').read_bytes()
    mz = MZ.parse(linked)
    offset, frame = publics[owner['id'].upper()]
    start = frame * 16 + offset
    expected = bind_records(document, lambda target: publics[target.upper()])
    if mz.load_image(linked)[start:start + len(expected)] != expected:
        raise ValueError('Linked record pointers differ from emergent public addresses')
    sites = sorted(r['load_offset'] for r in mz.relocations)
    if sites != [start + r['offset'] + 2 for r in refs]:
        raise ValueError('Linked relocation sites differ from source pointers')
    report = {'status': 'EQUAL', 'scope': 'isolated relocatable source test, not full-game integration',
              'linker_sha256': sha((work / 'TLINK.EXE').read_bytes()),
              'record_bytes': len(expected), 'pointer_fixups': len(refs),
              'relocation_sites': sites, 'linked_exe_sha256': sha(linked),
              'link_log': log, 'source_only': True, 'uses_original_exe': False}
    write_json(ROOT / 'docs/pointer-record-link.json', report)
    print(f"Source-only TLINK pointer table: {len(expected)} bytes and {len(refs)} relocations EQUAL")


if __name__ == '__main__':
    run()
