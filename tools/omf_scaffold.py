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


def rename_external(data: bytes, old: str, new: str) -> bytes:
    """Rename an EXTDEF symbol while preserving all OMF reference indices."""
    if not old or not new or len(new.encode('ascii')) > 255:
        raise MatchError('invalid external symbol rename')
    records = list(_records(data))
    changed = 0
    out = []
    for kind, body in records:
        if kind in (OmfReader.EXTDEF, OmfReader.LEXTDEF):
            at = 0
            rebuilt = bytearray()
            while at < len(body):
                size = body[at]
                end = at + 1 + size
                if end > len(body):
                    raise MatchError('truncated EXTDEF name')
                name = body[at + 1:end].decode('latin1')
                _, type_end = _index(body, end)
                replacement = new.encode('ascii') if name == old else body[at + 1:end]
                if name == old:
                    changed += 1
                rebuilt.append(len(replacement))
                rebuilt.extend(replacement)
                rebuilt.extend(body[end:type_end])
                at = type_end
            body = bytes(rebuilt)
        out.append((kind, body))
    if changed != 1:
        raise MatchError(f'expected one EXTDEF {old!r}, found {changed}')
    renamed = b''.join(_record(kind, body) for kind, body in out)
    checked = OmfReader().read(renamed)
    if old in checked.externals or new not in checked.externals:
        raise MatchError(f'external symbol rename {old!r} -> {new!r} did not round-trip')
    return renamed


def normalize_external_case(data: bytes, public_names) -> bytes:
    """Lowercase EXTDEF names only when a matching explicit public exists."""
    public_names = set(public_names)
    records = list(_records(data))
    changed = 0
    out = []
    for kind, body in records:
        if kind in (OmfReader.EXTDEF, OmfReader.LEXTDEF):
            at = 0
            rebuilt = bytearray()
            while at < len(body):
                size = body[at]
                end = at + 1 + size
                if end > len(body):
                    raise MatchError('truncated EXTDEF name')
                name = body[at + 1:end].decode('latin1')
                _, type_end = _index(body, end)
                lowered = name.lower()
                replacement = lowered.encode('ascii') if lowered in public_names else body[at + 1:end]
                if replacement != body[at + 1:end]:
                    changed += 1
                rebuilt.append(len(replacement))
                rebuilt.extend(replacement)
                rebuilt.extend(body[end:type_end])
                at = type_end
            body = bytes(rebuilt)
        out.append((kind, body))
    if not changed:
        return data
    renamed = b''.join(_record(kind, body) for kind, body in out)
    OmfReader().read(renamed)
    return renamed


def add_publics(data: bytes, publics, segment_name: str = '_TEXT') -> bytes:
    """Add verified labels to an existing segment without changing its bytes."""
    if not publics:
        return data
    original = OmfReader().read(data)
    segment = next((item for item in original.segment_defs
                    if item['name'] == segment_name), None)
    if segment is None:
        raise MatchError(f'object has no {segment_name} segment')
    existing = {public['name'] for public in original.publics}
    entries = []
    for name, offset in sorted(publics.items()):
        encoded = name.encode('ascii')
        if not encoded or len(encoded) > 255 or not 0 <= offset <= segment['length']:
            raise MatchError(f'invalid public {name!r} at {offset}')
        if name in existing:
            continue
        entries.append(bytes((len(encoded),)) + encoded
                       + struct.pack('<H', offset) + b'\x00')
    if not entries:
        return data
    body = bytes((0, segment['index'])) + b''.join(entries)
    records = list(_records(data))
    rebuilt = []
    inserted = False
    for kind, record_body in records:
        if kind == OmfReader.MODEND16 and not inserted:
            rebuilt.append((OmfReader.PUBDEF16, body))
            inserted = True
        rebuilt.append((kind, record_body))
    result = b''.join(_record(kind, record_body) for kind, record_body in rebuilt)
    checked = OmfReader().read(result)
    added = {name for name in publics if name not in existing}
    if not added.issubset({public['name'] for public in checked.publics}):
        raise MatchError('added public labels did not round-trip')
    return result


def remove_public(data: bytes, name: str) -> bytes:
    """Remove one PUBDEF entry without changing initialized bytes or fixups."""
    records = list(_records(data))
    removed = 0
    rebuilt = []
    for kind, body in records:
        if kind not in (OmfReader.PUBDEF16, OmfReader.PUBDEF32):
            rebuilt.append((kind, body))
            continue
        at = 0
        group, at = _index(body, at)
        segment, at = _index(body, at)
        entries = []
        while at < len(body):
            size = body[at]
            end = at + 1 + size
            if end + 2 > len(body):
                raise MatchError('truncated PUBDEF name')
            public_name = body[at + 1:end].decode('latin1')
            offset_end = end + (4 if kind == OmfReader.PUBDEF32 else 2)
            _, type_at = _index(body, offset_end)
            entry = body[at:type_at]
            if public_name == name:
                removed += 1
            else:
                entries.append(entry)
            at = type_at
        if entries:
            # PUBDEF headers are the two variable-length indices at the front.
            header_end = 0
            _, header_end = _index(body, header_end)
            _, header_end = _index(body, header_end)
            rebuilt.append((kind, body[:header_end] + b''.join(entries)))
        elif removed:
            continue
        else:
            rebuilt.append((kind, body))
    if removed != 1:
        raise MatchError(f'expected one PUBDEF {name!r}, found {removed}')
    result = b''.join(_record(kind, record_body) for kind, record_body in rebuilt)
    checked = OmfReader().read(result)
    if name in {public['name'] for public in checked.publics}:
        raise MatchError(f'PUBDEF removal did not round-trip for {name!r}')
    return result


def rename_external_addend(data: bytes, old: str, new: str, delta: int) -> bytes:
    """Rename an external and add a segment-relative offset to its FIXUPPs."""
    renamed = rename_external(data, old, new) if old != new else data
    module = OmfReader().read(renamed)
    try:
        external_index = module.externals.index(new) + 1
    except ValueError as error:
        raise MatchError(f'renamed external {new!r} is absent') from error
    if not -0xFFFF <= delta <= 0xFFFF:
        raise MatchError(f'invalid external addend {delta}')
    records = list(_records(renamed))
    frame_threads = {}
    target_threads = {}
    out = []
    changed = 0
    pending_data_patches = []
    last_segment = None
    last_offset = 0
    for kind, body in records:
        if kind == OmfReader.LEDATA16:
            segment_index, data_at = _index(body, 0)
            last_offset = struct.unpack_from('<H', body, data_at)[0]
            last_segment = segment_index
            out.append((kind, body))
            continue
        if kind != OmfReader.FIXUPP16:
            out.append((kind, body))
            continue
        body = bytearray(body)
        at = 0
        while at < len(body):
            if body[at] & 0x80:
                locat = (body[at] << 8) | body[at + 1]
                at += 2
                fixdat = body[at]
                at += 1
                frame_bit = fixdat >> 7
                frame_field = (fixdat >> 4) & 7
                target_bit = (fixdat >> 3) & 1
                no_displacement = (fixdat >> 2) & 1
                target_field = fixdat & 3
                self_relative = not ((locat >> 6) & 1)
                if frame_bit:
                    frame_method, _ = frame_threads.get(frame_field & 3, (None, 0))
                else:
                    frame_method = frame_field
                    if frame_method in (0, 1, 2):
                        _, at = _index(body, at)
                if target_bit:
                    target_method, target_index = target_threads.get(target_field & 3,
                                                                     (None, 0))
                else:
                    target_method = target_field
                    target_index, at = _index(body, at)
                if not no_displacement:
                    displacement_at = at
                    displacement = struct.unpack_from('<H', body, at)[0]
                    at += 2
                    if target_method in (2, 6) and target_index == external_index:
                        struct.pack_into('<H', body, displacement_at,
                                         (displacement + delta) & 0xFFFF)
                        changed += 1
                elif target_method in (2, 6) and target_index == external_index:
                    width = OmfReader.LOC_WIDTH.get((locat >> 10) & 0xF, 2)
                    pending_data_patches.append((last_segment, last_offset + (locat & 0x3FF), width))
                    changed += 1
            else:
                thread = body[at]
                at += 1
                is_frame = (thread >> 6) & 1
                method = (thread >> 2) & 7
                number = thread & 3
                datum = 0
                if not (is_frame and method in (4, 5, 6)):
                    datum, at = _index(body, at)
                if is_frame:
                    frame_threads[number] = (method, datum)
                else:
                    target_threads[number] = (method, datum)
        out.append((kind, bytes(body)))
    if pending_data_patches:
        patched = []
        for kind, body in out:
            if kind != OmfReader.LEDATA16:
                patched.append((kind, body))
                continue
            segment_index, data_at = _index(body, 0)
            offset = struct.unpack_from('<H', body, data_at)[0]
            payload_at = data_at + 2
            mutable = bytearray(body)
            for patch_segment, patch_offset, width in pending_data_patches:
                if patch_segment != segment_index:
                    continue
                relative = patch_offset - offset
                if relative < 0 or relative + width > len(mutable) - payload_at:
                    continue
                value = int.from_bytes(mutable[payload_at + relative:
                                              payload_at + relative + width], 'little')
                mutable[payload_at + relative:payload_at + relative + width] = (
                    ((value + delta) & ((1 << (width * 8)) - 1)).to_bytes(width, 'little'))
            patched.append((kind, bytes(mutable)))
        out = patched
    if changed == 0:
        raise MatchError(f'external {old!r} has no adjustable FIXUPP')
    result = b''.join(_record(kind, body) for kind, body in out)
    OmfReader().read(result)
    return result


def make_external_demand(names, module_name: str = 'LIBDEMAND') -> bytes:
    """Build a temporary OMF object containing unresolved external demands."""
    names = sorted(set(names))
    if not names:
        raise MatchError('external demand set is empty')
    if any(len(name.encode('ascii')) > 255 for name in names):
        raise MatchError('external demand name is too long')
    body = b''.join(bytes((len(name),)) + name.encode('ascii') + b'\x00'
                    for name in names)
    theadr = bytes((len(module_name),)) + module_name.encode('ascii')
    data = b''.join((_record(OmfReader.THEADR, theadr),
                     _record(OmfReader.EXTDEF, body),
                     _record(OmfReader.MODEND16, b'\x00')))
    checked = OmfReader().read(data, module_name + '.OBJ')
    if set(checked.externals) != set(names):
        raise MatchError('external demand object did not round-trip')
    return data


def make_dgroup_scaffold(data: bytes, bss_length: int, publics=None,
                         module_name: str = 'DGSCF') -> bytes:
    """Build a temporary grouped DATA/BSS contribution for linker sizing."""
    if len(data) > 0xFFFF or not 0 <= bss_length <= 0xFFFF:
        raise MatchError('synthetic DGROUP contribution exceeds OMF16 limits')

    def lname(value: str) -> bytes:
        encoded = value.encode('ascii')
        return bytes((len(encoded),)) + encoded

    theadr = bytes((len(module_name),)) + module_name.encode('ascii')
    lnames = b''.join(lname(value) for value in
                      ('_DATA', 'DATA', '_BSS', 'BSS', 'DGROUP'))
    data_seg = bytes((0x48,)) + struct.pack('<H', len(data)) + bytes((1, 2, 1))
    bss_seg = bytes((0x48,)) + struct.pack('<H', bss_length) + bytes((3, 4, 1))
    group = bytes((5, 0xFF, 2, 0xFF, 1))
    records = [_record(OmfReader.THEADR, theadr),
               _record(OmfReader.LNAMES, lnames),
               _record(OmfReader.SEGDEF16, data_seg),
               _record(OmfReader.SEGDEF16, bss_seg),
               _record(OmfReader.GRPDEF, group)]
    public_records = {1: [], 2: []}
    for name, value in sorted((publics or {}).items()):
        segment, offset = value
        if segment == '_DATA':
            segment = 1
        elif segment == '_BSS':
            segment = 2
        if segment not in public_records or not 0 <= offset <= 0xFFFF:
            raise MatchError(f'invalid synthetic public {name!r}')
        encoded = name.encode('ascii')
        if not encoded or len(encoded) > 255:
            raise MatchError(f'invalid synthetic public name {name!r}')
        public_records[segment].append(bytes((len(encoded),)) + encoded
                                        + struct.pack('<H', offset) + b'\x00')
    for segment, entries in public_records.items():
        if entries:
            records.append(_record(OmfReader.PUBDEF16,
                                   bytes((0, segment)) + b''.join(entries)))
    if data:
        records.append(_record(OmfReader.LEDATA16, bytes((1, 0, 0)) + data))
    records.append(_record(OmfReader.MODEND16, b'\x00'))
    result = b''.join(records)
    checked = OmfReader().read(result, module_name + '.OBJ')
    if checked.segment_length('_DATA') != len(data) or checked.segment_length('_BSS') != bss_length:
        raise MatchError('synthetic DGROUP lengths did not round-trip')
    if checked.groups[0]['name'] != 'DGROUP':
        raise MatchError('synthetic DGROUP group did not round-trip')
    return result


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
