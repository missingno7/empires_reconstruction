"""Verify narrow storage-address evidence; this does not allocate or link BSS."""
import hashlib
import json
from mz import MZ


def verify(document, original, frames):
    if document['format'] != 'empires-storage-observations-v1':
        raise ValueError('Unknown storage evidence format')
    if hashlib.sha256(original).hexdigest() != document['original_sha256']:
        raise ValueError('Storage evidence original identity mismatch')
    mz = MZ.parse(original)
    result = {}
    for item in document['objects']:
        offset = item['offset']
        if item.get('kind') == 'indexed-storage-base':
            if item['id'] in result:
                raise ValueError('Duplicate storage declaration')
            verify_indexed_base(item, original, mz, frames)
            result[item['id']] = offset
            continue
        if item.get('kind') == 'buffer-storage':
            size = item.get('size')
            if (type(offset) is not int or not 0 <= offset <= 65535 or
                    type(size) is not int or size < 1 or offset + size > 65536):
                raise ValueError('Invalid buffer storage declaration')
            if frames['DGROUP'] + offset < mz.declared_size - mz.header_size:
                raise ValueError('Expected buffer storage beyond on-disk load image')
            accesses, functions = set(), set()
            for observation in item['observations']:
                extent = observation['function_extent']
                code = original[extent['start']:extent['end']]
                if hashlib.sha256(code).hexdigest() != extent['sha256']:
                    raise ValueError('Buffer storage observation function identity mismatch')
                at = mz.file_offset(observation['load_offset'])
                if not extent['start'] <= at or at + 3 > extent['end']:
                    raise ValueError('Buffer storage observation outside function')
                if original[at:at + 3] != b'\xb8' + offset.to_bytes(2, 'little'):
                    raise ValueError('Buffer storage address instruction mismatch')
                access = observation.get('access')
                if access not in ('read', 'write'):
                    raise ValueError('Buffer storage observation must declare read or write')
                if any(at - mz.header_size <= r['load_offset'] < at - mz.header_size + 3
                       for r in mz.relocations):
                    raise ValueError('Buffer storage observation overlaps relocation')
                accesses.add(access)
                functions.add(extent['id'])
            if accesses != {'read', 'write'} or len(functions) < 2:
                raise ValueError('Buffer storage requires independent read and write observations')
            result[item['id']] = offset
            continue
        if item['id'] in result or item['size'] != 2 or not 0 <= offset <= 65534:
            raise ValueError('Invalid storage word declaration')
        if frames['DGROUP'] + offset < mz.declared_size - mz.header_size:
            raise ValueError('Expected storage beyond on-disk load image')
        accesses, functions = set(), set()
        for observation in item['observations']:
            extent = observation['function_extent']
            code = original[extent['start']:extent['end']]
            if hashlib.sha256(code).hexdigest() != extent['sha256']:
                raise ValueError('Storage observation function identity mismatch')
            at = mz.file_offset(observation['load_offset'])
            if not extent['start'] <= at < extent['end']:
                raise ValueError('Storage observation outside function')
            # Only these observed direct-address 16-bit instruction forms are accepted.
            kind = observation['instruction']
            prefix, width, access = {
                'mov-moffs16-ax': (b'\xa3', 3, 'write'),
                'mov-word-imm16': (b'\xc7\x06', 6, 'write'),
                'mov-bx-word': (b'\x8b\x1e', 4, 'read'),
                'cmp-word-imm8': (b'\x83\x3e', 5, 'read'),
            }[kind]
            instruction = original[at:at + width]
            if at + width > extent['end'] or not instruction.startswith(prefix):
                raise ValueError('Storage observation instruction mismatch')
            if int.from_bytes(instruction[len(prefix):len(prefix) + 2], 'little') != offset:
                raise ValueError('Storage observation address mismatch')
            if any(at - mz.header_size - 1 <= r['load_offset'] < at - mz.header_size + width for r in mz.relocations):
                raise ValueError('Storage observation overlaps relocation')
            accesses.add(access)
            functions.add(extent['id'])
        if accesses != {'read', 'write'} or len(functions) < 2:
            raise ValueError('Storage word requires independent function reads and writes')
        result[item['id']] = offset
    return result


def verify_indexed_base(item, original, mz, frames):
    """Corroborate a passed DS base with an independent scaled-index read."""
    offset, stride = item['offset'], item['record_stride']
    if type(offset) is not int or not 0 <= offset <= 65535 or type(stride) is not int or not 1 <= stride <= 65535:
        raise ValueError('Invalid indexed storage base')
    if frames['DGROUP'] + offset < mz.declared_size - mz.header_size:
        raise ValueError('Expected indexed storage beyond disk image')
    forms, extents = set(), set()
    for observation in item['observations']:
        extent = observation['function_extent']
        if hashlib.sha256(original[extent['start']:extent['end']]).hexdigest() != extent['sha256']:
            raise ValueError('Indexed storage function identity mismatch')
        at = mz.file_offset(observation['load_offset'])
        kind = observation['instruction']
        width = {'push-ds-buffer-call': 15, 'ds-indexed-byte-zero-test': 19}.get(kind)
        if width is None or not extent['start'] <= at or at + width > extent['end']:
            raise ValueError('Indexed storage observation outside function or unsupported')
        data = original[at:at + width]
        if kind == 'push-ds-buffer-call':
            if data[:2] != bytes.fromhex('1e b8') or data[4:6] != bytes.fromhex('50 b8') or data[8:10] != bytes.fromhex('50 e8') or data[12:] != bytes.fromhex('83 c4 06'):
                raise ValueError('DS buffer call instruction mismatch')
            address = int.from_bytes(data[2:4], 'little')
            target = (observation['load_offset'] + 12 + int.from_bytes(data[10:12], 'little', signed=True)) & 65535
            if target != observation['call_target'] or int.from_bytes(data[6:8], 'little') != observation['record_id']:
                raise ValueError('DS buffer call target or record mismatch')
        else:
            if data[:3] != bytes.fromhex('8b c6 ba') or data[5:11] != bytes.fromhex('f7 e2 8b d8 81 c3') or data[13:] != bytes.fromhex('1e 07 26 80 3f 00'):
                raise ValueError('Indexed DS byte test instruction mismatch')
            if int.from_bytes(data[3:5], 'little') != stride:
                raise ValueError('Indexed DS byte test stride mismatch')
            address = int.from_bytes(data[11:13], 'little')
        if address != offset:
            raise ValueError('Indexed storage address mismatch')
        if any(at - mz.header_size - 1 <= r['load_offset'] < at - mz.header_size + width for r in mz.relocations):
            raise ValueError('Indexed storage observation overlaps relocation')
        forms.add(kind)
        extents.add((extent['start'], extent['end']))
    ordered = sorted(extents)
    if forms != {'push-ds-buffer-call', 'ds-indexed-byte-zero-test'} or len(ordered) < 2 or any(a[1] > b[0] for a, b in zip(ordered, ordered[1:])):
        raise ValueError('Indexed base needs independent nonoverlapping call and read functions')


def verify_bindings(root, manifest, original):
    bindings = [b for owner in manifest['regions']
                for b in owner.get('build', {}).get('bindings', {}).values()
                if 'storage_evidence' in b]
    if not bindings:
        return
    document = json.loads((root / 'docs/storage-binding-evidence.json').read_text())
    offsets = verify(document, original, manifest['frames'])
    for item in document['objects']:
        if item.get('kind') != 'indexed-storage-base':
            continue
        for observation in item['observations']:
            if observation['instruction'] != 'push-ds-buffer-call':
                continue
            target = next((r for r in manifest['regions'] if r['id'] == observation['callee_owner']), None)
            if target is None or target['kind'] != 'MATCHING_C' or MZ.parse(original).load_offset(target['start']) != observation['call_target']:
                raise ValueError('Storage call requires its matching C callee owner')
    for binding in bindings:
        if (binding.get('coordinate') != 'DGROUP_offset' or 'owner' in binding or
                binding.get('offset') != offsets.get(binding['storage_evidence'])):
            raise ValueError('Binding contradicts verified storage evidence')
