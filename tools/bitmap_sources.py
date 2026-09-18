"""Editable logical PNG plus exact display metadata, independent of game fixtures."""
import argparse
from pathlib import Path
import sys

from indexed_png import read_png, write_png
from reconstruct import read_json, write_json
from resource_formats import SOURCE_FORMATS, encode_bitmap

BUILD_SOURCE_FORMATS = (*SOURCE_FORMATS, 'bitmap4-png-v1')


def source_files(bitmap, image_name):
    # Validate before creating either file.
    encode_bitmap(bitmap)
    pixels = [bytes.fromhex(row) for row in bitmap['pixel_rows']]
    png = write_png(bitmap['row_bytes'] * 2, pixels)
    metadata = {'format': 'bitmap4-png-v1', 'ega_cga_table': bitmap['ega_cga_table'],
                'vga_table': bitmap['vga_table'], 'image': image_name}
    return metadata, png


def export_source(bitmap, metadata_path):
    image_path = metadata_path.with_suffix('.png')
    metadata, png = source_files(bitmap, image_path.name)
    metadata_path.parent.mkdir(parents=True, exist_ok=True)
    image_path.write_bytes(png)
    write_json(metadata_path, metadata)
    return metadata


def image_path(document, metadata_path):
    if document['format'] != 'bitmap4-png-v1' or set(document) != {'format', 'ega_cga_table', 'vga_table', 'image'}:
        raise ValueError('Invalid PNG bitmap metadata')
    if not isinstance(document['image'], str) or Path(document['image']).name != document['image']:
        raise ValueError('Bitmap image must be a filename beside its metadata')
    path = (metadata_path.parent / document['image']).resolve()
    if not path.is_relative_to(metadata_path.parent.resolve()):
        raise ValueError('Bitmap image escapes its source directory')
    return path


def encode_source(document, metadata_path):
    width, rows = read_png(image_path(document, metadata_path).read_bytes())
    return encode_bitmap({'format': 'bitmap4-json-v1', 'ega_cga_table': document['ega_cga_table'],
                          'vga_table': document['vga_table'], 'row_bytes': width // 2,
                          'height': len(rows), 'pixel_rows': [row.hex() for row in rows]})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=('export', 'encode'))
    parser.add_argument('source', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    try:
        if args.source.resolve() == args.output.resolve():
            raise ValueError('Source and output paths must differ')
        if args.command == 'export':
            export_source(read_json(args.source), args.output)
        else:
            payload = encode_source(read_json(args.source), args.source)
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(payload)
    except (ValueError, OSError, KeyError) as error:
        print(f'FAIL: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
