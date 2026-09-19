"""Run the complete source-DATA Turbo Link 2.0 experiment and require identity."""
from pathlib import Path

from probe_source_data_link import run as source_data_link
from probe_shared_module_link import run as shared_module_link
from probe_data_interleaving import run as data_interleaving
from reconstruct import ROOT, read_json, write_json


def publish(source, shared_modules, final):
    """Validate and publish an exact-link receipt from freshly built stages."""
    if len(shared_modules) != 7:
        raise ValueError('Exact structural link requires seven shared source-module stages')
    if not source['byte_comparison']['load_image']['equal']:
        raise ValueError('Source DATA link no longer has an exact load image')
    comparison = final['byte_comparison']
    if (final['status'] != 'LAYOUT_PRESERVED'
            or final['matching_relocation_prefix_entries'] != 106
            or not comparison['full_file']['equal']):
        raise ValueError('Complete structural-link experiment is not byte-identical')
    linker = read_json(ROOT / 'layout/toolchain.json')['linkers'][0]
    report = {
        'format': 'empires-exact-structural-link-v1',
        'status': 'BYTE_IDENTICAL_STRUCTURAL_EXPERIMENT',
        'linker': {key: linker[key] for key in ('path', 'version_banner', 'sha256', 'role')},
        'steps': [
            {'name': 'source_data', 'status': source['status']},
            *({'name': shared['candidate'], 'status': shared['status'],
               **({'fixupp_order_adapter': shared['fixupp_order_adapter']}
                  if shared.get('fixupp_order_adapter') else {})}
              for shared in shared_modules),
            {'name': 'data_code_interleaving', 'status': final['status'],
             'matching_relocation_prefix_entries': final['matching_relocation_prefix_entries']},
        ],
        'byte_comparison': {key: value for key, value in comparison.items()
                            if key not in ('candidate', 'oracle')},
        'remaining_adapters': [
            'candidate DATA/code object interleaving',
            'arithmetic-module FIXUPP subrecord ordering',
            'aggregate TASM BSS remainder and historical storage ownership',
        ],
        'whole_build_reconstruction_complete': False,
        'limitation': ('Byte-identical TLINK output from relocatable inputs with zero raw DATA '
                       'and zero object symbol transforms; object order, aggregate BSS ownership, '
                       'and historical module proof remain.'),
    }
    write_json(ROOT / 'build/exact-structural-link-report.json', report)
    write_json(ROOT / 'docs/exact-structural-link.json', report)
    print('Exact structural TLINK experiment: BYTE IDENTICAL')
    print(comparison['candidate_sha256'])
    return report


def run():
    source = source_data_link()
    recipes = ('C_6C26_6C87.json', 'C_C5D1_C706.json', 'C_C77A_C898.json', 'C_D61C_D79C.json',
               'C_75F3_7856.json', 'C_AD25_AF45.json', 'C_DDD9_E095.json')
    shared_modules, previous = [], None
    for recipe in recipes:
        shared = shared_module_link(ROOT / 'recipes/modules' / recipe, True, previous)
        shared_modules.append(shared)
        previous = ROOT / 'build' / f"shared-source-data-link-report_{shared['candidate']}.json"
    final = data_interleaving(previous, ROOT / 'recipes/data/interleaving-candidate.json')
    return publish(source, shared_modules, final)


if __name__ == '__main__':
    run()
