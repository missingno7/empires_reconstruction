"""Verify naturally linked library DATA against pinned objects and the oracle."""
from pathlib import Path

from audit_tlink_data import contributions
from mz import MZ
from reconstruct import ROOT, bind_region, owned_library_modules, read_json, sha, write_json


def verify():
    manifest = read_json(ROOT / 'layout/manifest.json')
    report = read_json(ROOT / 'build/tlink-structural-report.json')
    original = (ROOT / manifest['original']['path']).read_bytes()
    if sha(original) != manifest['original']['sha256']:
        raise ValueError('Oracle identity differs')
    linked_path = Path(report['byte_comparison']['candidate'])
    linked = linked_path.read_bytes()
    if sha(linked) != report['outputs']['exe_sha256']:
        raise ValueError('Linked output differs from receipt')
    rows = contributions(linked_path.with_suffix('.MAP').read_text())
    modules = owned_library_modules(manifest['regions'], ROOT / 'toolchain',
                                    read_json(ROOT / 'layout/toolchain.json'))
    owners = sorted((o for o in manifest['regions'] if o['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY'
                     and o['build']['segment'] == '_DATA'), key=lambda o: o['start'])
    expected_order = [o['build']['library_module'] for o in owners]
    actual_order = [r['module'] for r in rows if r['segment'] == '_DATA' and r['length']
                    and r['module'] in expected_order]
    if actual_order != expected_order:
        raise ValueError('Library DATA contribution order differs')
    entries, previous_end = [], None
    image = MZ.parse(linked).load_image(linked)
    for owner in owners:
        module = modules[owner['id']]
        row = next(r for r in rows if r['segment'] == '_DATA'
                   and r['module'] == owner['build']['library_module'])
        definition = next(s for s in module.segment_defs if s['name'] == '_DATA')
        alignment = {1: 1, 2: 2, 3: 16, 4: 256, 5: 4}[definition['alignment_code']]
        start = row['load_start']
        if previous_end is not None and start != (previous_end + alignment - 1) // alignment * alignment:
            raise ValueError('Library DATA spacing differs from declared alignment')
        data, proof = bind_region(owner, module, MZ.parse(original), manifest['frames'],
                                  manifest['regions'], modules)
        if (start != owner['start'] - 512 or row['length'] != len(data)
                or original[owner['start']:owner['end']] != data
                or image[start:start + len(data)] != data):
            raise ValueError(f"{owner['id']}: source/oracle/linked DATA differs")
        entries.append({'owner': owner['id'], 'module': row['module'], 'load_start': start,
                        'bytes': len(data), 'alignment': alignment,
                        'module_sha256': owner['build']['module_sha256'],
                        'data_sha256': sha(data), 'fixups_checked': len(proof['fixups'])})
        previous_end = start + len(data)
    result = {'status': 'EQUAL', 'scope': 'runtime DATA source, order, alignment and actual linked bytes',
              'modules': entries, 'data_bytes': sum(e['bytes'] for e in entries),
              'linked_exe_sha256': sha(linked), 'whole_exe_equal': False}
    write_json(ROOT / 'docs/runtime-data-tail.json', result)
    print(f"Runtime DATA: {len(entries)} modules, {result['data_bytes']} source bytes EQUAL")
    return result


if __name__ == '__main__':
    verify()
