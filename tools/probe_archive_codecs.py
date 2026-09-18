"""Compare candidate encoders with every original stream; optionally promote exact ones."""
import argparse
import copy
import sys

from dat_archive import assemble_archive, validate_manifest
from reconstruct import ROOT, project_path, read_json, sha, write_json
from reconstruct_archives import NAMES
from resource_codecs import decode_payload, encode_payload, pack_pair_span, pack_rle, unpack_pair_span


def first_difference(expected, actual):
    if expected == actual:
        return None
    offset = next((i for i, (a, b) in enumerate(zip(expected, actual)) if a != b),
                  min(len(expected), len(actual)))
    return {'offset': offset, 'expected': expected[offset] if offset < len(expected) else None,
            'actual': actual[offset] if offset < len(actual) else None}


def probe(root=ROOT, promote=False):
    rows, plans = [], []
    for name in NAMES:
        path = root / f'layout/archives/{name}.json'
        manifest = read_json(path)
        validate_manifest(manifest)
        original = project_path(root, manifest['original']['path']).read_bytes()
        if len(original) != manifest['original']['size'] or sha(original) != manifest['original']['sha256']:
            raise ValueError(f'{name}: original archive identity differs')
        changed, outputs, payloads = copy.deepcopy(manifest), [], {}
        for entry in changed['resources']:
            block = original[entry['start']:entry['end']]
            if sha(block) != entry['expected_sha256']:
                raise ValueError(f"{entry['id']}: original resource identity differs")
            payloads[entry['id']] = block[2:]
            if not block:
                continue
            decoded = decode_payload(block[2:], entry['flags'])
            if sha(decoded) != entry['decoded']['sha256'] or len(decoded) != entry['decoded']['size']:
                raise ValueError(f"{entry['id']}: decoded identity differs")
            encoded = encode_payload(decoded, entry['flags'])
            pair_input = unpack_pair_span(block[2:]) if entry['flags'] & 2 else block[2:]
            row = {'id': entry['id'], 'flags': entry['flags'], 'encoded_bytes': len(block) - 2,
                   'decoded_bytes': len(decoded), 'original_sha256': sha(block[2:]),
                   'candidate_sha256': sha(encoded), 'candidate_bytes': len(encoded),
                   'exact': encoded == block[2:], 'first_difference': first_difference(block[2:], encoded)}
            if entry['flags'] & 1:
                rle = pack_rle(decoded)
                row['rle'] = {'exact': rle == pair_input, 'bytes': len(pair_input),
                              'sha256': sha(pair_input), 'first_difference': first_difference(pair_input, rle)}
            if entry['flags'] & 2:
                candidate_pair = pack_pair_span(pair_input)
                expected_trace, actual_trace = [], []
                unpack_pair_span(block[2:], expected_trace)
                unpack_pair_span(candidate_pair, actual_trace)
                token_difference = next(({'index': i, 'original': a, 'candidate': b}
                                         for i, (a, b) in enumerate(zip(expected_trace, actual_trace)) if a != b), None)
                row['pair_span'] = {'size_header_equals_output': int.from_bytes(block[2:4], 'little') == len(pair_input),
                                    'exact': candidate_pair == block[2:], 'first_token_difference': token_difference,
                                    'latest_tie_exact': pack_pair_span(pair_input, prefer_latest=True) == block[2:]}
            rows.append(row)
            # Identity-copying uncompressed payloads is not a compression promotion.
            if promote and row['exact'] and entry['flags'] and entry['kind'] == 'RAW_RESOURCE':
                entry['kind'] = 'MATCHING_RESOURCE'
                entry['encoder'] = 'greedy-rle-pair-span-v1'
                entry['source'] = f'raw/{name}/decoded/{entry["index"]:03d}.bin'
                outputs.append((project_path(root, entry['source']), decoded))
                payloads[entry['id']] = encoded
        trailing = original[manifest['trailing']['start']:] if manifest.get('trailing') else b''
        # Verify all new ownership and encoded payloads before publishing any manifest.
        if assemble_archive(changed, payloads, trailing) != original:
            raise ValueError(f'{name}: promotion changed the archive')
        plans.append((path, changed, outputs))
    summary = {'resources': len(rows), 'uncompressed_identity': sum(not r['flags'] for r in rows),
               'compressed_exact': sum(r['exact'] and bool(r['flags']) for r in rows),
               'compressed_mismatching': sum(not r['exact'] for r in rows),
               'rle_stages': sum('rle' in r for r in rows),
               'rle_stages_exact': sum(r.get('rle', {}).get('exact', False) for r in rows),
               'pair_span_stages': sum('pair_span' in r for r in rows),
               'pair_span_stages_exact': sum(r.get('pair_span', {}).get('exact', False) for r in rows)}
    report = {'format': 'empires-codec-probe-v1', 'summary': summary,
              'encoder_policy': 'greedy-rle-pair-span-v1',
              'implementation_sha256': sha((root / 'tools/resource_codecs.py').read_bytes()), 'resources': rows}
    build = root / 'build'
    build.mkdir(parents=True, exist_ok=True)
    write_json(build / 'codec-probe.json', report)
    changed_plans = [p for p in plans if p[2]]
    if changed_plans:
        for filename in ('archives-report.json', 'game-report.json', *(n + '.DAT' for n in NAMES)):
            (build / filename).unlink(missing_ok=True)
        for path, manifest, outputs in changed_plans:
            for target, data in outputs:
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
            temporary = path.with_suffix('.json.tmp')
            write_json(temporary, manifest)
            temporary.replace(path)
    print(summary)
    print(f'Promoted {sum(len(p[2]) for p in changed_plans)} compressed resources; report: {build / "codec-probe.json"}')
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--promote', action='store_true')
    args = parser.parse_args()
    try:
        probe(promote=args.promote)
    except (ValueError, OSError, KeyError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
