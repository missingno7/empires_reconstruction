"""Write a grouped-module recipe for a contiguous run of production modules.

Usage:
  python tools/tu_recipe.py FIRST LAST NEW_ID --evidence "..." [--flags ""]

The run FIRST..LAST (module ids or member ids, link order) must have been proven
byte-exact as one translation unit by tools/probe_tu.py with the same --flags.
The recipe lists every member owner in order with its current source path and
public symbol, in the format consumed by tools/production_plan.py (add NEW_ID to
its SHARED tuple, regenerate the plan, then merge the member files with
tools/merge_module.py).  Grouped modules inside the run are dissolved into their
members; their old recipes are deleted and their ids must be removed from SHARED.
"""
import argparse
import sys

from reconstruct import ROOT, read_json, write_json
from probe_tu import module_run


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('first')
    parser.add_argument('last')
    parser.add_argument('new_id')
    parser.add_argument('--evidence', required=True)
    parser.add_argument('--flags', default='')
    args = parser.parse_args(argv)
    plan = read_json(ROOT / 'layout/production-plan.json')
    regions = {r['id']: r for r in read_json(ROOT / 'layout/manifest.json')['regions']}
    run = module_run(plan, args.first, args.last)
    sources, dissolved = [], []
    for module in run:
        if module['tool'] != 'TCC.EXE':
            raise SystemExit(f"{module['id']} is an assembler module; convert it first")
        if module.get('recipe'):
            dissolved.append(module['recipe'])
        for member in module['members']:
            region = regions[member]
            sources.append({'owner': member, 'path': region['source'], 'public': region['build']['public']})
    recipe = {'format': 'empires-c-module-candidate-v1', 'id': args.new_id, 'segment': '_TEXT',
              'flags_append': args.flags, 'sources': sources, 'evidence': args.evidence}
    target = ROOT / f'recipes/modules/{args.new_id}.json'
    if target.exists():
        raise SystemExit(f'{target} exists')
    write_json(target, recipe)
    print(f'wrote {target.relative_to(ROOT).as_posix()}: {len(sources)} members from {len(run)} modules')
    for old in dissolved:
        print(f'dissolves {old} (delete it and drop its id from SHARED)')
    return 0


if __name__ == '__main__':
    sys.exit(main())
