"""Generate a real TASM BSS module from a verified reserve and public map."""
import re


SYMBOL = re.compile(r'^[A-Za-z_?$@][A-Za-z0-9_?$@]*$')


def bss_asm_source(length, publics, typed_reserves=()):
    """Emit one ordinary TASM BSS contribution.

    ``typed_reserves`` lets an evidenced source owner retain its storage shape
    instead of reducing every contribution to anonymous bytes.  Each item is a
    mapping with ``offset``, a symbolic ``name``, ``count``, and
    ``element_bytes``.  Reserves must be split at public anchors, so generated
    PUBDEF offsets remain exactly the canonical layout.
    """
    if type(length) is not int or not 0 < length <= 0xFFFF:
        raise ValueError('BSS length must fit one OMF16 segment')
    by_offset = {}
    for name, offset in publics.items():
        if not isinstance(name, str) or not SYMBOL.fullmatch(name):
            raise ValueError(f'Invalid TASM public {name!r}')
        if type(offset) is not int or not 0 <= offset <= length:
            raise ValueError(f'BSS public {name!r} is outside the reserve')
        by_offset.setdefault(offset, []).append(name)

    reserves = {}
    previous_end = 0
    for reserve in typed_reserves:
        if not isinstance(reserve, dict):
            raise ValueError('BSS typed reserve must be an object')
        offset, name = reserve.get('offset'), reserve.get('name')
        count, element_bytes = reserve.get('count'), reserve.get('element_bytes')
        if (type(offset) is not int or not isinstance(name, str) or not SYMBOL.fullmatch(name)
                or type(count) is not int or type(element_bytes) is not int
                or count <= 0 or element_bytes <= 0):
            raise ValueError('Invalid BSS typed reserve')
        size = count * element_bytes
        if not 0 <= offset < length or offset + size > length or offset < previous_end:
            raise ValueError('Overlapping or out-of-range BSS typed reserve')
        if offset in reserves:
            raise ValueError('Duplicate BSS typed reserve offset')
        reserves[offset] = (name, count, element_bytes)
        previous_end = offset + size

    lines = ["DGROUP group _BSS", "_BSS segment word public 'BSS'", 'assume ds:DGROUP']
    at = 0
    for offset in sorted(set(by_offset) | set(reserves)):
        if offset > at:
            lines.append(f'db {offset - at} dup (?)')
            at = offset
        for name in sorted(by_offset.get(offset, ())):
            lines.extend((f'public {name}', f'{name} label byte'))
        reserve = reserves.get(offset)
        if reserve:
            name, count, element_bytes = reserve
            # TASM 1.0 emits nested DUP(?) as initialized LEDATA.  Keep the
            # typed source label, but reserve the proven byte extent with its
            # single-level uninitialized form so this remains real BSS.
            lines.extend((f'{name} label byte', f'db {count * element_bytes} dup (?)'))
            at += count * element_bytes
    if at < length:
        lines.append(f'db {length - at} dup (?)')
    lines.extend(('_BSS ends', 'end', ''))
    return '\r\n'.join(lines)


def bss_slice(layout, start, end):
    """Return one contribution from an anchored canonical BSS layout.

    ``GAME_BSS.json`` records offsets in the combined game reserve.  A real
    source owner needs offsets relative to its own OMF contribution, so this
    helper makes that translation explicit without changing the canonical
    evidence map. It preserves aliases and assigns every label to the
    contribution containing its canonical offset.
    """
    if layout.get('format') != 'anchored-bss-layout-v1':
        raise ValueError('Unsupported BSS layout format')
    length = layout.get('length')
    publics = layout.get('publics')
    if type(length) is not int or not isinstance(publics, dict):
        raise ValueError('Invalid anchored BSS layout')
    if type(start) is not int or type(end) is not int or not 0 <= start < end <= length:
        raise ValueError('Invalid BSS contribution extent')
    sliced = {}
    for name, offset in publics.items():
        if not isinstance(name, str) or type(offset) is not int:
            raise ValueError('Invalid anchored BSS public')
        if start <= offset < end:
            sliced[name] = offset - start
    return {'length': end - start, 'publics': sliced}
