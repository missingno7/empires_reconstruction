"""Build and verify the symbolic TASM candidate for ``F_DF98``.

This is deliberately an evidence probe, not an input to the canonical link:
the complete arithmetic interval must stay one coherent OMF contribution until
its cross-function relocation ordering is recovered from source.
"""
import argparse
import tempfile
from pathlib import Path

from dos_runner import resolve_runner
from omf import OmfReader
from reconstruct import ROOT, compile_sources, read_json, read_object, sha, write_json


EXPECTED_FIXUPS = [
    {'offset': 24, 'target': 'LXMUL@', 'displacement': 0},
    {'offset': 102, 'target': 'LDIV@', 'displacement': 0},
]
EXPECTED_FIXUPP16 = ['cc01060102', 'cc18060101']


def run(root=ROOT, runner=None):
    lock = read_json(root / 'layout/toolchain.json')
    manifest = read_json(root / 'layout/manifest.json')
    owner = next(region for region in manifest['regions'] if region['id'] == 'F_DF98')
    candidate = {
        'id': 'F_DF98_SYMBOLIC',
        'kind': 'SYMBOLIC_ASM',
        'source': 'asm/F_DF98.ASM',
        'build': {'flags_append': ''},
    }
    reference = {
        'id': 'F_DF98_MECHANICAL_REFERENCE',
        'kind': owner['kind'],
        'source': owner['source'],
        'build': {'flags_append': owner['build']['flags_append']},
    }
    work = Path(tempfile.mkdtemp(prefix='fdf98-symbolic-', dir=root / 'build')).resolve()
    runner = runner or resolve_runner(lock)
    receipts, session = compile_sources(root, [candidate, reference], work, root / 'toolchain', runner, lock)
    object_path = work / receipts[candidate['id']]['object']
    raw = object_path.read_bytes()
    module = read_object(raw)
    text = module.segment_bytes('_TEXT')
    reference_object = read_object(
        (work / receipts[reference['id']]['object']).read_bytes())
    reference_text = reference_object.segment_bytes('_TEXT')
    actual_fixups = [
        {'offset': fixup['offset'], 'target': fixup['target'],
         'displacement': fixup['displacement']}
        for fixup in module.fixups_in('_TEXT')
    ]
    raw_fixupp = [body.hex() for kind, body in OmfReader.records(raw)
                  if kind == OmfReader.FIXUPP16]
    # The manifest digest is over the fully bound EXE region.  At this stage
    # both far-call operands are deliberately zero-filled OMF placeholders,
    # so compare to the freshly compiled canonical mechanical owner instead.
    if text != reference_text[:len(text)]:
        raise ValueError('F_DF98 symbolic _TEXT differs from fresh mechanical owner OMF')
    if actual_fixups != EXPECTED_FIXUPS:
        raise ValueError('F_DF98 symbolic fixup semantics differ')
    if raw_fixupp != EXPECTED_FIXUPP16:
        raise ValueError('F_DF98 symbolic FIXUPP record order differs')
    report = {
        'status': 'OBJECT_EQUAL', 'owner': 'F_DF98',
        'source': candidate['source'], 'runner': session['runner'],
        'object_path': str(object_path), 'text_bytes': len(text),
        'text_sha256': sha(text),
        'mechanical_reference_prefix_sha256': sha(reference_text[:len(text)]),
        'fixups': actual_fixups,
        'fixupp16_records': raw_fixupp,
        'limitation': ('Candidate only: the current 700-byte arithmetic OMF contribution '
                       'must remain intact until its complete relocation sequence is source-generated.'),
    }
    write_json(root / 'build/fdf98-symbolic-asm-report.json', report)
    print('F_DF98 symbolic TASM candidate: OBJECT EQUAL')
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runner', choices=('msdos-player', 'dosbox'))
    args = parser.parse_args()
    lock = read_json(ROOT / 'layout/toolchain.json')
    result = run(runner=resolve_runner(lock, backend=args.runner))
    print(result['text_sha256'])
