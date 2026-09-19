"""Lossless source format for the initialized sound state and lookup tables."""
import struct


FORMAT = 'empires-sound-data-v1'


def _words(data):
    if len(data) % 2:
        raise ValueError('Word array has odd length')
    return list(struct.unpack('<' + 'H' * (len(data) // 2), data))


def decode_sound_data(data, dgroup_offset):
    if len(data) != 1832:
        raise ValueError('Sound DATA extent must be 1,832 bytes')
    state_words = _words(data[:0x8e])
    note_divisors = _words(data[0x8e:0xbe])
    note_targets = _words(data[0xbe:0xc2])
    if note_targets != [dgroup_offset + 0x8e, dgroup_offset + 0xa6]:
        raise ValueError('Sound note-bank pointers differ')
    opl_port = struct.unpack_from('<H', data, 0xc2)[0]
    dispatch_words = _words(data[0xc4:0x10c])
    target_offsets = [value - dgroup_offset for value in dispatch_words]
    if any(offset < 0x119 or offset >= 0x716 for offset in target_offsets):
        raise ValueError('Sound dispatch target is outside lookup data')
    boundaries = sorted(set(target_offsets)) + [0x716]
    lookup_tables = []
    for start, stop in zip(boundaries, boundaries[1:]):
        lookup_tables.append({'id': f'lookup_{start:04x}',
                              'values': list(data[start:stop])})
    return {
        'format': FORMAT,
        'owner': 'DATA_01139E_SOUND',
        'state_words': state_words,
        'note_divisors': note_divisors,
        'note_banks': ['note_divisors', 'note_divisors_octave'],
        'opl_port': opl_port,
        'dispatch': [f'lookup_{offset:04x}' for offset in target_offsets],
        'lookup_prefix': list(data[0x10c:0x119]),
        'lookup_tables': lookup_tables,
        'tail_words': _words(data[0x716:]),
    }


def compile_sound_data(document):
    required = {'format', 'owner', 'state_words', 'note_divisors', 'note_banks',
                'opl_port', 'dispatch', 'lookup_prefix', 'lookup_tables', 'tail_words'}
    if set(document) != required or document['format'] != FORMAT:
        raise ValueError('Invalid sound DATA document')
    data, refs, publics = bytearray(), [], {document['owner']: 0}
    def words(values):
        if not isinstance(values, list) or any(type(v) is not int or not 0 <= v <= 0xffff
                                                for v in values):
            raise ValueError('Invalid sound word array')
        data.extend(struct.pack('<' + 'H' * len(values), *values))
    def near_pointer(target):
        if not isinstance(target, str) or not target:
            raise ValueError('Invalid sound near-pointer target')
        refs.append({'offset': len(data), 'target': target, 'loc': 'offset16'})
        data.extend(b'\0\0')
    words(document['state_words'])
    publics['note_divisors'] = len(data)
    words(document['note_divisors'])
    if len(document['note_divisors']) % 2:
        raise ValueError('Note divisor banks must split evenly')
    bank_size = len(document['note_divisors']) // 2 * 2
    publics['note_divisors_octave'] = publics['note_divisors'] + bank_size
    for target in document['note_banks']:
        near_pointer(target)
    words([document['opl_port']])
    dispatch_at = len(data)
    for target in document['dispatch']:
        near_pointer(target)
    prefix = document['lookup_prefix']
    if not isinstance(prefix, list) or any(type(v) is not int or not 0 <= v <= 255 for v in prefix):
        raise ValueError('Invalid lookup prefix')
    data.extend(prefix)
    seen = set()
    for table in document['lookup_tables']:
        if set(table) != {'id', 'values'} or table['id'] in seen:
            raise ValueError('Invalid or duplicate lookup table')
        seen.add(table['id'])
        publics[table['id']] = len(data)
        values = table['values']
        if not isinstance(values, list) or any(type(v) is not int or not 0 <= v <= 255 for v in values):
            raise ValueError('Invalid lookup table bytes')
        data.extend(values)
    if any(ref['target'] not in publics for ref in refs):
        raise ValueError('Sound near pointer has no public target')
    words(document['tail_words'])
    if len(data) != 1832 or dispatch_at != 0xc4:
        raise ValueError('Sound DATA layout differs from verified extent')
    return bytes(data), refs, publics


def bind_sound_data(document, resolve_owner):
    data, refs, publics = compile_sound_data(document)
    base, _ = resolve_owner(document['owner'])
    result = bytearray(data)
    for ref in refs:
        struct.pack_into('<H', result, ref['offset'], base + publics[ref['target']])
    return bytes(result)
