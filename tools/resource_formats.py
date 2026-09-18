"""Lossless structural sources; no guessed gameplay names or normalized unknowns."""


def byte_value(value):
    if type(value) is not int or not 0 <= value <= 255:
        raise ValueError('Expected unsigned byte')
    return value


def fixed_hex(value, length, field):
    data = bytes.fromhex(value)
    if len(data) != length:
        raise ValueError(f'{field}: expected {length} bytes, found {len(data)}')
    return data


def bitmap_document(data):
    if len(data) < 34:
        raise ValueError('Bitmap payload shorter than its 34-byte header')
    row_bytes, height = data[32:34]
    if len(data) != 34 + row_bytes * height:
        raise ValueError('Bitmap dimensions do not consume its complete payload')
    return {'format': 'bitmap4-json-v1', 'ega_cga_table': list(data[:16]), 'vga_table': list(data[16:32]),
            'row_bytes': row_bytes, 'height': height,
            'pixel_rows': [data[34 + y * row_bytes:34 + (y + 1) * row_bytes].hex() for y in range(height)]}


def encode_bitmap(document):
    if document['format'] != 'bitmap4-json-v1':
        raise ValueError('Unknown bitmap source format')
    palettes = []
    for field in ('ega_cga_table', 'vga_table'):
        if len(document[field]) != 16:
            raise ValueError(f'{field}: expected 16 entries')
        palettes.extend(byte_value(x) for x in document[field])
    row_bytes, height = byte_value(document['row_bytes']), byte_value(document['height'])
    if len(document['pixel_rows']) != height:
        raise ValueError('Bitmap row count differs from height')
    pixels = b''.join(fixed_hex(row, row_bytes, 'pixel row') for row in document['pixel_rows'])
    return bytes(palettes + [row_bytes, height]) + pixels


def level_document(data):
    # Two fixed parts. The header bytes also contain room links, some of which
    # cross into the first room preamble; keep the physical ownership disjoint.
    if len(data) != 2 * 13068:
        raise ValueError('Expected two 13068-byte level parts')
    parts = []
    for part in range(2):
        blob = data[part * 13068:(part + 1) * 13068]
        rooms = []
        for index in range(10):
            room = blob[64 + index * 1000:64 + (index + 1) * 1000]
            rooms.append({'preamble': room[:2].hex(),
                          'tile_rows': [room[2 + y * 38:2 + (y + 1) * 38].hex() for y in range(18)],
                          'preserved_tail': room[686:].hex()})
        parts.append({'header': blob[:64].hex(), 'rooms': rooms,
                      'separator': blob[10064:10068].hex(), 'preserved_record_block': blob[10068:].hex()})
    return {'format': 'level-parts-json-v1', 'parts': parts}


def encode_level(document):
    if document['format'] != 'level-parts-json-v1' or len(document['parts']) != 2:
        raise ValueError('Expected level-parts-json-v1 with two parts')
    output = bytearray()
    for part in document['parts']:
        output.extend(fixed_hex(part['header'], 64, 'part header'))
        if len(part['rooms']) != 10:
            raise ValueError('Expected ten room records per part')
        for room in part['rooms']:
            output.extend(fixed_hex(room['preamble'], 2, 'room preamble'))
            if len(room['tile_rows']) != 18:
                raise ValueError('Expected 18 tile rows')
            for row in room['tile_rows']:
                output.extend(fixed_hex(row, 38, 'tile row'))
            output.extend(fixed_hex(room['preserved_tail'], 314, 'room tail'))
        output.extend(fixed_hex(part['separator'], 4, 'separator'))
        output.extend(fixed_hex(part['preserved_record_block'], 3000, 'record block'))
    return bytes(output)
