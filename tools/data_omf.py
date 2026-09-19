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
        if not 0 <= ref['offset'] <= len(data) - 4:
            raise ValueError('Pointer fixup outside DATA')
        positions = set(range(ref['offset'], ref['offset'] + 4))
        if positions & occupied:
            raise ValueError('Overlapping DATA pointer fixups')
        occupied |= positions
    start = 0
    while start < len(data):
        end = min(start + 1000, len(data))
        for ref in refs:
            if ref['offset'] < end < ref['offset'] + 4:
                end = ref['offset']
        records.append(_record(OmfReader.LEDATA16, b'\x01' + struct.pack('<H', start) + data[start:end]))
        fixups = bytearray()
        for ref in refs:
            if start <= ref['offset'] < end:
                fixups.extend(struct.pack('>H', 0xCC00 | (ref['offset'] - start)))
                fixups.extend(bytes([0x16, 1, targets.index(ref['target']) + 1]))
        if fixups:
            records.append(_record(OmfReader.FIXUPP16, bytes(fixups)))
        start = end
    records.append(_record(OmfReader.MODEND16, b'\x00'))
    return b''.join(records)
