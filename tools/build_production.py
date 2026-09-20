"""Final source modules -> DATA/BSS -> one unmodified TLINK invocation."""
import copy
import os
from pathlib import Path
import shutil
import tempfile

from reconstruct import ROOT, read_json, write_json, sha, read_object
from omf_scaffold import make_text_padding
from production_plan import checked_plan
from production_data import prepare_data
from object_cache import compile_cached
from dos_runner import resolve_runner
from mz import MZ, encode_header
from link_support import link_errors, replace_library_member


def build(root=ROOT, verify=True, runner=None, dosbox=None, research=False):
    from build_exe import validate_toolchain, ORIGINAL_SHA256
    root = Path(root).resolve()
    (root / 'build').mkdir(exist_ok=True)
    # Invalidate success before any input validation can fail.
    for name in ('AEPROG.EXE', 'exe-build-report.json'):
        (root / 'build' / name).unlink(missing_ok=True)
    from factory_inputs import input_fingerprint
    initial_fingerprint = input_fingerprint(root)
    lock, verified = validate_toolchain(root)
    plan = checked_plan(root)
    runner = runner or resolve_runner(lock, backend='dosbox' if dosbox else None, executable=dosbox)
    work = Path(tempfile.mkdtemp(prefix='production-', dir=root / 'build')).resolve()
    units = work / 'WORK'
    units.mkdir()
    (work / 'TC/LIB').mkdir(parents=True)
    for name in ('C0C.OBJ', 'CC.LIB'):
        shutil.copyfile(root / 'toolchain' / name, work / 'TC/LIB' / name)
    modules = copy.deepcopy(plan['modules'])
    for module in modules:
        if module.get('sources'):
            source = work / (module['id'] + '.C')
            source.write_bytes(b'\r\n'.join((root / s).read_bytes() for s in module['sources']))
            module['source'] = source.relative_to(root).as_posix()
    receipts, session = compile_cached(root, modules, work / 'compile', root / 'toolchain', runner, lock,
                                       enabled=research)
    staged, member_objects = {}, {}
    for module in modules:
        blob = (work / 'compile' / receipts[module['id']]['object']).read_bytes()
        parsed = read_object(blob)
        # Extents are verification only; never shorten compiler output to fit.
        if parsed.segment_length('_TEXT') != module['end'] - module['start']:
            raise ValueError(f"{module['id']}: natural compiler TEXT extent differs")
        if any(a['before_module'] == module['id'] for a in plan.get('linker_alignment', [])):
            if next(s for s in parsed.segment_defs if s['name'] == '_TEXT')['alignment'] != 'word':
                raise ValueError('Source TEXT segment does not provide declared word alignment')
        for public in module['publics']:
            if public['symbol'] and not any(p['name'] == public['symbol'] and p['offset'] == public['offset']
                                            for p in parsed.publics_in('_TEXT')):
                raise ValueError(f"{module['id']}: public offset differs: {public}")
        (units / module['object']).write_bytes(blob)
        replacement = module['build'].get('linker_library_module')
        if replacement:
            library_path = work / 'TC/LIB/CC.LIB'
            library_path.write_bytes(replace_library_member(library_path.read_bytes(), replacement, blob))
        entry = dict(module, kind='library_replacement_source' if replacement else 'owner')
        staged[module['id']] = entry
        for member in module['members']:
            member_objects[member] = entry
    for pad in plan['padding']:
        (units / pad['object']).write_bytes(make_text_padding(pad['end'] - pad['start'], pad['id']))
    manifest = read_json(root / 'layout/manifest.json')
    parts, bss, data_sources, separated = prepare_data(root, work, manifest, staged, modules, member_objects)
    bss_owners = [{'id': b['id'], 'kind': 'MATCHING_ASM',
                   'source': (units / b['assembly']).relative_to(root).as_posix(), 'build': {}} for b in bss]
    bss_receipts, bss_session = compile_cached(root, bss_owners, work / 'bss', root / 'toolchain', runner, lock,
                                               enabled=research)
    for b in bss:
        blob = (work / 'bss' / bss_receipts[b['id']]['object']).read_bytes()
        parsed = read_object(blob)
        if parsed.segment_length('_BSS') != b['length']:
            raise ValueError(f"BSS extent differs: {b['id']}")
        (units / b['object']).write_bytes(blob)
    object_transforms = []
    for module in modules:
        raw = (work / 'compile' / receipts[module['id']]['object']).read_bytes()
        final = (units / module['object']).read_bytes()
        if raw != final:
            before, after = read_object(raw), read_object(final)
            object_transforms.append({
                'module': module['id'], 'compiler_sha256': sha(raw),
                'link_input_sha256': sha(final),
                'text_bytes_equal': before.segment_bytes('_TEXT') == after.segment_bytes('_TEXT'),
                'data_bytes_before': before.segment_length('_DATA'),
                'data_bytes_after': after.segment_length('_DATA')})
    if object_transforms:
        raise ValueError('Compiler objects must reach the linker unchanged')
    from public_index import collect
    if collect(root, plan, work) != read_json(root / 'layout/public-index.json'):
        raise ValueError('Public index differs from fresh objects; supervisor must reconcile layout/public-index.json')
    # Exactly one link, against the complete final object order.
    dosbox_mode = runner.backend == 'dosbox'
    prefix = 'C:\\WORK\\' if dosbox_mode else ''
    libprefix = 'C:\\TC\\LIB\\' if dosbox_mode else '..\\TC\\LIB\\'
    names = [libprefix + 'C0C.OBJ'] + [prefix + n for n in plan['object_order'][1:]]
    response = '/s ' + '+'.join(names) + ',OUT.EXE,OUT.MAP,' + libprefix + 'CC.LIB'
    (work / 'LINK.RSP').write_text(response, encoding='ascii')
    if dosbox_mode:
        import subprocess
        (work / 'BIN').mkdir()
        shutil.copyfile(root / 'toolchain/TLINK.EXE', work / 'BIN/TLINK.EXE')
        (work / 'GO.BAT').write_bytes(b'@echo off\r\nc:\r\ncd \\WORK\r\nC:\\BIN\\TLINK.EXE @C:\\LINK.RSP > LINK.LOG\r\n')
        conf = work / 'link.conf'
        conf.write_text(f'[sdl]\noutput=texture\n[mixer]\nnosound=true\n[autoexec]\nmount c "{work}"\nc:\ncall C:\\GO.BAT\nexit\n')
        result = subprocess.run([str(runner.executable), '-conf', str(conf), '--noprimaryconfig', '-noconsole', '-exit'],
                                cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120,
                                **runner._options())
    else:
        result, _, _ = runner.run(root / 'toolchain/TLINK.EXE', ['@..\\LINK.RSP'], units,
                                  log_path=units / 'LINK.LOG', timeout=120)
    log = (units / 'LINK.LOG').read_text(errors='replace')
    errors = link_errors(log, (units / 'OUT.MAP').read_text(errors='replace') if (units / 'OUT.MAP').exists() else '')
    if result.returncode or errors or not (units / 'OUT.EXE').exists():
        raise ValueError(f'TLINK failed: {errors}; inspect {units / "LINK.LOG"}')
    blob = (units / 'OUT.EXE').read_bytes()
    mz = MZ.parse(blob)
    expected_mz = MZ.parse_header(encode_header(read_json(root / 'layout/mz-header.json'),
                                              manifest['original']['size']), manifest['original']['size'])
    if mz.relocations != expected_mz.relocations or len(mz.relocations) != 106:
        raise ValueError('Ordered MZ relocations differ from canonical metadata')
    if sha(blob) != ORIGINAL_SHA256:
        raise ValueError(f'Full EXE hash differs: {sha(blob)}; candidate {units / "OUT.EXE"}')
    verification = {'performed': verify, 'reason': 'verification not requested'}
    if verify:
        oracle = (root / 'assets/AEPROG.EXE').read_bytes()
        if sha(oracle) != ORIGINAL_SHA256 or oracle != blob:
            raise ValueError('Original EXE identity/byte comparison failed')
        verification = {'performed': True, 'byte_identical': True, 'relocation_order_equal': True}
    bss_layout = read_json(root / 'src/data/GAME_BSS.json')
    report = {'format': 'empires-exe-build-v2', 'status': 'BUILT', 'sha256': sha(blob), 'size': len(blob),
              'output': str(root / 'build/AEPROG.EXE'), 'mode': 'RESEARCH' if research else 'ACCEPTANCE',
              'fresh_build': not research, 'link_invocations': 1, 'session': str(work),
              'plan_sha256': sha((root / 'layout/production-plan.json').read_bytes()),
              'toolchain_files': verified, 'runner': runner.receipt(),
              'compiled_source_modules': len(modules), 'text_extent_policy': 'CHECK_ONLY_NO_TRIMMING',
              'compiled_objects': {ident: receipt['object'] for ident, receipt in receipts.items()}, 'compile': session, 'bss_compile': bss_session,
              'relocations': 106, 'unresolved_symbols': 0, 'remaining_structural_adapters': [],
              'bss': {'bytes': bss_layout['length'], 'partitioned_source_bytes': sum(b['length'] for b in bss),
                      'aggregate_remainder_bytes': sum(b['length'] for b in bss if b['aggregate_storage'])},
              'compiler_object_transformations': object_transforms,
              'untouched_compiler_objects': len(modules) - len(object_transforms),
              'natural_code_alignment': plan.get('linker_alignment', []),
              'synthetic_code_padding_objects': len(plan['padding']),
              'pre_link_corrections': {'text_trimming': [],
                  'data_externalization': separated,
                  'library_segment_replacements': []},
              'generated_data_components': len(data_sources),
              'native_library_members': [{'owner': m['id'], 'member': m['build']['linker_library_module'],
                                          'object_sha256': sha((work / 'compile' / receipts[m['id']]['object']).read_bytes())}
                                         for m in modules if m['build'].get('linker_library_module')],
              'native_data_components': [{'owner': p['spec']['code_owner'], 'component': p['spec']['id'], 'bytes': len(p['data'])}
                                         for p in parts if p['spec'].get('retain_in_code_object')],
              'separated_data': separated, 'verification': verification}
    if input_fingerprint(root) != initial_fingerprint:
        raise ValueError('Construction inputs changed during the build; acceptance refused')
    report['input_fingerprint'] = initial_fingerprint
    shutil.copyfile(units / 'OUT.EXE', root / 'build/AEPROG.EXE')
    write_json(root / 'build/exe-build-report.json', report)
    return report
