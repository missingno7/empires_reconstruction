"""Exact offset-table/resource ownership and repacking for the two DAT files."""
import struct

from reconstruct import sha


def read_offsets(data):
    if len(data) < 4:
        raise ValueError('DAT file too short for first offset')
    first = struct.unpack_from('<I', data)[0]
    if first < 4 or first % 4 or first > len(data):
        raise ValueError('Invalid DAT first offset/table length')
    offsets = list(struct.unpack_from(f'<{first // 4}I', data))
    if any(a > b for a, b in zip(offsets, offsets[1:])) or offsets[-1] > len(data):
        raise ValueError('DAT offsets out of order or beyond EOF')
    for index, (start, end) in enumerate(zip(offsets, offsets[1:])):
        if 0 < end - start < 2:
            raise ValueError(f'DAT resource {index}: incomplete type/flags header')
    return offsets


def validate_manifest(manifest):
    if manifest['format'] != 'empires-dat-v1':
        raise ValueError('Unknown DAT manifest format')
    offsets = manifest['table']['offsets']
    if not isinstance(offsets, list) or not offsets:
        raise ValueError('DAT table must include its terminal offset')
    if any(type(x) is not int or not 0 <= x <= 0xFFFFFFFF for x in offsets):
        raise ValueError('DAT offsets must be unsigned 32-bit integers')
    if offsets[0] != 4 * len(offsets):
        raise ValueError('DAT first offset does not match table size')
    if len(manifest['resources']) != len(offsets) - 1:
        raise ValueError('DAT resource count differs from table')
    ids = set()
    cursor = offsets[0]
    for i, entry in enumerate(manifest['resources']):
        if entry['id'] in ids or entry['index'] != i:
            raise ValueError('Duplicate or incorrectly ordered DAT resource ID/index')
        ids.add(entry['id'])
        if entry['start'] != cursor or entry['start'] != offsets[i] or entry['end'] != offsets[i + 1]:
            raise ValueError(f"{entry['id']}: DAT table/ownership gap or overlap")
        length = entry['end'] - entry['start']
        if length < 0 or length == 1:
            raise ValueError(f"{entry['id']}: invalid DAT resource size")
        if length:
            for name in ('rtype', 'flags'):
                if type(entry[name]) is not int or not 0 <= entry[name] <= 255:
                    raise ValueError(f"{entry['id']}: invalid {name}")
            if entry['kind'] not in ('RAW_RESOURCE', 'MATCHING_RESOURCE'):
                raise ValueError(f"{entry['id']}: unsupported resource representation")
        elif entry['kind'] != 'EMPTY_RESOURCE':
            raise ValueError('Zero-length slots need explicit EMPTY_RESOURCE ownership')
        cursor = entry['end']
    trailing = manifest.get('trailing')
    if trailing:
        if trailing['start'] != cursor or trailing['end'] <= cursor:
            raise ValueError('DAT trailing ownership gap/overlap')
        cursor = trailing['end']
    if cursor != manifest['original']['size']:
        raise ValueError('DAT ownership does not reach EOF exactly')


def make_manifest(name, data):
    offsets = read_offsets(data)
    resources = []
    for i, (start, end) in enumerate(zip(offsets, offsets[1:])):
        block = data[start:end]
        entry = {'id': f'{name}_{i:03d}', 'index': i, 'start': start, 'end': end,
                 'kind': 'RAW_RESOURCE' if block else 'EMPTY_RESOURCE',
                 'rtype': block[0] if block else None, 'flags': block[1] if block else None,
                 'source': f'raw/{name}/{i:03d}.bin', 'expected_sha256': sha(block),
                 'payload_sha256': sha(block[2:])}
        resources.append(entry)
    trailing = None
    if offsets[-1] < len(data):
        trailing = {'id': f'{name}_TRAILING', 'start': offsets[-1], 'end': len(data),
                    'source': f'raw/{name}/trailing.bin', 'expected_sha256': sha(data[offsets[-1]:])}
    manifest = {'format': 'empires-dat-v1',
                'original': {'path': f'assets/{name}.DAT', 'size': len(data), 'sha256': sha(data)},
                'table': {'offsets': offsets, 'expected_sha256': sha(data[:offsets[0]])},
                'resources': resources, 'trailing': trailing}
    validate_manifest(manifest)
    return manifest


def encode_table(manifest):
    validate_manifest(manifest)
    return struct.pack(f'<{len(manifest["table"]["offsets"])}I', *manifest['table']['offsets'])


def assemble_archive(manifest, payloads, trailing=b''):
    """Encode only from owned metadata/payloads, with no original archive input."""
    table = encode_table(manifest)
    if sha(table) != manifest['table']['expected_sha256']:
        raise ValueError('DAT offset table differs from pinned metadata')
    result = bytearray(table)
    if set(payloads) != {r['id'] for r in manifest['resources']}:
        raise ValueError('Missing or extra DAT resource payload')
    for entry in manifest['resources']:
        payload = payloads[entry['id']]
        block = bytes([entry['rtype'], entry['flags']]) + payload if entry['kind'] != 'EMPTY_RESOURCE' else b''
        if entry['kind'] == 'EMPTY_RESOURCE' and payload:
            raise ValueError('Nonempty payload for an empty resource slot')
        if len(block) != entry['end'] - entry['start']:
            raise ValueError(f"{entry['id']}: resource output length {len(block)} differs from owned {entry['end']-entry['start']}")
        if sha(block) != entry['expected_sha256']:
            raise ValueError(f"{entry['id']}: resource bytes mismatch at file range 0x{entry['start']:X}..0x{entry['end']:X}")
        result.extend(block)
    owner = manifest.get('trailing')
    if owner:
        if len(trailing) != owner['end'] - owner['start'] or sha(trailing) != owner['expected_sha256']:
            raise ValueError('DAT trailing bytes differ')
    elif trailing:
        raise ValueError('Unexpected DAT trailing bytes')
    result.extend(trailing)
    if len(result) != manifest['original']['size'] or sha(result) != manifest['original']['sha256']:
        raise ValueError('Full DAT identity mismatch')
    return bytes(result)
