"""Replace oracle-copied initialized DATA with an ordered canonical source sequence.

Temporary binding aliases and BSS scaffold remain. Only the final comparison
opens the original executable; data bytes come from source encoders or OMF.
"""
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile

from data_omf import emit_data
from exe_data import encode_data
from omf import OmfReader
from omf_scaffold import add_publics, externalize_data_segment, make_dgroup_scaffold, rename_external_addend
from pointer_records import FORMAT, compile_records
from probe_tlink_layout import compare_linked_executable, link_errors, parse_map
from reconstruct import ROOT, read_json, sha, write_json


def run():
    (ROOT / 'build/source-data-link-report.json').unlink(missing_ok=True)
    baseline = read_json(ROOT / 'build/tlink-structural-report.json')
    if baseline['status'] != 'MAP_AVAILABLE' or baseline['link'].get('errors'):
        raise ValueError('Successful baseline required')
    manifest = read_json(ROOT / 'layout/manifest.json')
    owners = {o['id']: o for o in manifest['regions']}
    recipe = read_json(ROOT / 'recipes/data/game-initialized.json')
    old_work = Path(baseline['byte_comparison']['candidate']).parent.parent
    work = Path(tempfile.mkdtemp(prefix='source-data-link-', dir=ROOT / 'build')).resolve()
    for name in ('TC', 'BC', 'WORK'):
        shutil.copytree(old_work / name, work / name)
    for name in ('OUT.EXE', 'OUT.MAP', 'LINK.LOG'):
        (work / 'WORK' / name).unlink(missing_ok=True)
    if sha((work / 'BC/BIN/TLINK.EXE').read_bytes()) != baseline['linker']['sha256']:
        raise ValueError('TLINK identity differs from baseline')
    staged = {s['owner']: s for s in baseline['relocatable_scaffold'] if s.get('owner')}
    parts = []
    for spec in recipe['components']:
        refs = []
        if spec['format'] == 'raw-local':
            data = bytearray((ROOT / spec['source']).read_bytes())
            for pointer in spec.get('symbolic_pointer_fields', []):
                if not 0 <= pointer['offset'] <= len(data) - 4:
                    raise ValueError('Raw source pointer lies outside its contribution')
                struct.pack_into('<HH', data, pointer['offset'], pointer['addend'], 0)
                refs.append({'offset': pointer['offset'], 'target': pointer['target']})
            data = bytes(data)
        elif spec['format'] == 'compiled-data':
            code = owners[spec['code_owner']]
            module = OmfReader().read((work / 'WORK' / staged[code['id']]['object']).read_bytes())
            data = module.segment_bytes('_DATA')
            for fixup in module.fixups_in('_DATA'):
                if fixup['loc'] != 'pointer32' or fixup['target_kind'] != 'external' or fixup['self_relative']:
                    raise ValueError('Unsupported compiled DATA reference')
                binding = code['build']['bindings'][fixup['target']]
                if binding.get('coordinate') != 'DGROUP_offset' or 'owner' not in binding:
                    raise ValueError('Compiled pointer needs component ownership')
                if binding.get('addend', 0) or fixup['displacement']:
                    raise ValueError('Compiled pointer addend requires explicit normalization')
                refs.append({'offset': fixup['offset'], 'target': binding['owner']})
        elif spec['format'] == FORMAT:
            data, refs = compile_records(read_json(ROOT / spec['source']))
        else:
            data = encode_data(read_json(ROOT / spec['source']), spec['format'])
        skip = spec.get('skip', 0)
        if skip and refs:
            raise ValueError('Cannot slice pointer contribution')
        data = data[skip:]
        parts.append({'spec': spec, 'data': data, 'refs': refs,
                      'publics': {} if skip else {spec['id']: 0}})

    # Existing fixed binding evidence routes aliases to source components. It
    # supplies no placement directive or padding; TLINK concatenates sources.
    # This adapter remains explicitly temporary until historical declarations
    # and module storage ownership replace the recovered global names.
    frame = manifest['frames']['DGROUP']
    startup_path = work / 'TC/LIB/C0C.OBJ'
    startup = OmfReader().read(startup_path.read_bytes())
    startup_aliases = {}
    runtime_aliases = {}
    library = {name: OmfReader().read(blob) for name, blob in
               OmfReader().split_library((work / 'TC/LIB/CC.LIB').read_bytes())}
    def add_alias(symbol, dgroup_offset):
        if 0 <= dgroup_offset <= startup.segment_length('_DATA'):
            startup_aliases[symbol] = dgroup_offset
            return
        location = frame + 512 + dgroup_offset
        for part in parts:
            owner = owners[part['spec']['id']]
            start = owner['start'] + part['spec'].get('skip', 0)
            if start <= location < owner['end']:
                offset = location - start
                if symbol in part['publics'] and part['publics'][symbol] != offset:
                    raise ValueError('Conflicting component-relative alias')
                part['publics'][symbol] = offset
                return
        for owner in owners.values():
            if (owner['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY' and owner['build']['segment'] == '_DATA'
                    and owner['start'] <= location < owner['end']):
                module = library[owner['build']['library_module']]
                offset = location - owner['start']
                public = max((p for p in module.publics_in('_DATA') if p['offset'] <= offset),
                             key=lambda p: p['offset'])
                runtime_aliases[symbol] = (public['name'], offset - public['offset'])
                return
        raise ValueError(f'No source component for temporary alias {symbol}')

    dg = OmfReader().read((work / 'WORK/DGSCF.OBJ').read_bytes())
    for public in dg.publics_in('_DATA'):
        add_alias(public['name'], public['offset'])
    separated = []
    for owner_id, entry in staged.items():
        path = work / 'WORK' / entry['object']
        module = OmfReader().read(path.read_bytes())
        blob = path.read_bytes()
        for old, (target, delta) in runtime_aliases.items():
            if old in module.externals:
                blob = rename_external_addend(blob, old, target, delta)
        path.write_bytes(blob)
        if entry['kind'] != 'owner' or not module.segment_length('_DATA'):
            continue
        binding = owners[owner_id]['build']['module_segments']['_DATA']
        base = (owners[binding['owner']]['start'] - 512 - frame + binding.get('addend', 0)
                if 'owner' in binding else binding['offset'])
        symbol = '__DATA_' + owner_id
        add_alias(symbol, base)
        for public in module.publics_in('_DATA'):
            add_alias(public['name'], base + public['offset'])
        path.write_bytes(externalize_data_segment(path.read_bytes(), symbol))
        separated.append({'owner': owner_id, 'bytes': module.segment_length('_DATA')})
    bss_publics = {p['name']: ('_BSS', p['offset']) for p in dg.publics_in('_BSS')}
    bss_publics['GAME_BSS'] = ('_BSS', 0)
    startup_path.write_bytes(add_publics(startup_path.read_bytes(), startup_aliases, '_DATA'))
    bss_source = read_json(ROOT / 'src/data/GAME_BSS.json')
    if bss_source['format'] != 'unpartitioned-bss-reserve-v1' or bss_source['alignment'] != 'word':
        raise ValueError('Unsupported game BSS source')
    (work / 'WORK/DGSCF.OBJ').write_bytes(make_dgroup_scaffold(b'', bss_source['length'], bss_publics))
    names, sources = [], []
    for index, part in enumerate(parts):
        name = f'D{index:04}.OBJ'
        blob = emit_data(part['data'], part['publics'], part['refs'], f'D{index:04}')
        (work / 'WORK' / name).write_bytes(blob)
        names.append('C:\\WORK\\' + name)
        sources.append({'owner': part['spec']['id'], 'bytes': len(part['data']),
                        'format': part['spec']['format'], 'pointer_fixups': len(part['refs']),
                        'object_sha256': sha(blob)})
    response = (old_work / 'LINK.RSP').read_text()
    response = response.replace('C:\\WORK\\DGSCF.OBJ', '+'.join(names + ['C:\\WORK\\DGSCF.OBJ']))
    (work / 'LINK.RSP').write_text(response)
    shutil.copyfile(old_work / 'GO.BAT', work / 'GO.BAT')
    config = work / 'run.conf'
    config.write_text('[sdl]\noutput=texture\n[mixer]\nnosound=true\n[autoexec]\n'
                      f'mount c "{work}"\nc:\ncall c:\\GO.BAT\nexit\n')
    subprocess.run([baseline['link']['command'][0], '-conf', str(config), '--noprimaryconfig', '-noconsole', '-exit'],
                   cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120, check=True)
    map_path = work / 'WORK/OUT.MAP'
    errors = link_errors((work / 'WORK/LINK.LOG').read_text(errors='replace'), map_path.read_text(errors='replace'))
    segments, rows = parse_map(map_path)
    report = {'status': 'LINKED' if not errors else 'LINK_FAILED', 'errors': errors,
              'segments': segments, 'code_contributions_equal': rows == baseline['code_rows'],
              'source_contributions': sources, 'separated_data': separated,
              'oracle_copied_initialized_data_bytes': 0,
              'local_raw_source_bytes': sum(s['bytes'] for s in sources if s['format'] == 'raw-local'),
              'synthetic_bss_bytes': bss_source['length'],
              'temporary_startup_data_aliases': startup_aliases,
              'temporary_runtime_data_aliases': runtime_aliases,
              'byte_comparison': compare_linked_executable(work / 'WORK/OUT.EXE', ROOT / 'assets/AEPROG.EXE'),
              'limitation': 'Ordered source DATA; raw sources, recovered aliases, synthetic BSS and module grouping remain.'}
    write_json(ROOT / 'build/source-data-link-report.json', report)
    receipt = dict(report)
    receipt['byte_comparison'] = {key: value for key, value in report['byte_comparison'].items()
                                  if key not in ('candidate', 'oracle')}
    write_json(ROOT / 'docs/source-data-link.json', receipt)
    print(f"Source DATA link: {report['status']}; code unchanged: {report['code_contributions_equal']}")
    print(report['byte_comparison']['load_image'])
    print(report['byte_comparison']['initialized_data'])
    return report


if __name__ == '__main__':
    run()
