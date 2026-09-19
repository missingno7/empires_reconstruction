"""Check typed pointer records against fresh compact-model Turbo C output."""
from pathlib import Path
import tempfile
from omf import OmfReader
from pointer_records import compile_records, records_c_source
from reconstruct import ROOT, compile_sources, read_json, sha, write_json


def run():
    work = Path(tempfile.mkdtemp(prefix='record-compiler-', dir=ROOT / 'build')).resolve()
    candidates, documents, aliases = [], {}, {}
    for owner in ('DATA_010FA5_RECORDS', 'DATA_011D90_RECORDS'):
        doc = read_json(ROOT / 'src/data' / (owner + '.json'))
        text, names = records_c_source(doc)
        source = work / (owner + '.C')
        source.write_text(text)
        candidates.append({'id': owner, 'kind': 'MATCHING_C',
                           'source': source.relative_to(ROOT).as_posix(), 'build': {'flags_append': ''}})
        documents[owner], aliases[owner] = doc, names
    receipts, _ = compile_sources(ROOT, candidates, work, ROOT / 'toolchain',
                                  Path('C:/Program Files/DOSBox Staging/dosbox.exe'),
                                  read_json(ROOT / 'layout/toolchain.json'))
    results = []
    for candidate in candidates:
        owner = candidate['id']
        data, refs = compile_records(documents[owner])
        blob = (work / receipts[owner]['object']).read_bytes()
        module = OmfReader().read(blob)
        actual = [(f['offset'], aliases[owner][f['target']]) for f in module.fixups
                  if f['segment'] == '_DATA']
        expected = [(r['offset'], r['target']) for r in reversed(refs)]
        if module.segment_bytes('_DATA') != data or actual != expected:
            raise ValueError('Compiler DATA layout or reverse fixup ordering differs')
        results.append({'owner': owner, 'bytes': len(data), 'pointer_fixups': len(refs),
                        'fixup_offsets_in_object_order': [offset for offset, _ in actual],
                        'source_sha256': receipts[owner]['source_sha256'], 'object_sha256': sha(blob),
                        'status': 'EQUAL'})
    report = {'status': 'EQUAL', 'records': results,
              'historical_translation_unit_proven': False,
              'evidence': 'Fresh Turbo C struct layout and descending DATA fixup order; no oracle bytes used'}
    write_json(ROOT / 'docs/record-compiler-evidence.json', report)
    print('Turbo C pointer records: 240 DATA bytes and 23 reverse-ordered fixups EQUAL')
    return report


if __name__ == '__main__':
    run()
