"""Independent encoders for identified executable data components."""

DAC_FORMAT = 'dac6-rgb256-v1'
TEXT_FORMAT = 'ascii-nul-v1'
ASCII_FORMAT = 'ascii-v1'
RECORDS_FORMAT = 'fixed-records-v1'
U16_TABLE_FORMAT = 'u16le-table-v1'
ZERO_PAD_FORMAT = 'zero-pad-v1'


def palette_document(data):
    if len(data) != 768 or any(value > 63 for value in data):
        raise ValueError('Expected 256 RGB triples of six-bit DAC values')
    return {'format': DAC_FORMAT, 'entries': [list(data[i:i + 3]) for i in range(0, 768, 3)]}


def encode_data(document, encoder, resolve_pointer=None):
    from typed_data import FORMAT as TYPED_FORMAT, bind_typed_data
    from sound_instruments import FORMAT as INSTRUMENT_FORMAT, compile_sound_instruments
    from sound_data import FORMAT as SOUND_FORMAT, bind_sound_data
    from pointer_records import FORMAT, bind_records
    if encoder == TYPED_FORMAT:
        if resolve_pointer is None:
            raise ValueError('Typed DATA requires a target resolver')
        return bind_typed_data(document, resolve_pointer)
    if encoder == INSTRUMENT_FORMAT:
        return compile_sound_instruments(document)
    if encoder == SOUND_FORMAT:
        if resolve_pointer is None:
            raise ValueError('Sound DATA requires a target resolver')
        return bind_sound_data(document, resolve_pointer)
    if encoder == FORMAT:
        if resolve_pointer is None:
            raise ValueError('Symbolic pointer records require a target resolver')
        return bind_records(document, resolve_pointer)
    if encoder == ASCII_FORMAT:
        if set(document) != {'format', 'text'} or document.get('format') != encoder or not isinstance(document['text'], str):
            raise ValueError('ASCII source requires format and text only')
        if '\0' in document['text']:
            raise ValueError('ASCII source cannot contain an embedded NUL')
        try:
            return document['text'].encode('ascii')
        except UnicodeEncodeError as error:
            raise ValueError('ASCII source contains non-ASCII text') from error
    if encoder == TEXT_FORMAT:
        if set(document) != {'format', 'text'} or document.get('format') != encoder or not isinstance(document['text'], str):
            raise ValueError('ASCII string source requires format and text only')
        if '\0' in document['text']:
            raise ValueError('ASCII string source cannot contain an embedded terminator')
        try:
            return document['text'].encode('ascii') + b'\0'
        except UnicodeEncodeError as error:
            raise ValueError('ASCII string source contains non-ASCII text') from error
    if encoder == RECORDS_FORMAT:
        if (set(document) != {'format', 'record_size', 'records'} or
                document.get('format') != encoder or type(document['record_size']) is not int or
                document['record_size'] < 1 or not isinstance(document['records'], list)):
            raise ValueError('Fixed-record source requires format, record_size and records')
        size = document['record_size']
        data = bytearray()
        for record in document['records']:
            if not isinstance(record, str) or len(record) != size * 2:
                raise ValueError('Fixed record has the wrong hex length')
            try:
                data.extend(bytes.fromhex(record))
            except ValueError as error:
                raise ValueError('Fixed record contains non-hex data') from error
        return bytes(data)
    if encoder == U16_TABLE_FORMAT:
        if (set(document) != {'format', 'values'} or document.get('format') != encoder or
                not isinstance(document['values'], list)):
            raise ValueError('u16 table source requires format and values only')
        data = bytearray()
        for value in document['values']:
            if type(value) is not int or not 0 <= value <= 65535:
                raise ValueError('u16 table value is out of range')
            data.extend(value.to_bytes(2, 'little'))
        return bytes(data)
    if encoder == ZERO_PAD_FORMAT:
        if (set(document) != {'format', 'length'} or document.get('format') != encoder or
                type(document['length']) is not int or document['length'] < 0):
            raise ValueError('Zero-padding source requires format and nonnegative length only')
        return bytes(document['length'])
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
    if encoder == ASCII_FORMAT:
        if b'\0' in data:
            raise ValueError('ASCII data contains an embedded NUL')
        try:
            return {'format': encoder, 'text': data.decode('ascii')}
        except UnicodeDecodeError as error:
            raise ValueError('ASCII data contains non-ASCII bytes') from error
    if encoder == TEXT_FORMAT:
        if not data.endswith(b'\0') or b'\0' in data[:-1]:
            raise ValueError('Expected one terminated ASCII string')
        try:
            return {'format': TEXT_FORMAT, 'text': data[:-1].decode('ascii')}
        except UnicodeDecodeError as error:
            raise ValueError('String contains non-ASCII bytes') from error
    if encoder == RECORDS_FORMAT:
        raise ValueError('Fixed-record decoding requires an explicit record size')
    if encoder == U16_TABLE_FORMAT:
        if len(data) % 2:
            raise ValueError('u16 table has an odd byte length')
        return {'format': encoder, 'values': [int.from_bytes(data[i:i + 2], 'little')
                                               for i in range(0, len(data), 2)]}
    if encoder == ZERO_PAD_FORMAT:
        if any(data):
            raise ValueError('Zero-padding data contains a nonzero byte')
        return {'format': encoder, 'length': len(data)}
    if encoder != DAC_FORMAT:
        raise ValueError('Unknown executable data encoder')
    return palette_document(data)
