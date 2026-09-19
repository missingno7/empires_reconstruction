"""Build AEPROG.EXE from reconstructed sources through Turbo Link 2.0.

This is the canonical structural executable build.  Probe scripts remain the
small, independently useful evidence tools; this entry point owns their order
and creates every receipt in the current invocation.
"""
import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess

from probe_data_interleaving import run as interleave_data
from probe_exact_structural_link import publish as publish_exact_receipt
from probe_shared_module_link import run as link_shared_module
from probe_source_data_link import run as link_source_data
from probe_tlink_layout import run as build_baseline
from mz import MZ
from reconstruct import ROOT, read_json, write_json


ORIGINAL_SHA256 = '1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10'
MODULE_RECIPES = ('C_6C26_6C87.json', 'C_C5D1_C898.json', 'C_D61C_D79C.json',
                  'C_75F3_7856.json', 'C_AD25_AF45.json', 'C_DDD9_E095.json')
TRANSIENT_REPORTS = ('tlink-structural-report.json', 'source-data-link-report.json',
                     'shared-source-data-link-report.json', 'data-interleaving-report.json',
                     'exe-build-report.json')
SESSION_PREFIXES = ('tlink-structural-', 'source-data-', 'shared-link-',
                    'interleave-', 'module-group-')


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


def clear_stale_state(root):
    """Remove only generated structural-link state before a fresh build.

    Archive outputs and unrelated diagnostics remain intact. Each staged
    object below is then created in a new ``build/<prefix>*`` session, so an
    object or receipt from an earlier invocation cannot be selected.
    """
    build = (root / 'build').resolve()
    removed = []
    for name in TRANSIENT_REPORTS + ('AEPROG.EXE',):
        path = build / name
        if path.exists():
            path.unlink()
            removed.append(path.name)
    for recipe in MODULE_RECIPES:
        ident = read_json(root / 'recipes/modules' / recipe)['id']
        path = build / f'shared-source-data-link-report_{ident}.json'
        if path.exists():
            path.unlink()
            removed.append(path.name)
    for path in build.iterdir():
        if path.is_dir() and path.name.startswith(SESSION_PREFIXES):
            if not path.resolve().is_relative_to(build):
                raise ValueError(f'Structural session escapes build directory: {path}')
            shutil.rmtree(path)
            removed.append(path.name)
    return removed


def build(root=ROOT, verify=True, dosbox=None):
    """Run a fresh source -> OMF -> TLINK build and publish ``build/AEPROG.EXE``."""
    (root / 'build').mkdir(exist_ok=True)
    lock, toolchain_files = validate_toolchain(root)
    removed_state = clear_stale_state(root)
    dosbox = Path(dosbox or os.environ.get('DOSBOX', lock['dosbox_default']))

    # This baseline is construction, not a cached input.  Replacing F_F9BE
    # with the identical selected CC.LIB module prevents a duplicate TOUPPER
    # contribution while preserving historical library extraction.
    baseline = build_baseline(root=root, linker=root / 'toolchain/TLINK.EXE',
                              dosbox=dosbox,
                              promote_toupper=True, scaffold_dgroup=True, verify=verify)
    if baseline['status'] != 'MAP_AVAILABLE' or baseline['link']['unresolved_count']:
        raise ValueError('Fresh baseline link failed')
    source = link_source_data(verify=verify)
    if source['status'] != 'LINKED':
        raise ValueError('Source DATA link failed')
    previous = None
    stages, shared_reports = [], []
    for recipe_name in MODULE_RECIPES:
        recipe_path = root / 'recipes/modules' / recipe_name
        shared = link_shared_module(recipe_path, True, previous, verify=verify)
        stages.append({'recipe': recipe_name, 'candidate': shared['candidate'],
                       'fixupp_order_adapter': shared['fixupp_order_adapter']})
        shared_reports.append(shared)
        previous = root / 'build' / f"shared-source-data-link-report_{shared['candidate']}.json"
    final = interleave_data(previous, root / 'recipes/data/interleaving-candidate.json', verify=verify)
    exact_receipt = (publish_exact_receipt(source, shared_reports, final)
                     if verify else {'status': 'NOT_VERIFIED'})
    candidate = Path(final['byte_comparison']['candidate'])
    published = root / 'build/AEPROG.EXE'
    if not candidate.exists():
        raise ValueError('TLINK did not produce its final executable')
    shutil.copyfile(candidate, published)
    linked_mz = MZ.parse(published.read_bytes())
    if len(linked_mz.relocations) != 106:
        raise ValueError(f'TLINK emitted {len(linked_mz.relocations)} relocations, expected 106')

    oracle = root / 'assets/AEPROG.EXE'
    verification = {'performed': False,
                    'reason': ('verification not requested' if not verify
                               else 'original fixture unavailable')}
    if verify:
        if not oracle.exists():
            raise ValueError('Verification requested but assets/AEPROG.EXE is unavailable')
        verification = {'performed': True,
                        'byte_identical': published.read_bytes() == oracle.read_bytes(),
                        'sha256': sha256(published), 'expected_sha256': ORIGINAL_SHA256,
                        'relocation_order_equal': final['matching_relocation_prefix_entries'] == 106}
        if not all((verification['byte_identical'], verification['sha256'] == ORIGINAL_SHA256,
                    verification['relocation_order_equal'])):
            raise ValueError('Structural build differs from the original executable')
    bss_layout = read_json(root / 'src/data/GAME_BSS.json')
    report = {
        'format': 'empires-exe-build-v1', 'status': 'BUILT',
        'output': str(published), 'sha256': sha256(published), 'size': published.stat().st_size,
        'compiler': 'Turbo C 2.0', 'assembler': 'TASM 1.0',
        'linker': lock['linkers'][0], 'toolchain_files': toolchain_files,
        'compiled_source_modules': baseline['compile']['owner_count'],
        'generated_data_components': len(read_json(root / 'recipes/data/game-initialized.json')['components']),
        'bss': {'bytes': bss_layout['length'],
                'publics': len(bss_layout['publics']), 'source': 'src/data/GAME_BSS.json'},
        'unresolved_symbols': baseline['link']['unresolved_count'],
        'relocations': len(linked_mz.relocations), 'shared_module_stages': stages,
        'fresh_build': {'removed_previous_state': removed_state,
                        'final_link_session': str(candidate.parent.parent)},
        'exact_structural_receipt': exact_receipt['status'],
        'remaining_structural_adapters': [
            'candidate DATA/code object interleaving',
            'arithmetic-module FIXUPP subrecord ordering',
            'unpartitioned TASM BSS reserve and historical storage ownership',
        ],
        'fixture_dependency': {
            'assets/AEPROG.EXE': ('optional verification fixture only; construction uses the canonical '
                                  'manifest, MZ header, source-DATA recipes and GAME_BSS metadata'),
        },
        'verification': verification,
    }
    write_json(root / 'build/exe-build-report.json', report)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', nargs='?', choices=('verify',),
                        help='verify the published EXE against assets/AEPROG.EXE (the default)')
    parser.add_argument('--no-verify', action='store_true',
                        help='skip the final published-EXE byte comparison (component proof still needs the fixture)')
    parser.add_argument('--dosbox', type=Path,
                        help='override the DOSBox executable used for the historical toolchain')
    args = parser.parse_args()
    try:
        report = build(verify=not args.no_verify, dosbox=args.dosbox)
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f'FAIL: {error}')
        return 1
    print(f"AEPROG.EXE: {report['status']} {report['sha256']}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
