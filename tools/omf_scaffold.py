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


def ensure_turbo_c_dgroup(data: bytes) -> bytes:
    """Give a standalone TASM text object Turbo C's empty DGROUP shape.

    Turbo C 2.0 emits zero-length public ``_DATA`` and ``_BSS`` SEGDEFs and
    a ``DGROUP`` GRPDEF even for a translation unit containing only code.
    TASM does not.  Those declarations have no bytes, publics, externals, or
    fixups of their own, but TLINK uses them while combining DGROUP.  The
    structural build therefore normalizes symbolic-assembly units to the
    compiler's linker-facing empty-segment topology.
    """
    before = OmfReader().read(data)
    existing_group = next((entry for entry in before.groups if entry['name'] == 'DGROUP'), None)
    existing_segments = {item['name'] for item in before.segment_defs}
    # A number of reconstructed TASM modules already declare the complete
    # topology themselves.  Preserve their untouched OMF object rather than
    # treating a mechanically redundant rewrite as an active adapter.
    if ({'_DATA', '_BSS'} <= existing_segments and existing_group and
            existing_group['segments'] == ['_BSS', '_DATA']):
        return data
    records = list(_records(data))

    names = []
    for kind, body in records:
        if kind != OmfReader.LNAMES:
            continue
        at = 0
        while at < len(body):
            size = body[at]
            if at + 1 + size > len(body):
                raise MatchError('truncated LNAMES entry')
            names.append(body[at + 1:at + 1 + size].decode('latin1'))
            at += 1 + size
    if not names:
        raise MatchError('TASM object has no LNAMES record')
    name_indices = {name: index + 1 for index, name in enumerate(names)}
    dgroup_record_indexes = []
    group_ordinal = 0
    for record_index, (kind, body) in enumerate(records):
        if kind != OmfReader.GRPDEF:
            continue
        if group_ordinal >= len(before.groups):
            raise MatchError('reader/group-record count differs')
        if before.groups[group_ordinal]['name'] == 'DGROUP':
            dgroup_record_indexes.append(record_index)
            name_indices['DGROUP'], _ = _index(body, 0)
        group_ordinal += 1
    if len(dgroup_record_indexes) > 1:
        raise MatchError('TASM object has multiple DGROUP declarations')
    additions = [name for name in ('_DATA', 'DATA', '_BSS', 'BSS', 'DGROUP')
                 if name not in name_indices and not (name == 'DGROUP' and dgroup_record_indexes)]
    for name in additions:
        name_indices[name] = len(names) + 1
        names.append(name)

    definitions = {item['name']: item['index'] for item in before.segment_defs}
    next_segment = len(before.segment_defs) + 1
    missing = []
    for name, cls in (('_DATA', 'DATA'), ('_BSS', 'BSS')):
        if name not in definitions:
            definitions[name] = next_segment
            next_segment += 1
            # word-aligned public 16-bit segment, exactly as Turbo C emits.
            body = bytes((0x48,)) + struct.pack('<H', 0) + bytes((
                name_indices[name], name_indices[cls], 1))
            missing.append((OmfReader.SEGDEF16, body))

    dgroup_body = bytes((name_indices['DGROUP'], 0xFF, definitions['_BSS'],
                         0xFF, definitions['_DATA']))
    last_segdef = max((i for i, (kind, _) in enumerate(records)
                       if kind == OmfReader.SEGDEF16), default=None)
    if last_segdef is None:
        raise MatchError('TASM object has no SEGDEF record')
    # TASM may emit LNAMES after individual SEGDEFs.  Append supplemental
    # names immediately before the GRPDEF (or next non-SEGDEF record), so no
    # existing OMF name index is renumbered.
    insertion = next((i for i, (kind, _) in enumerate(records)
                      if i > last_segdef and kind == OmfReader.GRPDEF), None)
    if insertion is None:
        insertion = next((i for i, (kind, _) in enumerate(records)
                          if i > last_segdef and kind != OmfReader.LNAMES), len(records))
    rebuilt, inserted, inserted_group = [], False, False
    group_ordinal = 0
    for index, (kind, body) in enumerate(records):
        if index == insertion:
            encoded = b''.join(bytes((len(name.encode('ascii')),)) + name.encode('ascii')
                               for name in additions)
            if additions:
                rebuilt.append((OmfReader.LNAMES, encoded))
            rebuilt.extend(missing)
            if not dgroup_record_indexes:
                rebuilt.append((OmfReader.GRPDEF, dgroup_body))
                inserted_group = True
            inserted = True
        if kind == OmfReader.GRPDEF:
            group_name = before.groups[group_ordinal]['name']
            group_ordinal += 1
            if group_name == 'DGROUP':
                rebuilt.append((OmfReader.GRPDEF, dgroup_body))
                inserted_group = True
                continue
        rebuilt.append((kind, body))
    if not inserted:
        encoded = b''.join(bytes((len(name.encode('ascii')),)) + name.encode('ascii')
                           for name in additions)
        if additions:
            rebuilt.append((OmfReader.LNAMES, encoded))
        rebuilt.extend(missing)
    if not dgroup_record_indexes and not inserted_group:
        rebuilt.append((OmfReader.GRPDEF, dgroup_body))
        inserted_group = True
    if dgroup_record_indexes and not inserted_group:
        raise MatchError('failed to replace existing DGROUP')
    if not inserted_group:
        raise MatchError('failed to insert DGROUP')
    result = b''.join(_record(kind, body) for kind, body in rebuilt)
    after = OmfReader().read(result)
    if (before.segment_bytes('_TEXT') != after.segment_bytes('_TEXT')
            or before.publics_in('_TEXT') != after.publics_in('_TEXT')
            or before.externals != after.externals
            or before.fixups_in('_TEXT') != after.fixups_in('_TEXT')):
        raise MatchError('DGROUP normalization changed text-object semantics')
    group = next((entry for entry in after.groups if entry['name'] == 'DGROUP'), None)
    if (after.segment_length('_DATA') != 0 or after.segment_length('_BSS') != 0
            or not group or group['segments'] != ['_BSS', '_DATA']):
        raise MatchError(f'DGROUP normalization did not produce Turbo C topology: '
                         f'segments={after.segment_defs!r} groups={after.groups!r}')
    return result


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


def order_explicit_fixupp_subrecords(data: bytes, segment_name: str,
                                     descending: bool = True) -> bytes:
    """Order explicit FIXUPP subrecords without changing relocation meaning.

    This deliberately refuses frame/target threads: moving a threaded fixup
    could change which earlier thread definition it consumes.  Turbo C's
    arithmetic module uses one explicit FIXUPP record, so the narrow operation
    is sufficient and independently checkable.
    """
    records = list(_records(data))
    target_segment = _text_segment_index(records) if segment_name == '_TEXT' else None
    if target_segment is None:
        module = OmfReader().read(data)
        segment = next((item for item in module.segment_defs
                        if item['name'] == segment_name), None)
        if segment is None:
            raise MatchError(f'OMF contribution has no {segment_name} SEGDEF')
        target_segment = segment['index']
    out = []
    last_segment = None
    changed = 0
    for kind, body in records:
        if kind in (OmfReader.LEDATA16, OmfReader.LIDATA16):
            last_segment, _ = _index(body, 0)
        if kind != OmfReader.FIXUPP16 or last_segment != target_segment:
            out.append((kind, body))
            continue
        at, entries = 0, []
        while at < len(body):
            start = at
            if not body[at] & 0x80:
                raise MatchError('FIXUPP ordering refuses thread subrecords')
            locat = (body[at] << 8) | body[at + 1]
            at += 2
            fixdat = body[at]
            at += 1
            frame_thread = fixdat >> 7
            frame_method = (fixdat >> 4) & 7
            target_thread = (fixdat >> 3) & 1
            no_displacement = (fixdat >> 2) & 1
            if frame_thread or target_thread:
                raise MatchError('FIXUPP ordering refuses threaded fixups')
            if frame_method in (0, 1, 2):
                _, at = _index(body, at)
            _, at = _index(body, at)
            if not no_displacement:
                at += 2
            if at > len(body):
                raise MatchError('truncated explicit FIXUPP subrecord')
            entries.append((locat & 0x3FF, body[start:at]))
        ordered = sorted(entries, key=lambda entry: entry[0], reverse=descending)
        rebuilt = b''.join(entry for _, entry in ordered)
        changed += rebuilt != body
        out.append((kind, rebuilt))
    if not changed:
        raise MatchError('FIXUPP subrecords already have the requested order')
    result = b''.join(_record(kind, body) for kind, body in out)
    before, after = OmfReader().read(data), OmfReader().read(result)
    if (before.segments != after.segments or before.segment_lengths != after.segment_lengths
            or before.publics != after.publics or before.externals != after.externals
            or sorted(before.fixups, key=repr) != sorted(after.fixups, key=repr)):
        raise MatchError('FIXUPP ordering changed relocatable object semantics')
    return result


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
        body = bytes((0, segment))
        for entry in entries:
            if len(body) + len(entry) > 1000:
                records.append(_record(OmfReader.PUBDEF16, body))
                body = bytes((0, segment))
            body += entry
        if len(body) > 2:
            records.append(_record(OmfReader.PUBDEF16, body))
    # Keep records within the historical linker's input buffer. Offsets are
    # contribution-relative; chunking changes neither layout nor contents.
    for offset in range(0, len(data), 1000):
        records.append(_record(OmfReader.LEDATA16,
                               b'\x01' + struct.pack('<H', offset) + data[offset:offset + 1000]))
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


def externalize_data_segment(data: bytes, symbol: str) -> bytes:
    """Separate initialized DATA while preserving code and its symbolic fixups.

    The empty segment declaration remains for DGROUP/frame semantics. Code
    references to its contribution become references to the new source public.
    No placement coordinate is accepted by this transformation.
    """
    original = OmfReader().read(data)
    definition = next(s for s in original.segment_defs if s['name'] == '_DATA')
    data_index = definition['index']
    if not original.segment_length('_DATA'):
        raise MatchError('externalization requires a nonempty DATA contribution')
    for public in original.publics_in('_DATA'):
        data = remove_public(data, public['name'])
    records = list(_records(data))
    external_index = len(original.externals) + 1
    if external_index > 127 or len(symbol.encode('ascii')) > 255:
        raise MatchError('externalization exceeds bounded EXTDEF encoding')
    extdef = bytes([len(symbol)]) + symbol.encode('ascii') + b'\x00'
    out, targets, last_segment, seg_index, inserted = [], {}, None, 0, False
    for kind, body in records:
        if kind in (OmfReader.LEDATA16, OmfReader.LIDATA16):
            if not inserted:
                out.append((OmfReader.EXTDEF, extdef))
                inserted = True
            last_segment, _ = _index(body, 0)
            if last_segment == data_index:
                continue
        if kind == OmfReader.EXTDEF and inserted:
            raise MatchError('late EXTDEF would invalidate externalization index')
        if kind == OmfReader.SEGDEF16:
            seg_index += 1
            if seg_index == data_index:
                if body[0] >> 5 == 0:
                    raise MatchError('absolute DATA cannot be externalized')
                body = body[:1] + b'\x00\x00' + body[3:]
        if kind == OmfReader.FIXUPP16:
            at, rebuilt = 0, bytearray()
            while at < len(body):
                begin = at
                if not body[at] & 0x80:
                    thread = body[at]
                    at += 1
                    method, number = (thread >> 2) & 7, thread & 3
                    datum = 0
                    if not (thread & 0x40 and method >= 4):
                        datum, at = _index(body, at)
                    if not thread & 0x40:
                        targets[number] = (method, datum)
                    # Keep threads even when their DATA fixups are removed;
                    # later TEXT fixups may reference them.
                    rebuilt.extend(body[begin:at])
                    continue
                at += 2
                fixdat_at = at
                fixdat = body[at]
                at += 1
                frame = (fixdat >> 4) & 7
                if not fixdat & 0x80 and frame in (0, 1, 2):
                    _, at = _index(body, at)
                target_at = at
                if fixdat & 8:
                    method, datum = targets[fixdat & 3]
                else:
                    method = fixdat & 3
                    datum, at = _index(body, at)
                displacement_at = at
                if not fixdat & 4:
                    at += 2
                if last_segment == data_index:
                    continue
                if method == 0 and datum == data_index:
                    # Retain the original frame, source location and addend.
                    rebuilt.extend(body[begin:fixdat_at])
                    rebuilt.append((fixdat & 0xF4) | 2)
                    rebuilt.extend(body[fixdat_at + 1:target_at])
                    rebuilt.append(external_index)
                    rebuilt.extend(body[displacement_at:at])
                else:
                    rebuilt.extend(body[begin:at])
            if not rebuilt:
                continue
            body = bytes(rebuilt)
        out.append((kind, body))
    if not inserted:
        raise MatchError('object has no initialized records')
    result = b''.join(_record(kind, body) for kind, body in out)
    checked = OmfReader().read(result)
    if checked.segment_length('_DATA') != 0 or checked.segment_bytes('_TEXT') != original.segment_bytes('_TEXT'):
        raise MatchError('DATA externalization changed code or retained DATA')
    expected = []
    for fixup in original.fixups_in('_TEXT'):
        fixup = dict(fixup)
        if fixup['target_kind'] == 'segment' and fixup['target'] == '_DATA':
            fixup.update(target_kind='external', target=symbol)
        expected.append(fixup)
    if checked.fixups_in('_TEXT') != expected:
        raise MatchError('DATA externalization changed unrelated TEXT fixups')
    return result
