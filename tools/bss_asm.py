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
