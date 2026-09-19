"""Mechanical OMF contribution adapters for the relocatable experiment path."""
from __future__ import annotations
import struct

from omf import OmfReader, MatchError


def _records(data: bytes):
    at = 0
    while at < len(data):
        if at + 3 > len(data):
            raise MatchError('truncated OMF record')
        kind = data[at]
        length = struct.unpack_from('<H', data, at + 1)[0]
        end = at + 3 + length
        if end > len(data) or length < 1:
            raise MatchError('invalid OMF record length')
        yield kind, data[at + 3:end - 1]
        at = end


def _index(body: bytes, at: int):
    if body[at] < 0x80:
        return body[at], at + 1
    return ((body[at] & 0x7F) << 8) | body[at + 1], at + 2


def _record(kind: int, body: bytes) -> bytes:
    length = len(body) + 1
    header = bytes((kind,)) + struct.pack('<H', length) + body
    return header + bytes(((-sum(header)) & 0xFF,))


def make_text_padding(length: int, name: str = 'PAD') -> bytes:
    """Build a minimal relocatable public ``_TEXT`` zero contribution."""
    if length <= 0 or length > 0xFFFF:
        raise MatchError(f'invalid padding length {length}')

    def lname(value: str) -> bytes:
        encoded = value.encode('ascii')
        return bytes((len(encoded),)) + encoded

    theadr = bytes((len(name),)) + name.encode('ascii')
    lnames = lname('_TEXT') + lname('CODE') + lname('')
    segdef = bytes((0x28,)) + struct.pack('<H', length) + bytes((1, 2, 3))
    ledata = bytes((1, 0, 0)) + bytes(length)
    data = b''.join((_record(OmfReader.THEADR, theadr),
                     _record(OmfReader.LNAMES, lnames),
                     _record(OmfReader.SEGDEF16, segdef),
                     _record(OmfReader.LEDATA16, ledata),
                     _record(OmfReader.MODEND16, b'\x00')))
    checked = OmfReader().read(data)
    if checked.segment_length('_TEXT') != length:
        raise MatchError('padding SEGDEF length did not round-trip')
    if checked.segment_bytes('_TEXT') != bytes(length):
        raise MatchError('padding bytes did not round-trip')
    return data


def _text_segment_index(records):
    names = []
    segment_index = 0
    for kind, body in records:
        if kind == OmfReader.LNAMES:
            at = 0
            while at < len(body):
                size = body[at]
                names.append(body[at + 1:at + 1 + size].decode('latin1'))
                at += 1 + size
        elif kind == OmfReader.SEGDEF16:
            segment_index += 1
            acbp = body[0]
            at = 1 + (3 if (acbp >> 5) == 0 else 0) + 2
            name_index, _ = _index(body, at)
            if 0 < name_index <= len(names) and names[name_index - 1] == '_TEXT':
                return segment_index
    raise MatchError('OMF contribution has no _TEXT SEGDEF')


def trim_text_contribution(data: bytes, length: int) -> bytes:
    """Return a fresh object whose `_TEXT` SEGDEF and data end at `length`.

    The function only removes bytes after the requested extent. It refuses a
    contribution with FIXUPP locations beyond that extent because silently
    dropping a relocation would make the scaffold unsound.
    """
    original = OmfReader().read(data)
    declared = original.segment_length('_TEXT')
    if declared is None or length < 0 or length > declared:
        raise MatchError(f'invalid _TEXT trim {length} for declared length {declared}')
    if length == declared:
        return data
    if any(f['offset'] + f['width'] > length for f in original.fixups_in('_TEXT')):
        raise MatchError('cannot trim OMF contribution across a FIXUPP location')
    records = list(_records(data))
    text_index = _text_segment_index(records)
    out = []
    for kind, body in records:
        if kind == OmfReader.SEGDEF16:
            acbp = body[0]
            at = 1 + (3 if (acbp >> 5) == 0 else 0)
            old_length = struct.unpack_from('<H', body, at)[0]
            # Only the selected _TEXT SEGDEF is shortened.
            name_at = at + 2
            name_index, _ = _index(body, name_at)
            # Recover the name from the original metadata by SEGDEF index.
            seg_index = len([1 for k, _ in out if k == OmfReader.SEGDEF16]) + 1
            if seg_index == text_index:
                if old_length != declared:
                    raise MatchError('SEGDEF length differs from parsed object')
                body = body[:at] + struct.pack('<H', length) + body[at + 2:]
        elif kind == OmfReader.LEDATA16:
            seg_index, at = _index(body, 0)
            offset = struct.unpack_from('<H', body, at)[0]
            payload_at = at + 2
            if seg_index == text_index and offset + len(body[payload_at:]) > length:
                keep = max(0, length - offset)
                body = body[:payload_at + keep]
        elif kind == OmfReader.LIDATA16:
            seg_index, at = _index(body, 0)
            offset = struct.unpack_from('<H', body, at)[0]
            if seg_index == text_index and offset >= length:
                raise MatchError('cannot trim an opaque LIDATA block')
        out.append((kind, body))
    rebuilt = b''.join(_record(kind, body) for kind, body in out)
    checked = OmfReader().read(rebuilt)
    if checked.segment_length('_TEXT') != length:
        raise MatchError('trimmed OMF SEGDEF length did not round-trip')
    if checked.segment_bytes('_TEXT') != original.segment_bytes('_TEXT')[:length]:
        raise MatchError('trimmed OMF bytes differ from the source prefix')
    return rebuilt
