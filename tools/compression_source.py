"""Lossless pair-span syntax, separate from historical match-selection policy."""
import struct
from resource_codecs import BitReader, BitWriter, unpack_pair_span

FORMAT = 'pair-span-instructions-v1'


def decode_source(encoded):
    data = unpack_pair_span(encoded)
    reader, width, produced = BitReader(encoded[2:]), 9, 0
    instructions, checkpoints, emitted = [], [0], 0
    while produced < len(data):
        code = reader.read(width)
        if code == 256:
            instructions.append(['widen', 1])
            width += 1
            continue
        if code < 256:
            if instructions and instructions[-1][0] == 'literal':
                instructions[-1][1] += 1
            else:
                instructions.append(['literal', 1])
            produced += 1
        else:
            index = code - 257
            instructions.append(['pair', index])
            produced += checkpoints[index + 1] - checkpoints[index]
        emitted += 1
        if emitted % 2 == 0:
            checkpoints.append(produced)
    return data, {'format': FORMAT, 'instructions': instructions}


def encode_source(data, document):
    if (not isinstance(document, dict) or set(document) != {'format', 'instructions'}
            or document['format'] != FORMAT or not isinstance(document['instructions'], list)):
        raise ValueError('Invalid pair-span source document')
    if len(data) > 65535:
        raise ValueError('Pair-span input exceeds 16-bit size header')
    writer, width, at, emitted, checkpoints = BitWriter(), 9, 0, 0, [0]
    for instruction in document['instructions']:
        if (not isinstance(instruction, list) or len(instruction) != 2
                or type(instruction[1]) is not int):
            raise ValueError('Malformed compression instruction')
        op, value = instruction
        if at == len(data):
            raise ValueError('Compression instruction after output completion')
        if op == 'widen':
            if value != 1 or width == 16:
                raise ValueError('Invalid width escape')
            writer.write(256, width)
            width += 1
            continue
        if op == 'literal':
            if value <= 0 or at + value > len(data):
                raise ValueError('Literal run outside source')
            for byte in data[at:at + value]:
                writer.write(byte, width)
                at += 1
                emitted += 1
                if emitted % 2 == 0:
                    checkpoints.append(at)
        elif op == 'pair':
            if value < 0 or value + 1 >= len(checkpoints):
                raise ValueError('Pair reference is not yet defined')
            previous = data[checkpoints[value]:checkpoints[value + 1]]
            if data[at:at + len(previous)] != previous:
                raise ValueError('Pair reference differs from decoded source')
            writer.write(257 + value, width)
            at += len(previous)
            emitted += 1
            if emitted % 2 == 0:
                checkpoints.append(at)
        else:
            raise ValueError('Unknown compression instruction')
    if at != len(data):
        raise ValueError('Compression instructions do not cover source')
    return struct.pack('<H', len(data)) + writer.finish()
