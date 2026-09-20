"""Build AEPROG.EXE from reconstructed sources through Turbo Link 2.0.

This is the canonical one-link production build. Historical probes remain
independent evidence tools and never provide cached success to this path.
"""
import argparse
import hashlib
from pathlib import Path
import subprocess

from reconstruct import ROOT, read_json
from dos_runner import resolve_runner


ORIGINAL_SHA256 = '1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10'

def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def validate_toolchain(root):
    """Check every locally supplied, pinned Borland build input."""
    lock = read_json(root / 'layout/toolchain.json')
    entries = list(lock['files']) + list(lock.get('libraries', [])) + list(lock.get('objects', []))
    entries += list(lock.get('linkers', []))
    verified = []
    for entry in entries:
        path = root / 'toolchain' / entry['path']
        if not path.exists():
            raise ValueError(f"Missing pinned toolchain component {entry['path']}; run setup_toolchain.py")
        if sha256(path) != entry['sha256']:
            raise ValueError(f"Pinned toolchain component differs: {entry['path']}")
        verified.append(entry['path'])
    return lock, verified



def build(root=ROOT, verify=True, runner=None, dosbox=None, research=False):
    from build_production import build as production_build
    return production_build(root, verify, runner, dosbox, research)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', nargs='?', choices=('verify',),
                        help='verify the published EXE against assets/AEPROG.EXE (the default)')
    parser.add_argument('--research', action='store_true', help='use content-addressed object cache; never acceptance')
    parser.add_argument('--no-verify', action='store_true',
                        help='skip the optional fixture comparison; exact SHA and relocation proof still apply')
    parser.add_argument('--dosbox', type=Path, help='force the DOSBox reference backend')
    parser.add_argument('--msdos-player', type=Path, help='force an MS-DOS Player executable')
    parser.add_argument('--runner', choices=('msdos-player', 'dosbox'), help='select the DOS execution host')
    args = parser.parse_args()
    try:
        runner = resolve_runner(read_json(ROOT / 'layout/toolchain.json'), backend=args.runner, executable=args.msdos_player or args.dosbox)
        report = build(verify=not args.no_verify, runner=runner, research=args.research)
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        for name in ('AEPROG.EXE', 'exe-build-report.json'):
            (ROOT / 'build' / name).unlink(missing_ok=True)
        print(f'FAIL: {error}')
        return 1
    print(f"AEPROG.EXE: {report['status']} {report['sha256']}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
