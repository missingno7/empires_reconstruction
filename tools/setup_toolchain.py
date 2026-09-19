"""Copy pinned Borland tools and libraries from an existing local installation."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--from', dest='source', type=Path, required=True,
                        help='local Turbo C 2.0 BIN directory')
    parser.add_argument('--lib-from', type=Path, help='Library directory; default is ../LIB next to BIN')
    parser.add_argument('--linker-from', type=Path,
                        help='local pinned Turbo Link 2.0 executable; defaults to TLINK.EXE in --from')
    args = parser.parse_args()
    lock = json.loads((ROOT / 'layout/toolchain.json').read_text())
    inputs = [(entry, args.source / entry['path']) for entry in lock['files']]
    library_dir = args.lib_from or args.source.parent / 'LIB'
    inputs += [(entry, library_dir / entry['path']) for entry in lock.get('libraries', [])]
    inputs += [(entry, library_dir / entry.get('source', entry['path']))
               for entry in lock.get('objects', [])]
    linker_sources = []
    for entry, source in inputs:
        if hashlib.sha256(source.read_bytes()).hexdigest() != entry['sha256']:
            raise SystemExit(f'Wrong toolchain binary: {source}')
    for entry in lock.get('linkers', []):
        source = args.linker_from or args.source / entry['path']
        if not source.exists():
            raise SystemExit(f'Missing pinned linker: {source}; pass --linker-from to its local location')
        if hashlib.sha256(source.read_bytes()).hexdigest() != entry['sha256']:
            raise SystemExit(f'Wrong linker binary: {source}')
        linker_sources.append((entry, source))
    target = ROOT / 'toolchain'
    target.mkdir(exist_ok=True)
    for entry, source in inputs:
        shutil.copyfile(source, target / entry['path'])
    installed = len(inputs)
    for entry, source in linker_sources:
        shutil.copyfile(source, target / entry['path'])
        installed += 1
        print(f'Installed verified local linker {entry["path"]} ({entry["version_banner"]})')
    print(f'Installed and verified {installed} pinned toolchain files in {target}')


if __name__ == '__main__':
    main()
