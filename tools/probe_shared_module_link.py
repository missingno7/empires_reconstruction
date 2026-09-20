"""Replace individually staged objects with one verified shared C compilation."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

from omf import OmfReader
from omf_scaffold import (_record, _records, add_publics, normalize_external_case,
                          externalize_data_segment, order_explicit_fixupp_subrecords)
from mz import MZ
from probe_module_group import probe
from probe_tlink_layout import (parse_map, link_errors, compare_linked_executable,
                                comparison_not_requested)
from reconstruct import ROOT, read_json, sha, write_json
from dos_runner import DosRunner


def run(recipe_path, source_data=False, input_report_path=None, verify=True, runner=None):
    baseline = read_json(ROOT / 'build/tlink-structural-report.json')
    if baseline['status'] != 'MAP_AVAILABLE' or baseline['link'].get('errors'):
        raise ValueError('Shared replacement requires a successful baseline link')
    recipe = read_json(recipe_path)
    if runner is None:
        runner_info = baseline.get('runner') or baseline.get('compile', {}).get('session', {}).get('runner')
        runner = (DosRunner('msdos-player', Path(runner_info['path']))
                  if runner_info and runner_info.get('backend') == 'msdos-player' else None)
    proof = probe(recipe_path, runner=runner, verify=verify)
    input_report = (read_json(input_report_path) if input_report_path else
                    read_json(ROOT / 'build/source-data-link-report.json') if source_data else baseline)
    original_work = Path(input_report['byte_comparison']['candidate']).parent.parent
    comparison_rows = parse_map(original_work / 'WORK/OUT.MAP')[1]
    group_name = 'RG' + sha(recipe_path.read_bytes())[:6].upper()
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
    fixupp_adapter = recipe.get('fixupp_order_adapter')
    if fixupp_adapter:
        if fixupp_adapter != {'segment': '_TEXT', 'order': 'descending'}:
            raise ValueError('Unknown FIXUPP order adapter')
        data = order_explicit_fixupp_subrecords(data, '_TEXT', descending=True)
    if source_data and module.segment_length('_DATA'):
        if not proof['data_evidence']:
            raise ValueError('Shared DATA externalization requires proven contiguous ownership')
        manifest = read_json(ROOT / 'layout/manifest.json')
        first_data = next(o for o in manifest['regions'] if o['id'] == proof['data_evidence']['owners'][0])
        data = externalize_data_segment(data, '__DATA_' + first_data['build']['code_owner'])
    # A unique module name makes the shared contribution identifiable in TLINK's map.
    data = b''.join(_record(kind, bytes([len(group_name)]) + group_name.encode('ascii') if kind == OmfReader.THEADR else body)
                    for kind, body in _records(data))
    (work / 'WORK' / (group_name + '.OBJ')).write_bytes(data)
    response = (original_work / 'LINK.RSP').read_text()
    dos_paths = 'C:\\WORK\\' in response
    for index, entry in enumerate(selected):
        token = ('C:\\WORK\\' if dos_paths else '') + entry['object']
        if response.count(token) != 1:
            raise ValueError('Expected exactly one staged object in baseline response')
        response = response.replace(token, (('C:\\WORK\\' if dos_paths else '') + group_name + '.OBJ') if index == 0 else '')
    while '++' in response:
        response = response.replace('++', '+')
    response = response.replace('+,', ',')
    (work / 'LINK.RSP').write_text(response)
    runner_info = baseline.get('runner') or baseline.get('compile', {}).get('session', {}).get('runner')
    if runner_info and runner_info.get('backend') == 'msdos-player':
        runner = DosRunner('msdos-player', Path(runner_info['path']))
        direct_response = response.replace('C:\\TC\\LIB\\', '..\\TC\\LIB\\').replace('C:\\WORK\\', '')
        (work / 'LINK.RSP').write_text(direct_response)
        result, _, _ = runner.run(work / 'BC/BIN/TLINK.EXE', ['@..\\LINK.RSP'], work / 'WORK', timeout=120,
                                  log_path=work / 'WORK/LINK.LOG')
        if result.returncode:
            raise ValueError('MS-DOS Player TLINK shared-module link failed')
    else:
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
    shared = next(r for r in rows if r['module'] == group_name)
    base_rows = {r['module']: r for r in comparison_rows}
    selected_names = {Path(e['object']).stem + '.C' for e in selected}
    downstream = [r for r in rows if r['module'] != group_name and r['module'] in base_rows
                  and r != base_rows[r['module']]]
    first_base = base_rows[Path(selected[0]['object']).stem + '.C']['offset']
    expected_names = (set(base_rows) - selected_names) | {group_name}
    code_equal = (set(r['module'] for r in rows) == expected_names
                  and not downstream and shared['offset'] == first_base
                  and shared['length'] == proof['text_bytes'])
    linked_bytes = (work / 'WORK/OUT.EXE').read_bytes()
    def group_relocations(blob):
        return [r['load_offset'] for r in MZ.parse(blob).relocations
                if shared['offset'] <= r['load_offset'] < shared['offset'] + shared['length']]
    expected_relocations = [MZ.linear(item['segment'], item['offset'])
                            for item in read_json(ROOT / 'layout/mz-header.json')['relocations']
                            if shared['offset'] <= MZ.linear(item['segment'], item['offset']) < shared['offset'] + shared['length']]
    actual_relocations = group_relocations(linked_bytes)
    oracle_relocations = [(item['segment'], item['offset'])
                          for item in read_json(ROOT / 'layout/mz-header.json')['relocations']]
    linked_relocations = [(item['segment'], item['offset'])
                          for item in MZ.parse(linked_bytes).relocations]
    relocation_prefix = next((i for i, (actual, expected) in
                              enumerate(zip(linked_relocations, oracle_relocations))
                              if actual != expected),
                             min(len(linked_relocations), len(oracle_relocations)))
    report = {'status': 'CODE_PLACEMENT_EQUAL' if code_equal and not errors else 'DIVERGED',
              'candidate': recipe['id'], 'historical_module_proven': False, 'runner': runner_info,
              'source_data_mode': source_data,
              'group_relocation_order': {'actual': actual_relocations, 'expected': expected_relocations,
                                         'equal': actual_relocations == expected_relocations},
              'matching_relocation_prefix_entries': relocation_prefix,
              'code_bytes': proof['text_bytes'], 'data_evidence': proof['data_evidence'],
              'objects_replaced': len(selected), 'objects_added': 1,
              'shared_contribution': shared, 'downstream_code_divergences': downstream,
              'errors': errors, 'segments': segments,
              'byte_comparison': (compare_linked_executable(work / 'WORK/OUT.EXE', ROOT / 'assets/AEPROG.EXE')
                                  if verify else comparison_not_requested(work / 'WORK/OUT.EXE')),
              'source_object_sha256': sha(Path(proof['object_path']).read_bytes()),
              'staged_object_sha256': sha(data),
              'fixupp_order_adapter': fixupp_adapter,
              'limitation': 'Real shared C object in full TLINK experiment; existing DATA/BSS and symbol scaffolds remain.'}
    report_path = 'shared-source-data-link-report.json' if source_data else 'shared-module-link-report.json'
    write_json(ROOT / 'build' / report_path, report)
    write_json(ROOT / 'build' / (Path(report_path).stem + '_' + recipe['id'] + '.json'), report)
    print(f"Shared module TLINK: {report['status']}; downstream code divergences {len(downstream)}")
    if report['status'] != 'CODE_PLACEMENT_EQUAL':
        raise ValueError('Shared module changed code placement or introduced linker errors')
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--recipe', type=Path, default=ROOT / 'recipes/modules/C_75F3_7856.json')
    parser.add_argument('--source-data', action='store_true')
    parser.add_argument('--input-report', type=Path)
    args = parser.parse_args()
    run(args.recipe, args.source_data, args.input_report)
