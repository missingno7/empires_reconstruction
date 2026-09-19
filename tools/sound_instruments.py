"""Lossless source format for OPL voice state and 56-byte instrument records."""
import struct


FORMAT = 'empires-sound-instruments-v1'


def decode_sound_instruments(data):
    if len(data) != 1923:
        raise ValueError('Sound instrument extent must be 1,923 bytes')
    records = data[0x47:-4]
    if len(records) != 33 * 0x38 or data[-4:] != b'\xff' * 4:
        raise ValueError('Sound instrument record extent or terminator differs')
    return {
        'format': FORMAT,
        # g2fd2 begins one byte before this historical ownership extent.
        'slot_glyph_pairs_tail': list(data[:0x11]),
        'voice_operator_offsets': list(data[0x11:0x23]),
        'voice_disabled': list(data[0x23:0x35]),
        'voice_connection': list(data[0x35:0x47]),
        'instrument_records': [list(struct.unpack('<28h', records[i:i + 0x38]))
                               for i in range(0, len(records), 0x38)],
        'terminator_words': [-1, -1],
    }


def compile_sound_instruments(document):
    required = {'format', 'slot_glyph_pairs_tail', 'voice_operator_offsets',
                'voice_disabled', 'voice_connection', 'instrument_records',
                'terminator_words'}
    if set(document) != required or document['format'] != FORMAT:
        raise ValueError('Invalid sound instrument document')
    data = bytearray()
    for key, length in (('slot_glyph_pairs_tail', 17),
                        ('voice_operator_offsets', 18),
                        ('voice_disabled', 18), ('voice_connection', 18)):
        values = document[key]
        if (not isinstance(values, list) or len(values) != length or
                any(type(v) is not int or not 0 <= v <= 255 for v in values)):
            raise ValueError(f'Invalid {key} byte array')
        data.extend(values)
    records = document['instrument_records']
    if not isinstance(records, list) or len(records) != 33:
        raise ValueError('Expected 33 sound instrument records')
    for record in records:
        if (not isinstance(record, list) or len(record) != 28 or
                any(type(v) is not int or not -32768 <= v <= 32767 for v in record)):
            raise ValueError('Invalid 28-word sound instrument record')
        data.extend(struct.pack('<28h', *record))
    terminator = document['terminator_words']
    if terminator != [-1, -1]:
        raise ValueError('Sound instrument terminator differs')
    data.extend(struct.pack('<2h', *terminator))
    if len(data) != 1923:
        raise ValueError('Sound instrument layout differs from verified extent')
    return bytes(data)
