#!/usr/bin/env python3
"""Generate portable/tests/fixtures/resource_golden.json.

Independently decodes every record of assets/AE000.DAT and assets/AE001.DAT
using tools/resource_codecs.py (compression-stage RLE/pair-span, validated
by byte-exact archive round-trips) plus tools/portable/decode_ref.py (a
from-scratch transcription of the sprite-side ASM/C, display_mode == 4).
test_resource.c compares the C port's output against this manifest
(SHA-256 + decoded length per record).

Usage:
    python tools/portable/gen_resource_golden.py [asset_dir] [out_file]

Defaults: asset_dir = <repo>/assets, out_file =
<repo>/portable/tests/fixtures/resource_golden.json.
"""
from __future__ import annotations

import hashlib
import json
import os
import struct
import sys

_REPO_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(_REPO_ROOT, "tools"))
sys.path.insert(0, os.path.join(_REPO_ROOT, "tools", "portable"))

from resource_codecs import decode_payload  # noqa: E402
from decode_ref import decode_sprite_dispatch  # noqa: E402

ARCHIVES = [
    ("AE000", "AE000.DAT"),
    ("AE001", "AE001.DAT"),
]


def load_table(data: bytes):
    n0 = struct.unpack_from("<I", data, 0)[0]
    n = n0 // 4
    table = struct.unpack_from("<%dI" % n, data, 0)
    return table  # table[i] .. table[i+1) is record i, for i in 0..n-2


def decode_record(raw: bytes, display_mode: int = 4):
    if len(raw) < 2:
        raise ValueError("record shorter than the 2-byte header")
    gc0cb = raw[0]
    fl = raw[1]
    payload = raw[2:]
    decoded = bytearray(decode_payload(bytes(payload), fl & 3))
    decode_sprite_dispatch(decoded, gc0cb, display_mode=display_mode)
    return gc0cb, decoded


def main(argv):
    asset_dir = argv[1] if len(argv) > 1 else os.path.join(_REPO_ROOT, "assets")
    out_file = argv[2] if len(argv) > 2 else os.path.join(
        _REPO_ROOT, "portable", "tests", "fixtures", "resource_golden.json")

    manifest = {}
    total = 0
    skipped = 0

    for prefix, filename in ARCHIVES:
        path = os.path.join(asset_dir, filename)
        if not os.path.isfile(path):
            print(f"gen_resource_golden: {path} not found, skipping archive")
            continue
        with open(path, "rb") as f:
            data = f.read()
        table = load_table(data)
        n_records = len(table) - 1
        for i in range(n_records):
            key = f"{prefix}_{i:03d}"
            o1, o2 = table[i], table[i + 1]
            raw = data[o1:o2]
            total += 1
            try:
                gc0cb, decoded = decode_record(raw)
            except Exception as exc:  # noqa: BLE001 - deliberately broad, logged below
                manifest[key] = {"skipped": f"{type(exc).__name__}: {exc}"}
                skipped += 1
                continue
            manifest[key] = {
                "s": len(decoded),
                "sha256": hashlib.sha256(bytes(decoded)).hexdigest(),
                "gc0cb": gc0cb,
            }

    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    with open(out_file, "w", newline="\n") as f:
        json.dump(manifest, f, indent=2, sort_keys=True)
        f.write("\n")

    print(f"gen_resource_golden: wrote {len(manifest)} entries "
          f"({total - skipped} decoded, {skipped} skipped) to {out_file}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
