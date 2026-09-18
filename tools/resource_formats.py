"""Lossless structural sources; no guessed gameplay names or normalized unknowns."""
import struct

SOURCE_FORMATS = ('bitmap4-json-v1', 'level-parts-json-v1', 'resource-bank16-json-v1',
                  'bitmap-sequence-json-v1', 'font1-json-v1')


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


def record_document(data):
    if not data:
        return {'format': 'empty-record-v1'}
    if data[0] == 0x47:
        if len(data) < 36:
            raise ValueError('Truncated nested bitmap header')
        return {'format': 'bitmap4-record-v1', 'control': data[1], 'bitmap': bitmap_document(data[2:])}
    if len(data) >= 4 and data[0] == 0x32:
        row_bytes = (data[2] * 2 + 7) // 8
        if len(data) == 4 + row_bytes * data[3]:
            return {'format': 'bitmap1-record-v1', 'control': data[1], 'width_pairs': data[2], 'height': data[3],
                    'rows': [data[4 + y * row_bytes:4 + (y + 1) * row_bytes].hex() for y in range(data[3])]}
    return {'format': 'opaque-record-v1', 'bytes': data.hex()}


def encode_record(document):
    kind = document['format']
    if kind == 'empty-record-v1':
        return b''
    if kind == 'opaque-record-v1':
        return bytes.fromhex(document['bytes'])
    if kind == 'bitmap4-record-v1':
        return bytes((0x47, byte_value(document['control']))) + encode_bitmap(document['bitmap'])
    if kind == 'bitmap1-record-v1':
        control = byte_value(document['control'])
        width, height = byte_value(document['width_pairs']), byte_value(document['height'])
        if len(document['rows']) != height:
            raise ValueError('Monochrome row count differs from height')
        pixels = b''.join(fixed_hex(row, (width * 2 + 7) // 8, 'monochrome row') for row in document['rows'])
        return bytes((0x32, control, width, height)) + pixels
    raise ValueError(f'Unknown record format {kind}')


def bank_document(data):
    if len(data) < 2:
        raise ValueError('Missing bank offset table')
    table_size = int.from_bytes(data[:2], 'little')
    if table_size < 2 or table_size % 2 or table_size > len(data):
        raise ValueError('Invalid bank table size')
    offsets = struct.unpack_from(f'<{table_size // 2}H', data)
    if any(a > b for a, b in zip(offsets, offsets[1:])) or offsets[-1] > len(data):
        raise ValueError('Bank offsets out of order or beyond payload')
    return {'format': 'resource-bank16-json-v1',
            'records': [record_document(data[a:b]) for a, b in zip(offsets, offsets[1:])],
            'preserved_trailing': data[offsets[-1]:].hex()}


def encode_bank(document):
    if document['format'] != 'resource-bank16-json-v1':
        raise ValueError('Unknown bank format')
    records = [encode_record(r) for r in document['records']]
    offsets = [2 * (len(records) + 1)]
    for record in records:
        offsets.append(offsets[-1] + len(record))
    if offsets[-1] > 65535:
        raise ValueError('Bank offsets exceed 16 bits')
    return struct.pack(f'<{len(offsets)}H', *offsets) + b''.join(records) + bytes.fromhex(document['preserved_trailing'])


def sequence_document(data):
    records, at = [], 0
    while at < len(data):
        if len(data) - at < 36 or data[at] != 0x47:
            raise ValueError('Unowned bytes in bitmap sequence')
        end = at + 36 + data[at + 34] * data[at + 35]
        if end > len(data):
            raise ValueError('Truncated bitmap sequence record')
        records.append(record_document(data[at:end]))
        at = end
    return {'format': 'bitmap-sequence-json-v1', 'records': records}


def encode_sequence(document):
    if document['format'] != 'bitmap-sequence-json-v1':
        raise ValueError('Unknown sequence format')
    if any(r['format'] != 'bitmap4-record-v1' for r in document['records']):
        raise ValueError('Bitmap sequence contains a non-bitmap record')
    return b''.join(encode_record(r) for r in document['records'])


def font_document(data):
    if len(data) < 3:
        raise ValueError('Truncated font header')
    count, height = data[1] + 1, data[2]
    table_end = 3 + count * 3
    if len(data) < table_end:
        raise ValueError('Truncated font tables')
    cursor, glyphs = 0, []
    for i in range(count):
        width = data[3 + i]
        offset = data[3 + count + i] | (data[3 + 2 * count + i] << 8)
        # Current proven fonts are contiguous. Refuse aliases/gaps rather than
        # silently normalize a different font variant into this representation.
        if offset != cursor:
            raise ValueError('Font glyphs are not contiguous in table order')
        row_bytes = (width + 7) // 8
        end = table_end + offset + row_bytes * height
        if end > len(data):
            raise ValueError('Truncated font glyph')
        glyphs.append({'width': width, 'rows': [data[table_end + offset + y * row_bytes:
                                                   table_end + offset + (y + 1) * row_bytes].hex() for y in range(height)]})
        cursor += row_bytes * height
    return {'format': 'font1-json-v1', 'control': data[0], 'line_height': height,
            'glyphs': glyphs, 'preserved_trailing': data[table_end + cursor:].hex()}


def encode_font(document):
    if document['format'] != 'font1-json-v1' or not 1 <= len(document['glyphs']) <= 256:
        raise ValueError('Invalid font format/glyph count')
    height = byte_value(document['line_height'])
    offsets, widths, pixels = [], [], bytearray()
    for glyph in document['glyphs']:
        if len(pixels) > 65535:
            raise ValueError('Font glyph offset exceeds 16 bits')
        offsets.append(len(pixels))
        width = byte_value(glyph['width'])
        widths.append(width)
        if len(glyph['rows']) != height:
            raise ValueError('Glyph row count differs from font height')
        for row in glyph['rows']:
            pixels.extend(fixed_hex(row, (width + 7) // 8, 'glyph row'))
    header = bytes((byte_value(document['control']), len(widths) - 1, height))
    return (header + bytes(widths) + bytes(x & 255 for x in offsets) + bytes(x >> 8 for x in offsets)
            + pixels + bytes.fromhex(document['preserved_trailing']))


def decode_document(name, entry, payload):
    if name == 'AE001' and entry['index'] < 20:
        return level_document(payload)
    if entry['rtype'] == 0x47:
        return bitmap_document(payload)
    if entry['rtype'] == 1:
        return bank_document(payload)
    if entry['rtype'] == 0 and payload[:1] == b'G':
        return sequence_document(payload)
    if name == 'AE000' and entry['index'] in (0, 1):
        return font_document(payload)
    return None


def encode_document(document):
    encoders = {'bitmap4-json-v1': encode_bitmap, 'level-parts-json-v1': encode_level,
                'resource-bank16-json-v1': encode_bank, 'bitmap-sequence-json-v1': encode_sequence,
                'font1-json-v1': encode_font}
    if document['format'] not in encoders:
        raise ValueError(f"Unknown structured source format {document['format']}")
    return encoders[document['format']](document)
