"""Rename production source files and update every reference to the old path.

Usage:
  python tools/rename_source.py src/F_3A75.C=src/TURNLOOP.C [...]
  python tools/rename_source.py --map files.json      # {"src/F_3A75.C": "src/TURNLOOP.C"}

Only the path changes: the manifest regions, structural module list, module and
data recipes, the historical wave recipes and the tests that name the file are
rewritten, and the plan is regenerated. Object bytes never change.
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))


def rename(mapping, root=ROOT):
    for old, new in mapping.items():
        if not (root / old).is_file():
            raise SystemExit(f'missing source {old}')
        if (root / new).exists():
            raise SystemExit(f'target exists {new}')
        if not re.fullmatch(r'(src/[A-Za-z0-9_]{1,8}\.C|asm/[A-Za-z0-9_]{1,8}\.ASM)', new):
            raise SystemExit(f'target must be src/NAME.C or asm/NAME.ASM with an 8.3-safe name: {new}')
    touched = {}
    files = [p for folder in ('layout', 'recipes', 'src/data', 'tests', 'docs/current')
             for p in (root / folder).rglob('*') if p.suffix in ('.json', '.py', '.md')]
    pattern = re.compile('|'.join(re.escape(k) for k in mapping))
    for path in files:
        text = path.read_bytes().decode('latin1')
        new, count = pattern.subn(lambda m: mapping[m[0]], text)
        if count:
            path.write_bytes(new.encode('latin1'))
            touched[path.relative_to(root).as_posix()] = count
    for old, new in mapping.items():
        (root / old).rename(root / new)
    subprocess.run([sys.executable, str(root / 'tools/production_plan.py')], check=True, cwd=root)
    return touched


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('pairs', nargs='*', metavar='OLD=NEW')
    parser.add_argument('--map')
    args = parser.parse_args(argv)
    mapping = json.loads(Path(args.map).read_text()) if args.map else {}
    for pair in args.pairs:
        old, new = pair.split('=', 1)
        mapping[old] = new
    if not mapping:
        parser.error('nothing to rename')
    touched = rename(mapping)
    for f, n in sorted(touched.items()):
        print(f'{f}: {n}')
    print(f'{len(mapping)} source files renamed; probe the modules and run acceptance')
    return 0


if __name__ == '__main__':
    sys.exit(main())
