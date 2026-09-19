"""Test source-object interleaving constraints through TLINK, never edit an EXE."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

from mz import MZ
from omf import OmfReader
from probe_tlink_layout import (compare_linked_executable, comparison_not_requested,
                                link_errors, parse_map)
from reconstruct import ROOT, read_json, sha, write_json
from dos_runner import DosRunner


def interleave(objects, moves, data_objects, code_objects):
    result = list(objects)
    consumed = 0
    for move in moves:
        stop = next(i for i, (owner, _) in enumerate(data_objects) if owner == move['data_through']) + 1
        if stop <= consumed:
            raise ValueError('DATA intervals must advance in source order')
        moving = [name for _, name in data_objects[consumed:stop]]
        anchor = code_objects[move['after_code']]
        upper = code_objects[move['before_next_relocating_code']]
        if result.index(anchor) >= result.index(upper):
            raise ValueError('Invalid code insertion interval')
        if any(result.count(name) != 1 for name in moving):
            raise ValueError('DATA object missing or duplicated')
        result = [name for name in result if name not in moving]
        at = result.index(anchor) + 1
        result[at:at] = moving
        consumed = stop
    if [name for name in result if name in {n for _, n in data_objects}] != [n for _, n in data_objects]:
        raise ValueError('Interleaving changed DATA contribution order')
    return result


def run(input_path, recipe_path, verify=True):
    output_path = ROOT / 'build/data-interleaving-report.json'
    output_path.unlink(missing_ok=True)
    incoming, recipe = read_json(input_path), read_json(recipe_path)
    if (verify and not incoming['byte_comparison']['load_image']['equal']):
        raise ValueError('Interleaving requires a byte-identical incoming load image')
    baseline = read_json(ROOT / 'build/tlink-structural-report.json')
    data_recipe = read_json(ROOT / 'recipes/data/game-initialized.json')
    old_work = Path(incoming['byte_comparison']['candidate']).parent.parent
    if (verify and sha((old_work / 'WORK/OUT.EXE').read_bytes()) !=
            incoming['byte_comparison']['candidate_sha256']):
        raise ValueError('Incoming linked output differs from receipt')
    work = Path(tempfile.mkdtemp(prefix='interleave-', dir=ROOT / 'build')).resolve()
    for name in ('TC', 'BC', 'WORK'):
        shutil.copytree(old_work / name, work / name)
    for name in ('OUT.EXE', 'OUT.MAP', 'LINK.LOG'):
        (work / 'WORK' / name).unlink(missing_ok=True)
    response = (old_work / 'LINK.RSP').read_text()
    objects_text, remaining = response.split(',', 1)
    objects = objects_text.split('+')
    work_prefix = 'C:\\WORK\\' if 'C:\\WORK\\' in objects_text else ''
    data_objects = [(part['id'], f'{work_prefix}D{i:04}.OBJ') for i, part in enumerate(data_recipe['components'])]
    code_objects = {s['owner']: work_prefix + s['object'] for s in baseline['relocatable_scaffold'] if s.get('owner')}
    owners = {o['id']: o for o in read_json(ROOT / 'layout/manifest.json')['regions']}
    for move in recipe['moves']:
        for key in ('after_code', 'before_next_relocating_code'):
            owner = move[key]
            if code_objects[owner] in objects:
                continue
            # A previous experiment may have combined this owner into a real
            # compiler module. Resolve its public within actual link inputs.
            symbol = owners[owner]['build']['public']
            matches = []
            for token in objects:
                if token.upper().endswith('.OBJ'):
                    object_path = work / 'WORK' / token.rsplit('\\', 1)[-1]
                    # C0C.OBJ is deliberately addressed through ..\\TC\\LIB
                    # in the direct runner response; it is not a staged game
                    # contribution and cannot own a reconstructed public.
                    if not object_path.exists():
                        continue
                    module = OmfReader().read(object_path.read_bytes())
                    if any(p['name'] == symbol for p in module.publics_in('_TEXT')):
                        matches.append(token)
            if len(matches) != 1:
                raise ValueError('Code anchor public is not uniquely owned by a link input')
            code_objects[owner] = matches[0]
    ordered = interleave(objects, recipe['moves'], data_objects, code_objects)
    (work / 'LINK.RSP').write_text('+'.join(ordered) + ',' + remaining)
    runner_info = baseline.get('runner') or baseline.get('compile', {}).get('session', {}).get('runner')
    if runner_info and runner_info.get('backend') == 'msdos-player':
        runner = DosRunner('msdos-player', Path(runner_info['path']))
        direct_response = ('+'.join(ordered) + ',' + remaining).replace('C:\\TC\\LIB\\', '..\\TC\\LIB\\').replace('C:\\WORK\\', '')
        (work / 'LINK.RSP').write_text(direct_response)
        result, _, _ = runner.run(work / 'BC/BIN/TLINK.EXE', ['@..\\LINK.RSP'], work / 'WORK', timeout=120,
                                  log_path=work / 'WORK/LINK.LOG')
        if result.returncode:
            raise ValueError('MS-DOS Player TLINK DATA interleaving link failed')
    else:
        shutil.copyfile(old_work / 'GO.BAT', work / 'GO.BAT')
        config = work / 'run.conf'
        config.write_text('[sdl]\noutput=texture\n[mixer]\nnosound=true\n[autoexec]\n'
                          f'mount c "{work}"\nc:\ncall c:\\GO.BAT\nexit\n')
        subprocess.run([baseline['link']['command'][0], '-conf', str(config), '--noprimaryconfig', '-noconsole', '-exit'],
                       cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120, check=True)
    map_path = work / 'WORK/OUT.MAP'
    errors = link_errors((work / 'WORK/LINK.LOG').read_text(errors='replace'), map_path.read_text(errors='replace'))
    segments, rows = parse_map(map_path)
    old_segments, old_rows = parse_map(old_work / 'WORK/OUT.MAP')
    comparison = (compare_linked_executable(work / 'WORK/OUT.EXE', ROOT / 'assets/AEPROG.EXE')
                  if verify else comparison_not_requested(work / 'WORK/OUT.EXE'))
    oracle = [{'segment': item['segment'], 'offset': item['offset'],
               'load_offset': MZ.linear(item['segment'], item['offset'])}
              for item in read_json(ROOT / 'layout/mz-header.json')['relocations']]
    actual = MZ.parse((work / 'WORK/OUT.EXE').read_bytes()).relocations
    prefix = next((i for i, (a, b) in enumerate(zip(actual, oracle)) if a != b), min(len(actual), len(oracle)))
    report = {'status': 'LAYOUT_PRESERVED' if not errors and segments == old_segments and rows == old_rows else 'DIVERGED',
              'moves': recipe['moves'], 'errors': errors, 'runner': runner_info, 'segment_map_equal': segments == old_segments,
              'code_contributions_equal': rows == old_rows, 'matching_relocation_prefix_entries': prefix,
              'byte_comparison': comparison,
              'historical_module_proven': False,
              'limitation': 'Candidate source-object order; nonrelocating owners do not identify exact historical boundaries'}
    write_json(output_path, report)
    receipt = dict(report)
    # Keep the checked-in evidence host-neutral; the build report retains the
    # concrete runner identity and hash for each actual invocation.
    receipt.pop('runner', None)
    receipt['byte_comparison'] = {key: value for key, value in comparison.items()
                                  if key not in ('candidate', 'oracle')}
    write_json(ROOT / 'docs/data-interleaving.json', receipt)
    print(f"DATA interleaving: {report['status']}; {prefix} leading relocation entries match")
    print(comparison.get('full_file', {'available': False}))
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input-report', type=Path, default=ROOT / 'build/shared-source-data-link-report_RELOC_F_DDD9_F_DF98.json')
    parser.add_argument('--recipe', type=Path, default=ROOT / 'recipes/data/interleaving-candidate.json')
    args = parser.parse_args()
    run(args.input_report, args.recipe)
