"""Rename C-level symbols consistently across sources, production ASM and layout metadata.

Usage:
  python tools/rename_symbol.py f8480=dialog_draw f86c9=dialog_run [...]
  python tools/rename_symbol.py --map names.json        # {"f8480": "dialog_draw", ...}
  python tools/rename_symbol.py ... --refresh-index     # also rebuild layout/public-index.json

A symbol is the C identifier (without the leading underscore the compiler adds).
The rename touches: active production C sources and include/*.H (identifier and
the /*@SYM _name=...*/ traceability comments), production TASM sources, the
manifest and structural module bindings/publics, module recipes, and the
address-to-name record docs/current/symbol-names.json. The plan is regenerated.
Object bytes never change: only OMF symbol names do, so the public index has to
be regenerated from a fresh compile session (--refresh-index runs a research
build that is expected to stop at the index check, then collects the index).
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

from reconstruct import read_json, write_json

IDENT = re.compile(r'^[A-Za-z_]\w*$')


def production_sources(plan):
    c_files, asm_files = set(), set()
    for module in plan['modules']:
        sources = [module['source']] if 'source' in module else module.get('sources', [])
        (c_files if module['tool'] == 'TCC.EXE' else asm_files).update(sources)
    return sorted(c_files), sorted(asm_files)


def replace_words(text, mapping):
    """Replace whole identifiers, including their underscore-prefixed OMF spelling."""
    if not mapping:
        return text, 0
    pattern = re.compile(r'(?<![A-Za-z0-9_])_?(' + '|'.join(re.escape(k) for k in mapping) + r')(?![A-Za-z0-9_])')

    def swap(match):
        prefix = '_' if match[0].startswith('_') else ''
        return prefix + mapping[match[1]]
    return pattern.subn(swap, text)


def rewrite_text_file(path, mapping, newline_sensitive=True):
    raw = path.read_bytes()
    text = raw.decode('latin1')
    new, count = replace_words(text, mapping)
    if count:
        path.write_bytes(new.encode('latin1'))
    return count


def rename(mapping, refresh_index=False, root=ROOT):
    for old, new in mapping.items():
        if not IDENT.match(old) or not IDENT.match(new):
            raise SystemExit(f'not a C identifier: {old}={new}')
        if old == new:
            raise SystemExit(f'no-op rename: {old}')
        if len(new) > 31:
            # Turbo C 2.0 keeps 32 significant characters including the OMF underscore.
            raise SystemExit(f'name longer than 31 characters: {new}')
    plan = read_json(root / 'layout/production-plan.json')
    c_files, asm_files = production_sources(plan)
    # Reject names already in use as publics or externs in production sources.
    used = set()
    for f in c_files + asm_files + [p.relative_to(root).as_posix() for p in (root / 'include').glob('*.H')]:
        used.update(re.findall(r'(?<![A-Za-z0-9_])_?([A-Za-z_]\w*)', (root / f).read_text('latin1')))
    clashes = [new for new in mapping.values() if new in used and new not in mapping]
    if clashes:
        raise SystemExit(f'target names already appear in production sources: {clashes}')
    touched = {}
    # Historical wave recipes and their tests mutate/compare the current sources, so
    # they follow the rename too; inactive reference sources keep their names.
    tests = [p.relative_to(root).as_posix() for p in (root / 'tests').glob('*.py')]
    for f in c_files + [p.relative_to(root).as_posix() for p in (root / 'include').glob('*.H')] + asm_files + tests:
        n = rewrite_text_file(root / f, mapping)
        if n:
            touched[f] = n
    # Layout metadata: manifest bindings/publics, structural modules, recipes.
    omf = {'_' + k: '_' + v for k, v in mapping.items()}
    metadata = ['layout/manifest.json', 'layout/structural-source-modules.json']
    metadata += [p.relative_to(root).as_posix() for folder in ('recipes/modules', 'recipes/data', 'recipes/c', 'src/data')
                 for p in (root / folder).glob('*.json')]
    for f in metadata:
        path = root / f
        text = path.read_text('latin1')
        new, n = re.subn(r'"(' + '|'.join(re.escape(k) for k in omf) + r')"',
                         lambda m: '"' + omf[m[1]] + '"', text)
        if n:
            path.write_text(new, 'latin1')
            touched[f] = n
    # Traceability record: new name -> original reconstruction name and address.
    names_path = root / 'docs/current/symbol-names.json'
    record = read_json(names_path) if names_path.exists() else {'format': 'empires-symbol-names-v1', 'names': {}}
    index = {p['symbol'].lstrip('_'): p for p in read_json(root / 'layout/public-index.json')['publics']}
    for old, new in mapping.items():
        previous = record['names'].get(old, {})
        original = previous.get('original', old)
        entry = {'original': original}
        if old in index:
            entry['address'] = f"{index[old]['address']:04X}"
        record['names'].pop(old, None)
        record['names'][new] = entry
    record['names'] = dict(sorted(record['names'].items()))
    write_json(names_path, record)
    subprocess.run([sys.executable, str(root / 'tools/production_plan.py')], check=True, cwd=root)
    if refresh_index:
        subprocess.run([sys.executable, str(root / 'tools/build_exe.py'), '--research'], cwd=root)
        from public_index import collect
        work = max((p for p in (root / 'build').glob('production-*') if (p / 'WORK').exists()),
                   key=lambda p: p.stat().st_mtime)
        write_json(root / 'layout/public-index.json',
                   collect(root, read_json(root / 'layout/production-plan.json'), work))
        touched['layout/public-index.json'] = 1
    return touched


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('pairs', nargs='*', metavar='OLD=NEW')
    parser.add_argument('--map', help='JSON file with {"old": "new", ...}')
    parser.add_argument('--refresh-index', action='store_true')
    args = parser.parse_args(argv)
    mapping = {}
    if args.map:
        mapping.update(json.loads(Path(args.map).read_text()))
    for pair in args.pairs:
        old, new = pair.split('=', 1)
        mapping[old] = new
    if not mapping:
        parser.error('nothing to rename')
    touched = rename(mapping, args.refresh_index)
    for f, n in sorted(touched.items()):
        print(f'{f}: {n}')
    print(f'{len(mapping)} symbols renamed; run full acceptance next')
    return 0


if __name__ == '__main__':
    sys.exit(main())
