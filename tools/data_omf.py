"""Emit byte-aligned relocatable initialized DATA source contributions."""
import struct
from omf import OmfReader
from omf_scaffold import _record


def emit_data(data, publics, refs=(), module_name='DATA'):
    def name(value):
        encoded = value.encode('ascii')
        if not 0 < len(encoded) <= 255:
            raise ValueError('Invalid OMF name')
        return bytes([len(encoded)]) + encoded
    if not 0 < len(data) <= 65535:
        raise ValueError('DATA contribution length outside OMF16 range')
    targets = list(dict.fromkeys(r['target'] for r in refs))
    if len(targets) > 127:
        raise ValueError('Too many external targets for this emitter')
    records = [_record(OmfReader.THEADR, name(module_name)),
               _record(OmfReader.LNAMES, name('_DATA') + name('DATA') + name('DGROUP')),
               _record(OmfReader.SEGDEF16, b'\x28' + struct.pack('<H', len(data)) + b'\x01\x02\x00'),
               _record(OmfReader.GRPDEF, b'\x03\xff\x01')]
    for symbol, offset in publics.items():
        if not 0 <= offset < len(data):
            raise ValueError('Public outside source contribution')
        records.append(_record(OmfReader.PUBDEF16, b'\x01\x01' + name(symbol)
                               + struct.pack('<H', offset) + b'\x00'))
    for target in targets:
        records.append(_record(OmfReader.EXTDEF, name(target) + b'\x00'))
    occupied = set()
    for ref in refs:
        loc = ref.get('loc', 'pointer32')
        width = 4 if loc == 'pointer32' else 2 if loc == 'offset16' else 0
        if not width:
            raise ValueError('Unsupported DATA fixup location type')
        if not 0 <= ref['offset'] <= len(data) - width:
            raise ValueError('Pointer fixup outside DATA')
        positions = set(range(ref['offset'], ref['offset'] + width))
        if positions & occupied:
            raise ValueError('Overlapping DATA pointer fixups')
        occupied |= positions
    start = 0
    while start < len(data):
        end = min(start + 1000, len(data))
        for ref in refs:
            width = 4 if ref.get('loc', 'pointer32') == 'pointer32' else 2
            if ref['offset'] < end < ref['offset'] + width:
                end = ref['offset']
        records.append(_record(OmfReader.LEDATA16, b'\x01' + struct.pack('<H', start) + data[start:end]))
        fixups = bytearray()
        # Fresh Turbo C emits DATA fixups in descending source-offset order.
        for ref in sorted(refs, key=lambda r: -r['offset']):
            if start <= ref['offset'] < end:
                locat = 0xCC00 if ref.get('loc', 'pointer32') == 'pointer32' else 0xC400
                fixups.extend(struct.pack('>H', locat | (ref['offset'] - start)))
                fixups.extend(bytes([0x16, 1, targets.index(ref['target']) + 1]))
        if fixups:
            records.append(_record(OmfReader.FIXUPP16, bytes(fixups)))
        start = end
    records.append(_record(OmfReader.MODEND16, b'\x00'))
    return b''.join(records)
