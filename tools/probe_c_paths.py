"""Read-only compiler-path probes for existing ordinary-C arithmetic candidates.

Writes evidence only. Byte equality never authorizes module promotion.
"""
import copy
import tempfile
from pathlib import Path
from reconstruct import ROOT, read_json, write_json, compile_sources, read_object, bind_region, sha
from dos_runner import resolve_runner
from mz import MZ
from omf import OmfReader

CANDIDATES = ('F_DDD9', 'F_DE7E', 'F_DEFA')

def probe(root=ROOT):
    manifest = read_json(root / 'layout/manifest.json')
    owners = {o['id']: o for o in manifest['regions']}
    lock = read_json(root / 'layout/toolchain.json')
    fixture = (root / 'assets/AEPROG.EXE').read_bytes()
    mz = MZ.parse(fixture)
    library = {name: read_object(blob) for name, blob in OmfReader().split_library((root / 'toolchain/CC.LIB').read_bytes())}
    components = {o['id']: library[o['build']['library_module']] for o in owners.values()
                  if o.get('kind') == 'KNOWN_TOOLCHAIN_LIBRARY' and o['build'].get('library_module') in library}
    results = []
    for flags in ('', '-B'):
        units = [copy.deepcopy(owners[name]) for name in CANDIDATES]
        for unit in units:
            unit['source'] = 'src/' + unit['id'] + '.C'
            unit['kind'] = 'MATCHING_C'
            unit['build']['flags_append'] = flags
        work = Path(tempfile.mkdtemp(prefix='c-path-', dir=root / 'build'))
        receipts, _ = compile_sources(root, units, work, root / 'toolchain', resolve_runner(lock), lock)
        for unit in units:
            obj = read_object((work / receipts[unit['id']]['object']).read_bytes())
            expected = fixture[unit['start']:unit['end']]
            base = unit['start'] - mz.header_size
            expected_relocs = [r['load_offset'] - base for r in mz.relocations
                               if base <= r['load_offset'] < base + len(expected)]
            result = {'id': unit['id'], 'source': unit['source'], 'source_sha256': sha((root / unit['source']).read_bytes()),
                      'flags_append': flags, 'expected_bytes': len(expected), 'actual_bytes': obj.segment_length('_TEXT'),
                      'expected_relocations': expected_relocs, 'publics': obj.publics_in('_TEXT'),
                      'fixups': obj.fixups_in('_TEXT'), 'promotion': 'HELD_SHARED_MODULE'}
            try:
                actual, proof = bind_region(unit, obj, mz, manifest['frames'], list(owners.values()), components)
                result.update(bytes_equal=actual == expected,
                              actual_relocations=[r-base for r in proof['load_relocations']],
                              relocation_order_equal=proof['load_relocations'] == [base+r for r in expected_relocs])
            except ValueError as error:
                result.update(bytes_equal=False, error=str(error))
            results.append(result)
    report = {'format': 'empires-c-path-probes-v1', 'results': results,
              'limitation': 'Isolated compiler evidence only. Production M_DDD9_DF98 has four owners and globally ordered relocations; no promotion or production edits.',
              'fixture_sha256': sha(fixture)}
    write_json(root / 'docs/current/c-path-probes.json', report)
    return report

if __name__ == '__main__':
    import json
    report = probe()
    print(json.dumps([{k:v for k,v in r.items() if k not in ('fixups','publics','source_sha256')} for r in report['results']], indent=2))

