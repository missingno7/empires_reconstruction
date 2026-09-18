"""Strict resource decoders and candidate encoders for documented loader formats.

The pair-span codec is not conventional LZW. See docs/archive-formats.md.
Decoders reject malformed/truncated streams rather than dropping bytes.
"""
import struct


def unpack_rle(data, max_output=8 * 1024 * 1024):
    output = bytearray()
    at = 0
    while at < len(data):
        control = data[at]
        at += 1
        if 0 < control < 128:
            if at + control > len(data):
                raise ValueError('Truncated RLE literal')
            output.extend(data[at:at + control])
            at += control
        else:
            if at == len(data):
                raise ValueError('Truncated RLE repeat')
            count = 257 - control if control >= 128 else 1
            output.extend(bytes([data[at]]) * count)
            at += 1
        if len(output) > max_output:
            raise ValueError('RLE output exceeds configured limit')
    return bytes(output)


def pack_rle(data):
    """Greedy runs of 2..129; literals end before the next run or at 127 bytes."""
    output = bytearray()
    at = 0
    while at < len(data):
        end = at + 1
        while end < len(data) and end - at < 129 and data[end] == data[at]:
            end += 1
        if end - at >= 2:
            output.extend((257 - (end - at), data[at]))
            at = end
        else:
            end = at
            while end < len(data) and end - at < 127:
                if end + 1 < len(data) and data[end] == data[end + 1]:
                    break
                end += 1
            output.append(end - at)
            output.extend(data[at:end])
            at = end
    return bytes(output)


class BitReader:
    def __init__(self, data):
        self.data = data
        self.position = 0

    def read(self, width):
        if self.position + width > len(self.data) * 8:
            raise ValueError('Truncated pair-span code')
        value = 0
        for _ in range(width):
            value = (value << 1) | ((self.data[self.position // 8] >> (7 - self.position % 8)) & 1)
            self.position += 1
        return value


class BitWriter:
    def __init__(self):
        self.data = bytearray()
        self.pending = 0
        self.count = 0

    def write(self, value, width):
        if not 0 <= value < 1 << width:
            raise ValueError('Code does not fit its bit width')
        self.pending = (self.pending << width) | value
        self.count += width
        while self.count >= 8:
            self.count -= 8
            self.data.append((self.pending >> self.count) & 255)
            self.pending &= (1 << self.count) - 1

    def finish(self):
        if self.count:
            self.data.append(self.pending << (8 - self.count))
        return bytes(self.data)


def unpack_pair_span(data, trace=None):
    if len(data) < 2:
        raise ValueError('Missing pair-span output-size header')
    size = struct.unpack_from('<H', data)[0]
    reader = BitReader(data[2:])
    width, emitted = 9, 0
    output = bytearray()
    checkpoints = [0]
    while len(output) < size:
        bit_offset = reader.position
        code = reader.read(width)
        if code == 256:
            width += 1
            if width > 16:
                raise ValueError('Pair-span width exceeds 16 bits')
            continue
        start = len(output)
        if code < 256:
            output.append(code)
        else:
            index = code - 257
            if index + 1 >= len(checkpoints):
                raise ValueError(f'Invalid pair-span reference {code}')
            output.extend(output[checkpoints[index]:checkpoints[index + 1]])
        if trace is not None:
            trace.append({'code': code, 'width': width, 'bit_offset': bit_offset,
                          'output_offset': start, 'output_bytes': len(output) - start})
        emitted += 1
        if emitted % 2 == 0:
            checkpoints.append(len(output))
        if len(output) > size:
            raise ValueError('Pair-span output exceeds declared size')
    remaining = len(reader.data) * 8 - reader.position
    if remaining > 7 or (remaining and reader.read(remaining) != 0):
        raise ValueError('Unexpected pair-span trailing bits/bytes')
    return bytes(output)


def pack_pair_span(data, prefer_latest=False):
    """Longest established pair-span, growing width only when the code needs it."""
    if len(data) > 65535:
        raise ValueError('Pair-span input exceeds 16-bit size header')
    trie = {}
    writer = BitWriter()
    at, pair_start, emitted, width = 0, 0, 0, 9
    while at < len(data):
        node = trie
        end, best_end, code = at, at + 1, data[at]
        while end < len(data) and data[end] in node:
            node = node[data[end]]
            end += 1
            if None in node:
                best_end, code = end, node[None]
        while code >= 1 << width:
            writer.write(256, width)
            width += 1
        writer.write(code, width)
        at = best_end
        emitted += 1
        if emitted % 2 == 0:
            node = trie
            for value in data[pair_start:at]:
                node = node.setdefault(value, {})
            code = 257 + emitted // 2 - 1
            if prefer_latest or None not in node:
                node[None] = code
            pair_start = at
    return struct.pack('<H', len(data)) + writer.finish()


def decode_payload(payload, flags):
    if flags & ~3:
        raise ValueError(f'Unknown compression flags 0x{flags:02X}')
    if flags & 2:
        payload = unpack_pair_span(payload)
    if flags & 1:
        payload = unpack_rle(payload)
    return payload


def encode_payload(payload, flags):
    if flags & ~3:
        raise ValueError(f'Unknown compression flags 0x{flags:02X}')
    if flags & 1:
        payload = pack_rle(payload)
    if flags & 2:
        payload = pack_pair_span(payload)
    return payload
