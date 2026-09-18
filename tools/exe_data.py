"""Independent encoders for identified executable data components."""

DAC_FORMAT = 'dac6-rgb256-v1'


def palette_document(data):
    if len(data) != 768 or any(value > 63 for value in data):
        raise ValueError('Expected 256 RGB triples of six-bit DAC values')
    return {'format': DAC_FORMAT, 'entries': [list(data[i:i + 3]) for i in range(0, 768, 3)]}


def encode_data(document, encoder):
    if encoder != DAC_FORMAT or document.get('format') != encoder:
        raise ValueError('Unknown or inconsistent executable data encoder')
    if set(document) != {'format', 'entries'} or not isinstance(document['entries'], list) or len(document['entries']) != 256:
        raise ValueError('DAC source requires exactly 256 entries and no extra fields')
    data = bytearray()
    for rgb in document['entries']:
        if not isinstance(rgb, list) or len(rgb) != 3 or any(type(c) is not int or not 0 <= c <= 63 for c in rgb):
            raise ValueError('DAC entry must be three unsigned six-bit channel values')
        data.extend(rgb)
    return bytes(data)


def decode_data(data, encoder):
    if encoder != DAC_FORMAT:
        raise ValueError('Unknown executable data encoder')
    return palette_document(data)
