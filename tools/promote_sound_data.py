"""Decode the verified sound DATA extent into canonical structured source."""
from pathlib import Path

from reconstruct import ROOT, read_json, write_json
from sound_data import decode_sound_data


def run(output=ROOT / 'src/data/DATA_01139E_SOUND.json'):
    manifest = read_json(ROOT / 'layout/manifest.json')
    owner = next(item for item in manifest['regions'] if item['id'] == 'DATA_01139E_SOUND')
    # This migration utility deliberately reads the ignored oracle slice. The
    # canonical fixed and relocatable builds consume only the emitted JSON.
    data = (ROOT / 'raw/01139E-011AC6.bin').read_bytes()
    dgroup_offset = owner['start'] - 512 - manifest['frames']['DGROUP']
    write_json(output, decode_sound_data(data, dgroup_offset))
    print(f'Wrote {output}: {len(data)} structured sound DATA bytes')


if __name__ == '__main__':
    run()
