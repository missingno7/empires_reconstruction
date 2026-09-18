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


def verify_bindings(root, manifest, original):
    bindings = [b for owner in manifest['regions']
                for b in owner.get('build', {}).get('bindings', {}).values()
                if 'storage_evidence' in b]
    if not bindings:
        return
    document = json.loads((root / 'docs/storage-binding-evidence.json').read_text())
    offsets = verify(document, original, manifest['frames'])
    for binding in bindings:
        if (binding.get('coordinate') != 'DGROUP_offset' or 'owner' in binding or
                binding.get('offset') != offsets.get(binding['storage_evidence'])):
            raise ValueError('Binding contradicts verified storage evidence')
