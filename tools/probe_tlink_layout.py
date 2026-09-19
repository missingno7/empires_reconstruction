"""Run a relocatable C/OMF scaffold through a local Borland TLINK.

This is an experiment path. It never changes the fixed manifest or canonical
EXE. It compiles the proven C owners after the compact startup extent, links
them with the pinned local C0C.OBJ and CC.LIB, and records the first address or
module-order divergence from the fixed oracle.
"""
import argparse
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import tempfile

from omf_scaffold import (make_dgroup_scaffold, make_external_demand,
                          add_publics, make_text_padding,
                          normalize_external_case, rename_external, remove_public,
                          rename_external_addend,
                          trim_text_contribution)
from omf import OmfReader
from mz import MZ
from reconstruct import ROOT, compile_sources, read_json, sha, write_json


DEFAULT_LINKER = ROOT / 'toolchain/TLINK.EXE'
RECOVERED_SYMBOL_ALIASES = {
    'F_233E': [('_delay', '_f6c57')],
    'F_56C6': [('_delay', '_f6c57')],
}
CODE_ROW = re.compile(
    r'^\s*([0-9A-F]+):([0-9A-F]+)\s+([0-9A-F]+)\s+C=CODE\s+S=_TEXT\s+.*?M=(\S+)\s+ACBP=([0-9A-F]+)')
SEGMENT_ROW = re.compile(
    r'^\s*([0-9A-F]+)H\s+([0-9A-F]+)H\s+([0-9A-F]+)H\s+(\S+)\s+(\S+)')


def parse_map(path):
    rows, segments = [], []
    detailed = False
    for line in path.read_text(errors='replace').splitlines():
        if line.startswith('Detailed map of segments'):
            detailed = True
            continue
        if detailed:
            match = CODE_ROW.match(line)
            if match:
                segment, offset, length, module, acbp = match.groups()
                rows.append({'segment': int(segment, 16), 'offset': int(offset, 16),
                             'length': int(length, 16), 'module': module,
                             'acbp': int(acbp, 16)})
        elif not line.startswith(' Start'):
            match = SEGMENT_ROW.match(line)
            if match:
                start, stop, length, name, cls = match.groups()
                segments.append({'start': int(start, 16), 'stop': int(stop, 16),
                                 'length': int(length, 16), 'name': name, 'class': cls})
    return segments, rows


def linker_files(linker):
    names = ['TLINK.EXE', 'DPMI16BI.OVL', 'DPMILOAD.EXE', 'DPMIMEM.DLL']
    result = []
    for name in names:
        path = linker.parent / name
        if path.exists():
            result.append(path)
    return result


def link_errors(log, map_text=''):
    """TLINK can write fixup failures only to its detailed map."""
    return list(dict.fromkeys(line.strip() for line in (log + '\n' + map_text).splitlines()
                             if re.search(r'error:|undefined symbol|bad object file|fatal|fixup overflow',
                                          line, re.I)))


def initialized_data_end(segments):
    # TLINK prints start == stop for empty segments; stop + 1 is not an extent.
    return max(s['start'] + s['length'] for s in segments
               if s['name'] in {'_DATA', '_EMUSEG', '_CRTSEG', '_CVTSEG', '_SCNSEG'})


def compare_linked_executable(candidate, oracle):
    """Compare a TLINK output at header, relocation, load-image and file levels."""
    result = {'candidate': str(candidate), 'oracle': str(oracle), 'available': False}
    if not candidate.exists() or not oracle.exists():
        return result
    candidate_bytes = candidate.read_bytes()
    oracle_bytes = oracle.read_bytes()
    result.update({'available': True, 'candidate_sha256': sha(candidate_bytes),
                   'oracle_sha256': sha(oracle_bytes),
                   'candidate_size': len(candidate_bytes), 'oracle_size': len(oracle_bytes)})
    try:
        candidate_mz = MZ.parse(candidate_bytes)
        oracle_mz = MZ.parse(oracle_bytes)
    except ValueError as error:
        result['parse_error'] = str(error)
        return result
    candidate_fields = struct.unpack_from('<14H', candidate_bytes)
    oracle_fields = struct.unpack_from('<14H', oracle_bytes)
    field_names = ('e_magic', 'e_cblp', 'e_cp', 'e_crlc', 'e_cparhdr', 'e_minalloc',
                   'e_maxalloc', 'e_ss', 'e_sp', 'e_csum', 'e_ip', 'e_cs',
                   'e_lfarlc', 'e_ovno')
    result['mz'] = {
        'candidate_fields': dict(zip(field_names, candidate_fields)),
        'oracle_fields': dict(zip(field_names, oracle_fields)),
        'fields_equal': candidate_fields == oracle_fields,
        'relocation_count_equal': len(candidate_mz.relocations) == len(oracle_mz.relocations),
        'relocation_order_equal': candidate_mz.relocations == oracle_mz.relocations,
        'relocation_pairs_equal': sorted((r['segment'], r['offset']) for r in candidate_mz.relocations) ==
                                  sorted((r['segment'], r['offset']) for r in oracle_mz.relocations),
        'missing_sites': sorted({r['load_offset'] for r in oracle_mz.relocations} -
                                {r['load_offset'] for r in candidate_mz.relocations}),
        'extra_sites': sorted({r['load_offset'] for r in candidate_mz.relocations} -
                              {r['load_offset'] for r in oracle_mz.relocations}),
    }
    candidate_load = candidate_mz.load_image(candidate_bytes)
    oracle_load = oracle_mz.load_image(oracle_bytes)

    def slice_comparison(name, start, end):
        left = candidate_load[start:end]
        right = oracle_load[start:end]
        first = next((start + i for i, (a, b) in enumerate(zip(left, right)) if a != b), None)
        if first is None and len(left) != len(right):
            first = start + min(len(left), len(right))
        return {'equal': left == right, 'start': start, 'end': end,
                'candidate_length': len(left), 'oracle_length': len(right),
                'differing_byte_count': sum(a != b for a, b in zip(left, right)) + abs(len(left) - len(right)),
                'first_difference': first}

    result['load_image'] = {
        'equal': candidate_load == oracle_load,
        'first_difference': next((i for i, (a, b) in enumerate(zip(candidate_load, oracle_load))
                                  if a != b), min(len(candidate_load), len(oracle_load))
                                 if len(candidate_load) != len(oracle_load) else None),
        'candidate_length': len(candidate_load), 'oracle_length': len(oracle_load),
    }
    result['text'] = slice_comparison('_TEXT', 0, 0xFA23)
    result['initialized_data'] = slice_comparison('_DATA', 0xFA30, 0x13332)
    result['full_file'] = {
        'equal': candidate_bytes == oracle_bytes,
        'differing_bytes_outside_relocation_table': sum(
            a != b and not result['mz']['oracle_fields']['e_lfarlc'] <= i <
            result['mz']['oracle_fields']['e_lfarlc'] + 4 * result['mz']['oracle_fields']['e_crlc']
            for i, (a, b) in enumerate(zip(candidate_bytes, oracle_bytes))) + abs(len(candidate_bytes) - len(oracle_bytes)),
        'first_difference': next((i for i, (a, b) in enumerate(zip(candidate_bytes, oracle_bytes))
                                  if a != b), min(len(candidate_bytes), len(oracle_bytes))
                                 if len(candidate_bytes) != len(oracle_bytes) else None),
    }
    return result


def _omf_records(data):
    """Yield ``(kind, start, end)`` for one OMF module or object."""
    at = 0
    while at < len(data):
        if at + 3 > len(data):
            raise ValueError('truncated OMF record')
        length = struct.unpack_from('<H', data, at + 1)[0]
        end = at + 3 + length
        if end > len(data) or length < 1:
            raise ValueError('invalid OMF record length')
        yield data[at], at, end
        at = end


def replace_library_module_segments(library, module_name, replacement_object):
    """Replace one library module's LEDATA records with source-object bytes.

    Every initialized segment contribution must have the same OMF record size
    in both modules. The library's page layout, dictionary and module metadata
    remain unchanged, so this is a structural linker experiment rather than a
    new library format claim.
    """
    page = struct.unpack_from('<H', library, 1)[0] + 3
    module_start = page
    while module_start < len(library) and library[module_start] == OmfReader.THEADR:
        at = module_start
        name = ''
        while at < len(library):
            kind = library[at]
            length = struct.unpack_from('<H', library, at + 1)[0]
            if kind == OmfReader.THEADR:
                size = library[at + 3]
                name = library[at + 4:at + 4 + size].decode('latin1')
            at += 3 + length
            if kind in (OmfReader.MODEND16, OmfReader.MODEND32):
                break
        module_end = at
        if name == module_name:
            module = library[module_start:module_end]
            candidate_records = [replacement_object[a:b] for k, a, b in _omf_records(replacement_object)
                                 if k == OmfReader.LEDATA16]
            original_records = [(a, b) for k, a, b in _omf_records(module)
                                 if k == OmfReader.LEDATA16]
            if not candidate_records or len(candidate_records) != len(original_records):
                raise ValueError(f'{module_name}: initialized OMF segment contributions differ')
            replacements = []
            for (a, b), candidate_record in zip(original_records, candidate_records):
                if len(candidate_record) != b - a:
                    raise ValueError(f'{module_name}: source LEDATA size differs from library module')
                replacements.append((a, b, candidate_record))
            patched_module = module
            for a, b, candidate_record in reversed(replacements):
                patched_module = patched_module[:a] + candidate_record + patched_module[b:]
            source_module = OmfReader().read(replacement_object, module_name)
            patched = OmfReader().read(patched_module, module_name)
            for segment in source_module.segments:
                if patched.segment_bytes(segment) != source_module.segment_bytes(segment):
                    raise ValueError(f'{module_name}: source/library {segment} replacement failed')
            return library[:module_start] + patched_module + library[module_end:]
        module_start = ((module_end + page - 1) // page) * page
    raise ValueError(f'{module_name}: module not found in library')


def run(root=ROOT, linker=DEFAULT_LINKER, dosbox=None, promote_toupper=False,
        demand_historical_library=False, normalize_recovered_symbols=False,
        scaffold_dgroup=False, normalize_case_symbols=False,
        expose_internal_labels=False):
    linker = Path(linker).resolve()
    if not linker.exists():
        raise ValueError(f'linker candidate is unavailable: {linker}')
    dosbox = Path(dosbox or os.environ.get(
        'DOSBOX', 'C:/Program Files/DOSBox Staging/dosbox.exe')).resolve()
    lock = read_json(root / 'layout/toolchain.json')
    manifest = read_json(root / 'layout/manifest.json')
    c0c = root / 'toolchain/C0C.OBJ'
    cc_lib = root / 'toolchain/CC.LIB'
    if not c0c.exists() or not cc_lib.exists():
        raise ValueError('run tools/setup_toolchain.py first to install C0C.OBJ and CC.LIB')
    startup_length = 0x1BC
    owners = [o for o in manifest['regions']
              if o['kind'] == 'MATCHING_C' and o['start'] >= 512 + startup_length]
    compile_owners = [o for o in owners
                      if not (promote_toupper and o['id'] == 'F_F9BE')]
    library_replacements = {
        o['id']: o['build']['linker_library_module']
        for o in compile_owners
        if o.get('build', {}).get('linker_library_module')
    }
    linkable_owners = [o for o in compile_owners if o['id'] not in library_replacements]
    root_build = root / 'build'
    root_build.mkdir(exist_ok=True)
    work = Path(tempfile.mkdtemp(prefix='tlink-structural-', dir=root_build)).resolve()
    compile_work = work / 'compile'
    receipts, compile_session = compile_sources(root, compile_owners, compile_work,
                                                root / 'toolchain', dosbox, lock)
    explicit_publics = set()
    for receipt in receipts.values():
        explicit_publics.update(public['name'] for public in
                                 OmfReader().read((compile_work / receipt['object']).read_bytes()).publics)
    explicit_publics.update(public['name'] for public in
                             OmfReader().read(c0c.read_bytes()).publics)
    owner_by_id = {owner['id']: owner for owner in compile_owners}
    load_regions = sorted(manifest['regions'], key=lambda item: item['start'])
    library_blob = cc_lib.read_bytes()
    for owner_id, module_name in library_replacements.items():
        library_blob = replace_library_module_segments(
            library_blob, module_name,
            (compile_work / receipts[owner_id]['object']).read_bytes())
    library_available = {name: blob for name, blob in
                         OmfReader().split_library(library_blob)}

    def library_symbol_at(module_name, offset):
        """Return the public that best names an internal library offset."""
        module = OmfReader().read(library_available[module_name], module_name)
        publics = [public for public in module.publics if public['segment'] == '_TEXT']
        exact = next((public for public in publics if public['offset'] == offset), None)
        if exact:
            return exact['name'], 0
        primary = next((public for public in publics if public['offset'] == 0), None)
        if primary:
            return primary['name'], offset
        return None

    internal_labels = {}
    library_internal_aliases = {}
    scoped_aliases = {}
    binding_targets = {}
    for region in manifest['regions']:
        for symbol, binding in region.get('build', {}).get('bindings', {}).items():
            binding_targets.setdefault(symbol, set()).add((binding.get('coordinate'),
                binding.get('offset'), binding.get('owner'), binding.get('addend', 0)))
    if expose_internal_labels:
        module_externals = {}
        for owner_id, receipt in receipts.items():
            module = OmfReader().read((compile_work / receipt['object']).read_bytes())
            module_externals[owner_id] = set(module.externals)
            for external in module.externals:
                match = re.fullmatch(r'_F([0-9A-Fa-f]+)', external, re.IGNORECASE)
                if not match:
                    continue
                target = int(match.group(1), 16)
                region = next((item for item in load_regions
                               if item['start'] - 512 <= target < item['end'] - 512), None)
                if region and region['id'] in owner_by_id:
                    internal_labels.setdefault(region['id'], {})[external] = (
                        target - (region['start'] - 512))
        for source_region in manifest['regions']:
            for symbol, binding in source_region.get('build', {}).get('bindings', {}).items():
                if binding.get('coordinate') != 'code_offset':
                    continue
                if 'offset' in binding:
                    target = binding['offset']
                elif binding.get('owner'):
                    target_owner = next((item for item in load_regions
                                         if item['id'] == binding['owner']), None)
                    if not target_owner:
                        continue
                    target = target_owner['start'] - 512 + binding.get('addend', 0)
                    if target_owner['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY' and binding.get('public'):
                        module_name = target_owner['build']['library_module']
                        publics = OmfReader().read(library_available[module_name]).publics_in('_TEXT')
                        public = next((p for p in publics if p['name'] == binding['public']), None)
                        if public is None:
                            raise ValueError('Declared library target public is absent')
                        target += public['offset']
                else:
                    continue
                target_region = next((item for item in load_regions
                                      if item['start'] - 512 <= target < item['end'] - 512), None)
                if target_region and target_region['id'] in owner_by_id:
                    owner_id = source_region['id']
                    if symbol in module_externals.get(owner_id, set()):
                        delta = target - (target_region['start'] - 512)
                        label = symbol
                        if len(binding_targets[symbol]) > 1:
                            label = f"__RC_{target_region['id']}_{delta:X}"
                            scoped_aliases.setdefault(owner_id, {})[symbol] = label
                        internal_labels.setdefault(target_region['id'], {})[label] = delta
                if target_region and target_region.get('build', {}).get('segment') == '_TEXT':
                    target_build = target_region.get('build', {})
                    module_name = (target_build.get('library_module') or
                                   target_build.get('linker_library_module'))
                    if module_name in library_available:
                        target_offset = target - (target_region['start'] - 512)
                        selected = library_symbol_at(module_name, target_offset)
                        if selected:
                            owner_id = source_region['id']
                            if symbol in module_externals.get(owner_id, set()) and selected != (symbol, 0):
                                library_internal_aliases.setdefault(owner_id, {})[symbol] = selected
        for owner_id, externals in module_externals.items():
            for external in externals:
                match = re.fullmatch(r'_F([0-9A-Fa-f]+)', external, re.IGNORECASE)
                if not match:
                    continue
                target = int(match.group(1), 16)
                target_region = next((item for item in load_regions
                                      if item['start'] - 512 <= target < item['end'] - 512), None)
                if not target_region or target_region['id'] in owner_by_id:
                    continue
                target_build = target_region.get('build', {})
                module_name = (target_build.get('library_module') or
                               target_build.get('linker_library_module'))
                if module_name not in library_available:
                    continue
                selected = library_symbol_at(module_name, target - (target_region['start'] - 512))
                if selected:
                    library_internal_aliases.setdefault(owner_id, {})[external] = (
                        selected[0], selected[1])
        # C0C.OBJ enters through the conventional Turbo C ``_main`` public.
        # The recovered owner is named by its original numeric entry label;
        # expose the startup spelling as a symbol alias in that object rather
        # than changing its generated bytes or forcing an address.
        if 'F_4A93' in owner_by_id:
            internal_labels.setdefault('F_4A93', {})['_main'] = 0
    tc_lib = work / 'TC/LIB'
    bc_bin = work / 'BC/BIN'
    dos_work = work / 'WORK'
    tc_lib.mkdir(parents=True)
    bc_bin.mkdir(parents=True)
    dos_work.mkdir(parents=True)
    shutil.copyfile(c0c, tc_lib / 'C0C.OBJ')
    (tc_lib / 'CC.LIB').write_bytes(library_blob)
    for path in linker_files(linker):
        shutil.copyfile(path, bc_bin / path.name.upper())
    scaffold = []
    object_names = []
    staged_publics = set()
    if demand_historical_library:
        library_modules = []
        for region in manifest['regions']:
            if (region['kind'] == 'KNOWN_TOOLCHAIN_LIBRARY' or
                    region.get('build', {}).get('linker_library_module')):
                module = region.get('build', {}).get('library_module')
                if not module:
                    module = region.get('build', {}).get('linker_library_module')
                if module and module not in library_modules:
                    library_modules.append(module)
        demands = []
        for module in library_modules:
            if module not in library_available:
                raise ValueError(f'expected CC.LIB module is unavailable: {module}')
            parsed = OmfReader().read(library_available[module], module)
            demands.extend(public['name'] for public in parsed.publics)
        demand = make_external_demand(demands)
        demand_name = 'LBDMD.OBJ'
        (dos_work / demand_name).write_bytes(demand)
        object_names.append(demand_name)
        scaffold.append({
            'kind': 'historical_library_demand',
            'object': demand_name,
            'module_count': len(library_modules),
            'public_demand_count': len(set(demands)),
            'modules': library_modules,
            'staged_sha256': sha(demand),
            'staged_size': len(demand),
        })
    owner_by_id = {owner['id']: owner for owner in compile_owners}
    code_end = max(owner['end'] for owner in owners)
    pad_index = 0
    for region in sorted(manifest['regions'], key=lambda item: item['start']):
        if region['id'] in owner_by_id:
            owner = owner_by_id[region['id']]
            receipt = receipts[owner['id']]
            source = compile_work / receipt['object']
            original_bytes = source.read_bytes()
            source_bytes = original_bytes
            transforms = []
            if expose_internal_labels:
                for old, new in scoped_aliases.get(owner['id'], {}).items():
                    source_bytes = rename_external(source_bytes, old, new)
                    transforms.append(f'caller-scoped EXTDEF {old} -> {new}')
            if normalize_case_symbols:
                normalized = normalize_external_case(source_bytes, explicit_publics)
                if normalized != source_bytes:
                    source_bytes = normalized
                    transforms.append('EXTDEF case normalization to explicit public')
            if promote_toupper and owner['id'] == 'F_A525':
                externals = OmfReader().read(source_bytes).externals
                if '_ff9be' in externals:
                    source_bytes = rename_external(source_bytes, '_ff9be', '_toupper')
                    transforms.append('EXTDEF _ff9be -> _toupper')
                elif '_toupper' not in externals:
                    raise ValueError('F_A525 references neither recovered nor historical toupper public')
            if normalize_recovered_symbols:
                for old, new in RECOVERED_SYMBOL_ALIASES.get(owner['id'], []):
                    if old in OmfReader().read(source_bytes).externals:
                        source_bytes = rename_external(source_bytes, old, new)
                        transforms.append(f'EXTDEF {old} -> {new}')
            if expose_internal_labels and owner['id'] in internal_labels:
                labels = internal_labels[owner['id']]
                updated = add_publics(source_bytes, labels)
                if updated != source_bytes:
                    source_bytes = updated
                    transforms.append(f'PUBDEF internal labels ({len(labels)})')
            if expose_internal_labels and owner['id'] in library_internal_aliases:
                for old, (new, delta) in library_internal_aliases[owner['id']].items():
                    source_bytes = rename_external_addend(source_bytes, old, new, delta)
                    transforms.append(f'EXTDEF {old} -> {new} + {delta}')
            owned_length = owner['end'] - owner['start']
            replacement_module = library_replacements.get(owner['id'])
            if replacement_module is None:
                current_module = OmfReader().read(source_bytes)
                duplicate_publics = [public['name'] for public in current_module.publics
                                     if public['name'] in staged_publics]
                for public_name in duplicate_publics:
                    source_bytes = remove_public(source_bytes, public_name)
                    transforms.append(f'removed duplicate PUBDEF {public_name}')
                staged_publics.update(public['name'] for public in OmfReader().read(source_bytes).publics)
            trimmed = trim_text_contribution(source_bytes, owned_length)
            staged = dos_work / Path(receipt['object']).name
            staged.write_bytes(trimmed)
            if replacement_module is None:
                object_names.append(staged.name)
            scaffold.append({
                'kind': 'library_replacement_source' if replacement_module else 'owner',
                'owner': owner['id'],
                'object': staged.name,
                'original_sha256': sha(original_bytes),
                'staged_sha256': sha(trimmed),
                'original_size': len(source_bytes),
                'staged_size': len(trimmed),
                'owned_text_length': owned_length,
                **({'library_module': replacement_module} if replacement_module else {}),
                **({'transforms': transforms} if transforms else {}),
            })
        elif (region.get('classification') == 'alignment_padding'
              and startup_length + 512 <= region['start'] < code_end):
            length = region['end'] - region['start']
            name = f'P{pad_index:04d}.OBJ'
            pad_index += 1
            padding = make_text_padding(length, Path(name).stem)
            (dos_work / name).write_bytes(padding)
            object_names.append(name)
            scaffold.append({
                'kind': 'alignment_padding',
                'owner': region['id'],
                'object': name,
                'staged_sha256': sha(padding),
                'staged_size': len(padding),
                'owned_text_length': length,
            })
    def link_once(tag, names):
        final = tag == 'FINAL'
        response_name = 'LINK.RSP' if final else f'{tag}.RSP'
        log_name = 'LINK.LOG' if final else f'{tag}.LOG'
        output_name = 'OUT.EXE' if final else f'{tag}.EXE'
        map_name = 'OUT.MAP' if final else f'{tag}.MAP'
        response = '/s C:\\TC\\LIB\\C0C.OBJ+' + '+'.join(
            f'C:\\WORK\\{name}' for name in names)
        response += f',{output_name},{map_name},C:\\TC\\LIB\\CC.LIB'
        (work / response_name).write_text(response, encoding='ascii')
        batch = ('@echo off\r\n'
                 'c:\r\n'
                 'cd \\work\r\n'
                 'set PATH=C:\\BC\\BIN\r\n'
                 f'tlink @C:\\{response_name} > {log_name}\r\n'
                 'echo DONE>DONE.TXT\r\n')
        (work / 'GO.BAT').write_bytes(batch.encode('ascii'))
        config = ('[sdl]\noutput=texture\n[mixer]\nnosound=true\n[dosbox]\n'
                  'machine=svga_s3\nmemsize=16\n[cpu]\ncore=auto\n'
                  'cputype=auto\n[autoexec]\n'
                  f'mount c "{work}"\nc:\ncall c:\\go.bat\nexit\n')
        config_path = work / f'{tag}.conf'
        config_path.write_text(config, encoding='utf-8')
        command = [str(dosbox), '-conf', str(config_path), '--noprimaryconfig',
                   '-noconsole', '-exit']
        result = subprocess.run(command, cwd=work, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT, timeout=600)
        log_path = dos_work / log_name
        map_path = dos_work / map_name
        exe_path = dos_work / output_name
        log = log_path.read_text(errors='replace') if log_path.exists() else ''
        return result, command, log, map_path, exe_path, response

    sizing = None
    if scaffold_dgroup:
        base_result, _, base_log, base_map, _, _ = link_once('BASE', object_names)
        base_segments, _ = parse_map(base_map) if base_map.exists() else ([], [])
        data_segment = next((s for s in base_segments if s['name'] == '_DATA'), None)
        stack_segment = next((s for s in base_segments if s['name'] == '_STACK'), None)
        if not data_segment or not stack_segment:
            raise ValueError('baseline TLINK map lacks DATA or STACK for DGROUP scaffold sizing')
        data_end = initialized_data_end(base_segments)
        text_end = max(r['end'] - 512 for r in manifest['regions']
                       if r.get('build', {}).get('segment') == '_TEXT')
        dgroup_load = (text_end + 15) & ~15
        image_end = max(r['end'] - 512 for r in manifest['regions'])
        target_data_span = image_end - dgroup_load
        target_stack = MZ.parse((root / 'assets/AEPROG.EXE').read_bytes()).ss * 16
        current_data_span = data_end - data_segment['start']
        data_tail_length = max(0, target_data_span - current_data_span)
        bss_tail_length = max(0, target_stack - stack_segment['start'] - data_tail_length)
        image = (root / 'assets/AEPROG.EXE').read_bytes()
        data_tail_start = 512 + dgroup_load + current_data_span
        data_tail = image[data_tail_start:data_tail_start + data_tail_length]
        if len(data_tail) != data_tail_length:
            raise ValueError('oracle initialized-data tail is shorter than scaffold sizing')
        cc_publics = set()
        for _, blob in OmfReader().split_library(cc_lib.read_bytes()):
            cc_publics.update(public['name'] for public in OmfReader().read(blob).publics)
        injected_publics = {name for labels in internal_labels.values() for name in labels}
        explicit_publics_casefold = {name.lower() for name in explicit_publics | injected_publics}
        dgroup_publics = {}
        for region in manifest['regions']:
            for symbol, binding in region.get('build', {}).get('bindings', {}).items():
                # Turbo Link 2.0 rejects duplicate PUBDEFs even when the
                # duplicate is the same startup/global symbol. The synthetic
                # scaffold should define only unresolved data names; C0C and
                # reconstructed objects already provide the rest.
                if (binding.get('coordinate') != 'DGROUP_offset' or
                        symbol in cc_publics or symbol in explicit_publics or
                        symbol.lower() in explicit_publics_casefold):
                    continue
                if 'offset' in binding:
                    offset = binding['offset']
                elif binding.get('owner') in {item['id'] for item in manifest['regions']}:
                    target = next(item for item in manifest['regions']
                                   if item['id'] == binding['owner'])
                    offset = target['start'] - 512 - dgroup_load + binding.get('addend', 0)
                else:
                    continue
                if offset < 0:
                    continue
                if offset < target_data_span:
                    location = ('_DATA', offset)
                elif offset - target_data_span < 0x10000:
                    location = ('_BSS', offset - target_data_span)
                else:
                    continue
                dgroup_publics.setdefault(symbol, location)
        scaffold_blob = make_dgroup_scaffold(data_tail, bss_tail_length, dgroup_publics)
        scaffold_name = 'DGSCF.OBJ'
        (dos_work / scaffold_name).write_bytes(scaffold_blob)
        object_names.append(scaffold_name)
        sizing = {
            'baseline_returncode': base_result.returncode,
            'baseline_unresolved_count': sum('Undefined symbol' in line
                                             for line in base_log.splitlines()),
            'baseline_segments': base_segments,
            'target_dgroup_load': dgroup_load,
            'target_initialized_span': target_data_span,
            'target_stack_load': target_stack,
            'baseline_data_span': current_data_span,
            'baseline_stack_load': stack_segment['start'],
            'synthetic_data_bytes': data_tail_length,
            'synthetic_bss_bytes': bss_tail_length,
        }
        scaffold.append({
            'kind': 'synthetic_dgroup_data_bss',
            'object': scaffold_name,
            'staged_sha256': sha(scaffold_blob),
            'staged_size': len(scaffold_blob),
            'data_sha256': sha(data_tail),
            'data_bytes': data_tail_length,
            'bss_bytes': bss_tail_length,
            'public_count': len(dgroup_publics),
            'sizing': sizing,
        })
    result, command, log, map_path, exe_path, response = link_once('FINAL', object_names)
    unresolved = [line.strip() for line in log.splitlines()
                  if 'Undefined symbol' in line]
    errors = link_errors(log, map_path.read_text(errors='replace') if map_path.exists() else '')
    segments, rows = parse_map(map_path) if map_path.exists() else ([], [])
    code_rows = [row for row in rows if row['module'].startswith('R')]
    divergences = []
    for index, owner in enumerate(linkable_owners):
        expected_start = owner['start'] - 512
        expected_length = owner['end'] - owner['start']
        if index >= len(code_rows):
            divergences.append({'index': index, 'owner': owner['id'],
                                'reason': 'missing_linker_row'})
            break
        row = code_rows[index]
        if row['offset'] != expected_start or row['length'] != expected_length:
            divergences.append({'index': index, 'owner': owner['id'],
                                'module': row['module'],
                                'expected_start': expected_start,
                                'actual_start': row['offset'],
                                'expected_length': expected_length,
                                'actual_length': row['length']})
            break
    report = {
        'format': 'empires-tlink-structural-experiment-v1',
        'mode': ('library_toupper_with_historical_demand_and_symbol_normalization_and_case_and_internal_labels_and_dgroup_scaffold'
                 if demand_historical_library and normalize_recovered_symbols and normalize_case_symbols and expose_internal_labels and scaffold_dgroup
                 else 'library_toupper_with_historical_demand_and_symbol_normalization_and_case_and_internal_labels'
                 if demand_historical_library and normalize_recovered_symbols and normalize_case_symbols and expose_internal_labels
                 else 'library_toupper_with_historical_demand_and_symbol_normalization_and_case_and_dgroup_scaffold'
                 if demand_historical_library and normalize_recovered_symbols and normalize_case_symbols and scaffold_dgroup
                 else 'library_toupper_with_historical_demand_and_symbol_normalization_and_case'
                 if demand_historical_library and normalize_recovered_symbols and normalize_case_symbols
                 else 'library_toupper_with_historical_demand_and_symbol_normalization_and_dgroup_scaffold'
                 if demand_historical_library and normalize_recovered_symbols and scaffold_dgroup
                 else 'library_toupper_with_historical_demand_and_symbol_normalization'
                 if demand_historical_library and normalize_recovered_symbols
                 else 'library_toupper_with_historical_demand' if demand_historical_library
                 else 'library_toupper_promotion' if promote_toupper
                 else 'ordinary_c_owners'),
        'status': 'MAP_AVAILABLE' if map_path.exists() and exe_path.exists() and not errors else 'LINK_FAILED',
        'options': {'demand_historical_library': demand_historical_library,
                    'scaffold_dgroup': scaffold_dgroup,
                    'normalize_recovered_symbols': normalize_recovered_symbols,
                    'normalize_case_symbols': normalize_case_symbols,
                    'expose_internal_labels': expose_internal_labels},
        'linker': {'path': str(linker), 'sha256': sha(linker.read_bytes()),
                   'files': [{'name': p.name, 'sha256': sha(p.read_bytes())}
                             for p in linker_files(linker)]},
        'startup_object': {'path': 'C0C.OBJ', 'sha256': sha(c0c.read_bytes()),
                           'text_bytes': startup_length},
        'compile': {'owner_count': len(compile_owners),
                    'all_matching_c_owner_count': len(owners),
                    'session': compile_session},
        'relocatable_scaffold': scaffold,
        'link': {'returncode': result.returncode, 'command': command,
                 'errors': errors, 'log': log,
                 'unresolved_symbols': unresolved[:200],
                 'unresolved_count': len(unresolved),
                 'dgroup_sizing': sizing},
        'segments': segments,
        'code_rows': rows,
        'code_comparison': {'expected_owner_count': len(linkable_owners),
                            'source_owner_count': len(compile_owners),
                            'library_replacement_count': len(library_replacements),
                            'actual_code_row_count': len(code_rows),
                            'first_divergence': divergences[0] if divergences else None},
        'library_replacements': [
            {'owner': owner_id, 'module': module_name,
             'expected_start': next(o['start'] - 512 for o in owners if o['id'] == owner_id),
             'expected_length': next(o['end'] - o['start'] for o in owners if o['id'] == owner_id),
             'actual': next((row for row in rows if row['module'].upper() == module_name.upper()), None)}
            for owner_id, module_name in library_replacements.items()
        ],
        'library_comparison': {
            'target_owner': 'F_F9BE',
            'expected_start': next(o['start'] - 512 for o in owners if o['id'] == 'F_F9BE'),
            'expected_length': next(o['end'] - o['start'] for o in owners if o['id'] == 'F_F9BE'),
            'actual_toupper': next((row for row in rows if row['module'].upper() == 'TOUPPER'), None),
        },
        'outputs': {
            'exe_sha256': sha(exe_path.read_bytes()) if exe_path.exists() else None,
            'map_sha256': sha(map_path.read_bytes()) if map_path.exists() else None,
        },
        'byte_comparison': compare_linked_executable(exe_path, root / 'assets/AEPROG.EXE'),
        'interpretation': 'TLINK placement is experimental; the fixed manifest remains the oracle. A no-demand run can prove the complete _TEXT prefix while unresolved DGROUP symbols remain until reconstructed DATA/BSS sources replace the synthetic scaffold.'
    }
    write_json(root_build / 'tlink-structural-report.json', report)
    print(f"TLINK map: {report['status']}; code rows {len(code_rows)}/{len(compile_owners)}; unresolved {len(unresolved)}")
    print(f"First code divergence: {report['code_comparison']['first_divergence']}")
    print(f"Report: {root_build / 'tlink-structural-report.json'}")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--linker', type=Path, default=DEFAULT_LINKER)
    parser.add_argument('--dosbox', type=Path)
    parser.add_argument('--promote-toupper', action='store_true',
                        help='use CC.LIB TOUPPER via a temporary external-symbol normalization')
    parser.add_argument('--demand-historical-library', action='store_true',
                        help='add a temporary unresolved-public demand object for manifest library modules')
    parser.add_argument('--normalize-recovered-symbols', action='store_true',
                        help='apply verified temporary aliases for recovered function publics')
    parser.add_argument('--scaffold-dgroup', action='store_true',
                        help='size a temporary DATA/BSS OMF contribution from the baseline TLINK map')
    parser.add_argument('--normalize-case-symbols', action='store_true',
                        help='lowercase EXTDEFs only when their lowercase explicit OMF public exists')
    parser.add_argument('--expose-internal-labels', action='store_true',
                        help='add numeric labels to reconstructed C objects when their target owner is known')
    args = parser.parse_args()
    try:
        run(linker=args.linker, dosbox=args.dosbox, promote_toupper=args.promote_toupper,
            demand_historical_library=args.demand_historical_library,
            normalize_recovered_symbols=args.normalize_recovered_symbols,
            scaffold_dgroup=args.scaffold_dgroup,
            normalize_case_symbols=args.normalize_case_symbols,
            expose_internal_labels=args.expose_internal_labels)
    except (OSError, ValueError, KeyError, subprocess.SubprocessError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
