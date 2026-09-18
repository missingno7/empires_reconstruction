"""Independent encoders for identified executable data components."""

DAC_FORMAT = 'dac6-rgb256-v1'
TEXT_FORMAT = 'ascii-nul-v1'


def palette_document(data):
    if len(data) != 768 or any(value > 63 for value in data):
        raise ValueError('Expected 256 RGB triples of six-bit DAC values')
    return {'format': DAC_FORMAT, 'entries': [list(data[i:i + 3]) for i in range(0, 768, 3)]}


def encode_data(document, encoder):
    if encoder == TEXT_FORMAT:
        if set(document) != {'format', 'text'} or document.get('format') != encoder or not isinstance(document['text'], str):
            raise ValueError('ASCII string source requires format and text only')
        if '\0' in document['text']:
            raise ValueError('ASCII string source cannot contain an embedded terminator')
        try:
            return document['text'].encode('ascii') + b'\0'
        except UnicodeEncodeError as error:
            raise ValueError('ASCII string source contains non-ASCII text') from error
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
    if encoder == TEXT_FORMAT:
        if not data.endswith(b'\0') or b'\0' in data[:-1]:
            raise ValueError('Expected one terminated ASCII string')
        try:
            return {'format': TEXT_FORMAT, 'text': data[:-1].decode('ascii')}
        except UnicodeDecodeError as error:
            raise ValueError('String contains non-ASCII bytes') from error
    if encoder != DAC_FORMAT:
        raise ValueError('Unknown executable data encoder')
    return palette_document(data)
