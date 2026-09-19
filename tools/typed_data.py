"""Small typed initialized-DATA components with symbolic 16-bit OMF references."""
import struct


FORMAT = 'typed-data-v1'


def compile_typed_data(document):
    if set(document) != {'format', 'owner', 'fields'} or document['format'] != FORMAT:
        raise ValueError('Invalid typed DATA document')
    if not isinstance(document['owner'], str) or not document['owner']:
        raise ValueError('Typed DATA owner is missing')
    data, refs = bytearray(), []
    seen = set()
    for field in document['fields']:
        if not isinstance(field, dict) or not isinstance(field.get('name'), str):
            raise ValueError('Invalid typed DATA field')
        name, kind = field['name'], field.get('type')
        if not name or name in seen:
            raise ValueError('Typed DATA field names must be unique')
        seen.add(name)
        if kind in ('pointer32', 'offset16'):
            if set(field) != {'name', 'type', 'target', 'addend'}:
                raise ValueError('Invalid symbolic DATA reference')
            target, addend = field['target'], field['addend']
            if not isinstance(target, str) or not target or type(addend) is not int or not 0 <= addend <= 0xffff:
                raise ValueError('Invalid symbolic DATA target or addend')
            refs.append({'offset': len(data), 'target': target, 'addend': addend, 'loc': kind})
            data.extend(struct.pack('<H', addend))
            if kind == 'pointer32':
                data.extend(b'\0\0')
            continue
        if set(field) != {'name', 'type', 'values'} or not isinstance(field['values'], list):
            raise ValueError('Invalid typed DATA value field')
        values = field['values']
        limits = {'u8': (0, 255, 'B'), 'i8': (-128, 127, 'b'),
                  'u16': (0, 65535, 'H'), 'i16': (-32768, 32767, 'h')}
        if kind not in limits:
            raise ValueError('Unsupported typed DATA field type')
        low, high, code = limits[kind]
        if any(type(value) is not int or not low <= value <= high for value in values):
            raise ValueError('Typed DATA value outside field range')
        data.extend(struct.pack('<' + code * len(values), *values))
    if not data:
        raise ValueError('Typed DATA document is empty')
    return bytes(data), refs, {document['owner']: 0}


def bind_typed_data(document, resolve):
    data, refs, _ = compile_typed_data(document)
    result = bytearray(data)
    for ref in refs:
        offset, segment = resolve(ref['target'])
        value = offset + ref['addend']
        if not 0 <= value <= 0xffff:
            raise ValueError('Bound typed DATA offset exceeds 16 bits')
        if ref['loc'] == 'pointer32':
            struct.pack_into('<HH', result, ref['offset'], value, segment)
        else:
            struct.pack_into('<H', result, ref['offset'], value)
    return bytes(result)
