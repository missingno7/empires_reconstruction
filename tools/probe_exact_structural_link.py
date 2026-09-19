"""Run the complete source-DATA Turbo Link 2.0 experiment and require identity."""
from pathlib import Path

from probe_source_data_link import run as source_data_link
from probe_shared_module_link import run as shared_module_link
from probe_data_interleaving import run as data_interleaving
from reconstruct import ROOT, read_json, write_json


def run():
    source = source_data_link()
    if not source['byte_comparison']['load_image']['equal']:
        raise ValueError('Source DATA link no longer has an exact load image')
    first = shared_module_link(ROOT / 'recipes/modules/C_75F3_7856.json', True)
    first_path = ROOT / 'build/shared-source-data-link-report_C_75F3_7856.json'
    second = shared_module_link(ROOT / 'recipes/modules/C_AD25_AF45.json', True, first_path)
    second_path = ROOT / 'build/shared-source-data-link-report_RELOC_F_AD25_F_ADCF.json'
    third = shared_module_link(ROOT / 'recipes/modules/C_DDD9_E095.json', True, second_path)
    third_path = ROOT / 'build/shared-source-data-link-report_RELOC_F_DDD9_F_DF98.json'
    final = data_interleaving(third_path, ROOT / 'recipes/data/interleaving-candidate.json')
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
            {'name': first['candidate'], 'status': first['status']},
            {'name': second['candidate'], 'status': second['status']},
            {'name': third['candidate'], 'status': third['status'],
             'fixupp_order_adapter': third['fixupp_order_adapter']},
            {'name': 'data_code_interleaving', 'status': final['status'],
             'matching_relocation_prefix_entries': final['matching_relocation_prefix_entries']},
        ],
        'byte_comparison': {key: value for key, value in comparison.items()
                            if key not in ('candidate', 'oracle')},
        'remaining_adapters': [
            'candidate DATA/code object interleaving',
            'arithmetic-module FIXUPP subrecord ordering',
            'unpartitioned TASM BSS reserve and historical storage ownership',
        ],
        'whole_build_reconstruction_complete': False,
        'limitation': ('Byte-identical TLINK output from relocatable inputs with zero raw DATA '
                       'and zero object symbol transforms; object order, BSS ownership and '
                       'historical module proof remain.'),
    }
    write_json(ROOT / 'build/exact-structural-link-report.json', report)
    write_json(ROOT / 'docs/exact-structural-link.json', report)
    print('Exact structural TLINK experiment: BYTE IDENTICAL')
    print(comparison['candidate_sha256'])
    return report


if __name__ == '__main__':
    run()
