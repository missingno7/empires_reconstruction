"""Merge the member sources of a grouped module into one readable source file.

Usage:
  python tools/merge_module.py C_7D91_880A src/DIALOG.C

The members of a grouped module (recipes/modules/<ID>.json) are already compiled
as one translation unit by concatenation. This tool makes that unit a real file:
it writes the members in link order under section banners that keep the original
region ids and addresses, points the recipe and the manifest regions at the new
file, and removes the member files. Bytes do not change: the concatenation the
build compiles is the same text. Run `python tools/production_plan.py` and
`python tools/probe_module.py <ID>` afterwards, then full acceptance.
"""
import argparse
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from reconstruct import read_json, write_json


def banner(owner, address, first):
    text = f"/* ---- {owner} (original code at 0x{address:04X}) ---- */"
    return ('' if first else '\r\n') + text + '\r\n'


def merge(module_id, target, root=ROOT, description=None):
    plan = {m['id']: m for m in read_json(root / 'layout/production-plan.json')['modules']}
    if module_id not in plan or not plan[module_id].get('recipe'):
        raise SystemExit(f'{module_id} is not a grouped module in the plan')
    recipe_path = root / plan[module_id]['recipe']
    recipe = read_json(recipe_path)
    manifest = read_json(root / 'layout/manifest.json')
    regions = {r['id']: r for r in manifest['regions']}
    target = Path(target)
    target_rel = target.as_posix() if not target.is_absolute() else target.relative_to(root).as_posix()
    if (root / target_rel).exists():
        raise SystemExit(f'{target_rel} already exists')
    parts = []
    seen = []
    for index, entry in enumerate(recipe['sources']):
        path = entry['path']
        if path in seen:
            continue
        seen.append(path)
        region = regions[entry['owner']]
        text = (root / path).read_bytes().decode('latin1').replace('\r\n', '\n').replace('\r', '\n')
        if re.search(r'^/\* ---- ' + re.escape(entry['owner']) + r' \(original code at', text, re.M):
            # An already merged member file: keep its own section banners and
            # drop its file head so its sections nest into the larger unit.
            head = re.match(r'/\* ' + re.escape(path) + r':.*?\*/\s*', text, re.S)
            if head:
                text = text[head.end():]
            parts.append(text.rstrip('\n') + '\n')
            continue
        parts.append(banner(entry['owner'], region['start'] - 512, index == 0).replace('\r\n', '\n') + text.rstrip('\n') + '\n')
    head = (f'/* {target_rel}: {description or module_id}.\n'
            f'   One translation unit; the sections below were the separate member\n'
            f'   sources of grouped module {module_id} and keep their original ids. */\n\n')
    (root / target_rel).write_bytes((head + '\n'.join(parts)).encode('latin1'))
    for entry in recipe['sources']:
        entry['path'] = target_rel
    recipe['evidence'] = recipe.get('evidence', '') + f' Members merged into {target_rel}.'
    write_json(recipe_path, recipe)
    for entry in recipe['sources']:
        regions[entry['owner']]['source'] = target_rel
    write_json(root / 'layout/manifest.json', manifest)
    for path in seen:
        (root / path).unlink()
    return target_rel, seen


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('module')
    parser.add_argument('target')
    parser.add_argument('--description')
    args = parser.parse_args(argv)
    target, removed = merge(args.module, args.target, description=args.description)
    print(f'{target}: merged {len(removed)} member files; regenerate the plan and probe {args.module}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
