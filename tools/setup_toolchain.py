"""Copy pinned Borland tools and libraries from an existing local installation."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--from', dest='source', type=Path,
                        default=Path('D:/Games/DOS/dos_recosystem/empires_forged/toolchain/dos/TC/BIN'))
    parser.add_argument('--lib-from', type=Path, help='Library directory; default is ../LIB next to BIN')
    args = parser.parse_args()
    lock = json.loads((ROOT / 'layout/toolchain.json').read_text())
    inputs = [(entry, args.source / entry['path']) for entry in lock['files']]
    library_dir = args.lib_from or args.source.parent / 'LIB'
    inputs += [(entry, library_dir / entry['path']) for entry in lock.get('libraries', [])]
    for entry, source in inputs:
        if hashlib.sha256(source.read_bytes()).hexdigest() != entry['sha256']:
            raise SystemExit(f'Wrong toolchain binary: {source}')
    target = ROOT / 'toolchain'
    target.mkdir(exist_ok=True)
    for entry, source in inputs:
        shutil.copyfile(source, target / entry['path'])
    print(f'Installed and verified {len(inputs)} pinned toolchain files in {target}')


if __name__ == '__main__':
    main()
