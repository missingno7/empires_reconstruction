"""Replace individually staged objects with one verified shared C compilation."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

from omf import OmfReader
from omf_scaffold import _record, _records, add_publics, normalize_external_case
from probe_module_group import probe
from probe_tlink_layout import parse_map, link_errors, compare_linked_executable
from reconstruct import ROOT, read_json, sha, write_json


def run(recipe_path):
    baseline = read_json(ROOT / 'build/tlink-structural-report.json')
    if baseline['status'] != 'MAP_AVAILABLE' or baseline['link'].get('errors'):
        raise ValueError('Shared replacement requires a successful baseline link')
    recipe = read_json(recipe_path)
    proof = probe(recipe_path)
    original_work = Path(baseline['byte_comparison']['candidate']).parent.parent
    work = Path(tempfile.mkdtemp(prefix='shared-link-', dir=ROOT / 'build')).resolve()
    for name in ('TC', 'BC', 'WORK'):
        shutil.copytree(original_work / name, work / name)
    if sha((work / 'BC/BIN/TLINK.EXE').read_bytes()) != baseline['linker']['sha256']:
        raise ValueError('Staged TLINK identity differs from baseline receipt')
    for name in ('OUT.EXE', 'OUT.MAP', 'LINK.LOG'):
        (work / 'WORK' / name).unlink(missing_ok=True)
    staged = {s['owner']: s for s in baseline['relocatable_scaffold'] if s.get('owner')}
    selected = [staged[s['owner']] for s in recipe['sources']]
    data = Path(proof['object_path']).read_bytes()
    module = OmfReader().read(data)
    explicit = set()
    for path in (work / 'WORK').glob('*.OBJ'):
        explicit.update(p['name'] for p in OmfReader().read(path.read_bytes()).publics)
    data = normalize_external_case(data, explicit)
    labels = {}
    for source, entry in zip(recipe['sources'], selected):
        base = next(p['offset'] for p in module.publics if p['name'] == source['public'])
        old = OmfReader().read((work / 'WORK' / entry['object']).read_bytes())
        for public in old.publics_in('_TEXT'):
            labels[public['name']] = base + public['offset']
    data = add_publics(data, labels)
    # A unique module name makes the shared contribution identifiable in TLINK's map.
    data = b''.join(_record(kind, b'\x06RGROUP' if kind == OmfReader.THEADR else body)
                    for kind, body in _records(data))
    (work / 'WORK/GROUP.OBJ').write_bytes(data)
    response = (original_work / 'LINK.RSP').read_text()
    for index, entry in enumerate(selected):
        token = 'C:\\WORK\\' + entry['object']
        if response.count(token) != 1:
            raise ValueError('Expected exactly one staged object in baseline response')
        response = response.replace(token, 'C:\\WORK\\GROUP.OBJ' if index == 0 else '')
    while '++' in response:
        response = response.replace('++', '+')
    response = response.replace('+,', ',')
    (work / 'LINK.RSP').write_text(response)
    shutil.copyfile(original_work / 'GO.BAT', work / 'GO.BAT')
    config = work / 'run.conf'
    config.write_text('[sdl]\noutput=texture\n[mixer]\nnosound=true\n[autoexec]\n'
                      f'mount c "{work}"\nc:\ncall c:\\GO.BAT\nexit\n')
    dosbox = baseline['link']['command'][0]
    subprocess.run([dosbox, '-conf', str(config), '--noprimaryconfig', '-noconsole', '-exit'],
                   cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120, check=True)
    map_path = work / 'WORK/OUT.MAP'
    text = map_path.read_text(errors='replace')
    errors = link_errors((work / 'WORK/LINK.LOG').read_text(errors='replace'), text)
    segments, rows = parse_map(map_path)
    shared = next(r for r in rows if r['module'] == 'RGROUP')
    base_rows = {r['module']: r for r in baseline['code_rows']}
    selected_names = {Path(e['object']).stem + '.C' for e in selected}
    downstream = [r for r in rows if r['module'] != 'RGROUP' and r['module'] in base_rows
                  and r != base_rows[r['module']]]
    first_base = base_rows[Path(selected[0]['object']).stem + '.C']['offset']
    expected_names = (set(base_rows) - selected_names) | {'RGROUP'}
    code_equal = (set(r['module'] for r in rows) == expected_names
                  and not downstream and shared['offset'] == first_base
                  and shared['length'] == proof['text_bytes'])
    report = {'status': 'CODE_PLACEMENT_EQUAL' if code_equal and not errors else 'DIVERGED',
              'candidate': recipe['id'], 'historical_module_proven': False,
              'code_bytes': proof['text_bytes'], 'data_evidence': proof['data_evidence'],
              'objects_replaced': len(selected), 'objects_added': 1,
              'shared_contribution': shared, 'downstream_code_divergences': downstream,
              'errors': errors, 'segments': segments,
              'byte_comparison': compare_linked_executable(work / 'WORK/OUT.EXE', ROOT / 'assets/AEPROG.EXE'),
              'source_object_sha256': sha(Path(proof['object_path']).read_bytes()),
              'staged_object_sha256': sha(data),
              'limitation': 'Real shared C object in full TLINK experiment; existing DATA/BSS and symbol scaffolds remain.'}
    write_json(ROOT / 'build/shared-module-link-report.json', report)
    print(f"Shared module TLINK: {report['status']}; downstream code divergences {len(downstream)}")
    if report['status'] != 'CODE_PLACEMENT_EQUAL':
        raise ValueError('Shared module changed code placement or introduced linker errors')
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipe', type=Path, default=ROOT / 'recipes/modules/C_75F3_7856.json')
    run(parser.parse_args().recipe)
