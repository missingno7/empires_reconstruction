"""Lossless logical-index PNG interchange (4/8-bit indexed, non-interlaced).

Palette entries label logical pixel indices; DOS colour tables stay in JSON.
No colour quantization, palette remapping, or conversion from RGB is performed.
"""
import struct
import zlib

SIGNATURE = b'\x89PNG\r\n\x1a\n'
PALETTE = bytes(channel for index in range(16) for channel in (index * 17,) * 3)


def chunk(kind, data):
    return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff)


def write_png(width, rows):
    if type(width) is not int or width <= 0 or width % 2 or not rows:
        raise ValueError('Logical PNG requires positive even width and nonempty rows')
    if any(len(row) != width // 2 for row in rows):
        raise ValueError('Packed pixel row length differs from PNG width')
    header = struct.pack('>IIBBBBB', width, len(rows), 4, 3, 0, 0, 0)
    scanlines = b''.join(b'\0' + row for row in rows)
    return (SIGNATURE + chunk(b'IHDR', header) + chunk(b'PLTE', PALETTE)
            + chunk(b'IDAT', zlib.compress(scanlines, 9)) + chunk(b'IEND', b''))


def paeth(a, b, c):
    p = a + b - c
    da, db, dc = abs(p - a), abs(p - b), abs(p - c)
    return a if da <= db and da <= dc else (b if db <= dc else c)


def read_png(data):
    if not data.startswith(SIGNATURE):
        raise ValueError('Invalid PNG signature')
    position, header, palette, compressed = 8, None, None, bytearray()
    ended, idat_seen, idat_closed = False, False, False
    while position < len(data):
        if position + 12 > len(data):
            raise ValueError('Truncated PNG chunk')
        length = struct.unpack_from('>I', data, position)[0]
        kind = data[position + 4:position + 8]
        end = position + 12 + length
        if end > len(data):
            raise ValueError('Truncated PNG chunk body')
        body = data[position + 8:end - 4]
        crc = struct.unpack_from('>I', data, end - 4)[0]
        if crc != zlib.crc32(kind + body) & 0xffffffff:
            raise ValueError('PNG CRC mismatch')
        if header is None and kind != b'IHDR':
            raise ValueError('PNG must start with IHDR')
        if idat_seen and kind != b'IDAT':
            idat_closed = True
        if kind == b'IHDR':
            if header is not None or length != 13:
                raise ValueError('Invalid or duplicate PNG header')
            header = struct.unpack('>IIBBBBB', body)
        elif kind == b'PLTE':
            if palette is not None or idat_seen or length % 3 or not 3 <= length <= 768:
                raise ValueError('Invalid PNG palette')
            palette = body
        elif kind == b'IDAT':
            if palette is None or idat_closed:
                raise ValueError('Invalid PNG data chunk order')
            idat_seen = True
            compressed.extend(body)
        elif kind == b'IEND':
            if length or not idat_seen or end != len(data):
                raise ValueError('Invalid PNG end or trailing bytes')
            ended = True
        elif kind == b'tRNS':
            raise ValueError('Logical-index PNG must remain opaque; DOS transparency is index-based')
        elif not (kind[0] & 32):
            raise ValueError(f'Unsupported critical PNG chunk {kind!r}')
        position = end
    if not ended or header is None or palette is None:
        raise ValueError('Incomplete PNG')
    width, height, depth, colour, compression, filtering, interlace = header
    if colour != 3 or depth not in (4, 8) or compression or filtering or interlace:
        raise ValueError('Expected non-interlaced indexed PNG at 4 or 8 bits')
    if not 0 < width <= 510 or width % 2 or not 0 < height <= 255:
        raise ValueError('PNG dimensions do not fit a DOS bitmap')
    if palette[:48] != PALETTE or (depth == 4 and len(palette) != 48):
        raise ValueError('PNG palette was changed/reordered; preserve the 16 logical index labels')
    stride = width // 2 if depth == 4 else width
    expected = (stride + 1) * height
    decoder = zlib.decompressobj()
    try:
        scanlines = decoder.decompress(bytes(compressed), expected + 1)
    except zlib.error as error:
        raise ValueError(f'Invalid PNG deflate stream: {error}') from error
    if len(scanlines) != expected or not decoder.eof or decoder.unused_data or decoder.unconsumed_tail:
        raise ValueError('PNG decompressed size/termination differs from dimensions')
    previous = bytearray(stride)
    rows = []
    for y in range(height):
        at = y * (stride + 1)
        method, row = scanlines[at], bytearray(scanlines[at + 1:at + 1 + stride])
        if method > 4:
            raise ValueError('Unknown PNG scanline filter')
        for x in range(stride):
            left, up, upper_left = row[x - 1] if x else 0, previous[x], previous[x - 1] if x else 0
            prediction = (0, left, up, (left + up) // 2, paeth(left, up, upper_left))[method]
            row[x] = (row[x] + prediction) & 255
        previous = row
        if depth == 8:
            if any(value > 15 for value in row):
                raise ValueError('PNG uses a pixel index outside the DOS 4-bit range')
            row = bytes((row[x] << 4) | row[x + 1] for x in range(0, width, 2))
        rows.append(bytes(row))
    return width, rows
