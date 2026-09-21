"""Analyze the complete runtime recursively; retain analyze() as the legacy veneer API.

The CLI emits docs/current/runtime-cfg.json, with unknown gaps kept separate.
"""
import json
from pathlib import Path

from reconstruct import ROOT, write_json


OWNER_START = 0x059C
VENEER_BYTES = 0x3C


def public_aliases(source):
    """Return aliases declared between each adjacent ``asm db`` veneer."""
    groups, current = [[]], []
    for raw in source.read_text(encoding='utf-8').splitlines():
        line = raw.strip()
        if line.startswith('asm public '):
            current.append(line.split()[-1])
        elif line.startswith('asm db '):
            groups[-1].extend(current)
            current = []
            groups.append([])
    # The source declares the block base (historically f039c, now runtime_base)
    # as the C function entry rather than inline assembly, so it is the first
    # veneer root.
    groups[0].insert(0, '_runtime_base')
    return groups[:-1]


def analyze(binary, source):
    if len(binary) < VENEER_BYTES or VENEER_BYTES % 3:
        raise ValueError('RUNTIME_BLOCK veneer extent is invalid')
    aliases = public_aliases(source)
    roots = []
    for index, offset in enumerate(range(0, VENEER_BYTES, 3)):
        if binary[offset] != 0xE9:
            raise ValueError(f'Veneer {offset:#x} is not a near jump')
        displacement = int.from_bytes(binary[offset + 1:offset + 3], 'little', signed=True)
        target = offset + 3 + displacement
        if not VENEER_BYTES <= target < len(binary):
            raise ValueError(f'Veneer {offset:#x} target escapes owner')
        roots.append({
            'veneer_offset': offset,
            'target_offset': target,
            'owner_load_offset': OWNER_START + target,
            'aliases': aliases[index] if index < len(aliases) else [],
        })
    if len(roots) != 20:
        raise ValueError('Unexpected runtime dispatch-root count')
    return {'format': 'empires-runtime-block-cfg-roots-v1',
            'owner': 'RUNTIME_BLOCK', 'bytes': len(binary),
            'veneer_bytes': VENEER_BYTES, 'root_count': len(roots),
            'roots': roots,
            'limitation': ('Dispatch roots only. Recursive branch/call recovery and explicit table '
                           'boundaries remain required before symbolic source replacement.')}


def main():
    from runtime_cfg import runtime_cfg
    from runtime_source import oracle_bytes
    report = runtime_cfg(oracle_bytes())
    (ROOT / 'docs/current').mkdir(exist_ok=True)
    write_json(ROOT / 'docs/current/runtime-cfg.json', report)
    print(f"RUNTIME_BLOCK: {len(report['basic_blocks'])} blocks, {len(report['instructions'])} reachable instructions")
    return report


if __name__ == '__main__':
    main()
