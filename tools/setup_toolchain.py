"""Copy only the three pinned Borland tools from an existing local installation."""
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
    args = parser.parse_args()
    lock = json.loads((ROOT / 'layout/toolchain.json').read_text())
    for entry in lock['files']:
        source = args.source / entry['path']
        if hashlib.sha256(source.read_bytes()).hexdigest() != entry['sha256']:
            raise SystemExit(f'Wrong toolchain binary: {source}')
    target = ROOT / 'toolchain'
    target.mkdir(exist_ok=True)
    for entry in lock['files']:
        shutil.copyfile(args.source / entry['path'], target / entry['path'])
    print(f'Installed and verified TCC.EXE, CPP.EXE, TASM.EXE in {target}')


if __name__ == '__main__':
    main()
