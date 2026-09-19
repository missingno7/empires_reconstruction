"""Lossless 20-byte records with two symbolic far pointers and byte fields."""
import struct

FORMAT = 'u16-farptr-u8-farptr-u8-tail8-v1'


def compile_records(document):
    if set(document) != {'format', 'records'} or document['format'] != FORMAT:
        raise ValueError('Invalid pointer-record document')
    data, references = bytearray(), []
    if not isinstance(document['records'], list):
        raise ValueError('Records must be a list')
    for row in document['records']:
        if set(row) != {'word', 'pointer_a', 'byte_a', 'pointer_b', 'byte_b', 'tail'}:
            raise ValueError('Invalid pointer-record fields')
        def integer(value, maximum):
            if type(value) is not int or not 0 <= value <= maximum:
                raise ValueError('Record integer outside field range')
            return value
        data.extend(struct.pack('<H', integer(row['word'], 65535)))
        for suffix in ('a', 'b'):
            pointer = row['pointer_' + suffix]
            if (set(pointer) != {'target', 'addend'} or not isinstance(pointer['target'], str)
                    or not pointer['target'] or not pointer['target'].isascii()):
                raise ValueError('Invalid symbolic pointer')
            references.append({'offset': len(data), **pointer})
            data.extend(struct.pack('<HH', integer(pointer['addend'], 65535), 0))
            data.append(integer(row['byte_' + suffix], 255))
        if not isinstance(row['tail'], list) or len(row['tail']) != 8:
            raise ValueError('Record tail requires eight bytes')
        data.extend(integer(value, 255) for value in row['tail'])
    return bytes(data), references


def bind_records(document, resolve):
    data, references = compile_records(document)
    data = bytearray(data)
    for ref in references:
        offset, segment = resolve(ref['target'])
        struct.pack_into('<HH', data, ref['offset'], offset + ref['addend'], segment)
    return bytes(data)


def records_object(document, public):
    """Emit a relocatable DATA contribution: no load addresses or final frames."""
    from omf import OmfReader
    from omf_scaffold import _record
    data, refs = compile_records(document)
    if not data or len(data) > 1000:
        raise ValueError('Record contribution must fit one bounded LEDATA record')
    def name(value):
        encoded = value.encode('ascii')
        if not 0 < len(encoded) <= 255:
            raise ValueError('OMF name length outside range')
        return bytes([len(encoded)]) + encoded
    targets = list(dict.fromkeys(r['target'] for r in refs))
    if len(targets) > 127:
        raise ValueError('Too many external targets for bounded record contribution')
    records = [_record(OmfReader.THEADR, name(public)),
               _record(OmfReader.LNAMES, name('_DATA') + name('DATA') + name('DGROUP')),
               _record(OmfReader.SEGDEF16, b'\x28' + struct.pack('<H', len(data)) + b'\x01\x02\x00'),
               _record(OmfReader.GRPDEF, b'\x03\xff\x01'),
               _record(OmfReader.PUBDEF16, b'\x01\x01' + name(public) + b'\x00\x00\x00'),
               _record(OmfReader.EXTDEF, b''.join(name(t) + b'\x00' for t in targets)),
               _record(OmfReader.LEDATA16, b'\x01\x00\x00' + data)]
    fixups = bytearray()
    for ref in refs:
        fixups.extend(struct.pack('>H', 0xCC00 | ref['offset']))
        # Segment-relative pointer32, frame = DGROUP, target = external, no displacement.
        fixups.extend(bytes([0x16, 1, targets.index(ref['target']) + 1]))
    records.extend([_record(OmfReader.FIXUPP16, bytes(fixups)),
                    _record(OmfReader.MODEND16, b'\x00')])
    return b''.join(records)
