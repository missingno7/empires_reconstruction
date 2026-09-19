"""Generate a real TASM BSS module from a verified reserve and public map."""
import re


SYMBOL = re.compile(r'^[A-Za-z_?$@][A-Za-z0-9_?$@]*$')


def bss_asm_source(length, publics):
    if type(length) is not int or not 0 < length <= 0xFFFF:
        raise ValueError('BSS length must fit one OMF16 segment')
    by_offset = {}
    for name, offset in publics.items():
        if not isinstance(name, str) or not SYMBOL.fullmatch(name):
            raise ValueError(f'Invalid TASM public {name!r}')
        if type(offset) is not int or not 0 <= offset <= length:
            raise ValueError(f'BSS public {name!r} is outside the reserve')
        by_offset.setdefault(offset, []).append(name)
    lines = ["DGROUP group _BSS", "_BSS segment word public 'BSS'", 'assume ds:DGROUP']
    at = 0
    for offset in sorted(by_offset):
        if offset > at:
            lines.append(f'db {offset - at} dup (?)')
            at = offset
        for name in sorted(by_offset[offset]):
            lines.extend((f'public {name}', f'{name} label byte'))
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
