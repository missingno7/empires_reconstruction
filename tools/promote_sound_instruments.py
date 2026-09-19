from pathlib import Path
from reconstruct import write_json
from sound_instruments import decode_sound_instruments

ROOT = Path(__file__).resolve().parents[1]


def run(output=ROOT / 'src/data/DATA_012C03_SOUND_INSTRUMENTS.json'):
    data = (ROOT / 'raw/012C03-013386.bin').read_bytes()
    write_json(output, decode_sound_instruments(data))
    print(f'Wrote {output}: {len(data)} structured sound instrument bytes')


if __name__ == '__main__':
    run()
