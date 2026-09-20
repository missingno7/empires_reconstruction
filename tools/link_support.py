"""Pure linker diagnostics and verified library contribution helpers."""
import re
import struct
from omf import OmfReader

def link_errors(log, map_text=''):
    """TLINK can write fixup failures only to its detailed map."""
    return list(dict.fromkeys(line.strip() for line in (log + '\n' + map_text).splitlines()
                             if re.search(r'error:|undefined symbol|bad object file|fatal|fixup overflow',
                                          line, re.I)))


def _omf_records(data):
    """Yield ``(kind, start, end)`` for one OMF module or object."""
    at = 0
    while at < len(data):
        if at + 3 > len(data):
            raise ValueError('truncated OMF record')
        length = struct.unpack_from('<H', data, at + 1)[0]
        end = at + 3 + length
        if end > len(data) or length < 1:
            raise ValueError('invalid OMF record length')
        yield data[at], at, end
        at = end


def replace_library_module_segments(library, module_name, replacement_object):
    """Preserve library OMF record topology while replacing verified segment bytes.

    Hosts may choose different LEDATA chunk boundaries. The pinned library's
    record layout is retained; only its initialized payload is replaced.
    """
    page = struct.unpack_from('<H', library, 1)[0] + 3; module_start = page
    while module_start < len(library) and library[module_start] == OmfReader.THEADR:
        at, name = module_start, ''
        while at < len(library):
            kind, length = library[at], struct.unpack_from('<H', library, at + 1)[0]
            if kind == OmfReader.THEADR: name = library[at + 4:at + 4 + library[at + 3]].decode('latin1')
            at += 3 + length
            if kind in (OmfReader.MODEND16, OmfReader.MODEND32): break
        module_end = at
        if name == module_name:
            original = library[module_start:module_end]
            source_module, original_module = OmfReader().read(replacement_object, module_name), OmfReader().read(original, module_name)
            if source_module.segments != original_module.segments or source_module.segment_lengths != original_module.segment_lengths:
                raise ValueError(f'{module_name}: source segment topology differs')
            # Existing library LEDATA payloads are patch locations; their segment
            # indexes and offsets remain untouched, including record boundaries.
            payloads = {segment: source_module.segment_bytes(segment) for segment in source_module.segments}
            patched = bytearray(original)
            for kind, begin, finish in _omf_records(original):
                if kind != OmfReader.LEDATA16: continue
                body = original[begin + 3:finish - 1]; seg, offset = body[0], struct.unpack_from('<H', body, 1)[0]
                segment = original_module.segment_defs[seg - 1]['name']; length = len(body) - 3
                replacement = payloads[segment][offset:offset + length]
                if len(replacement) != length: raise ValueError(f'{module_name}: LEDATA lies outside source segment')
                patched[begin + 6:finish - 1] = replacement
                patched[finish - 1] = (-sum(patched[begin:finish - 1])) & 255
            patched_module = bytes(patched); parsed = OmfReader().read(patched_module, module_name)
            for segment in source_module.segments:
                if parsed.segment_bytes(segment) != source_module.segment_bytes(segment):
                    raise ValueError(f'{module_name}: source/library {segment} replacement failed')
            return library[:module_start] + patched_module + library[module_end:]
        module_start = ((module_end + page - 1) // page) * page
    raise ValueError(f'{module_name}: module not found in library')


def replace_library_member(library, module_name, replacement_object):
    """Package an intact compiler object; update archive offsets, never FIXUPP.

    Dictionary structure: TIS OMF 1.1 library format, 512-byte blocks,
    37 bucket offsets in two-byte units and little-endian module page numbers.
    https://www.os2site.com/sw/dev/openwatcom/docs/omf.pdf
    Existing names/hash buckets are retained because public identity is fixed.
    """
    page = struct.unpack_from('<H', library, 1)[0] + 3
    dictionary_start, blocks = struct.unpack_from('<IH', library, 3)
    modules = OmfReader().split_library(library)
    original = next((b for n,b in modules if n == module_name), None)
    if original is None:
        raise ValueError('Library member is absent')
    before = OmfReader().read(original)
    after = OmfReader().read(replacement_object)
    if before.publics != after.publics:
        raise ValueError('Native library member public identity differs')
    output = bytearray(library[:page])
    remap = {}
    old_offset = page
    for name, blob in modules:
        remap[old_offset // page] = len(output) // page
        output.extend(replacement_object if name == module_name else blob)
        output.extend(bytes((-len(output)) % page))
        old_offset += ((len(blob) + page - 1) // page) * page
    trailer_size = (-len(output)) % 512
    if trailer_size < 3:
        trailer_size += 512
    output.extend(b'\xf1' + struct.pack('<H', trailer_size - 3) + bytes(trailer_size - 3))
    new_dictionary_start = len(output)
    dictionary = bytearray(library[dictionary_start:dictionary_start + blocks * 512])
    if len(dictionary) != blocks * 512:
        raise ValueError('Truncated library dictionary')
    for base in range(0, len(dictionary), 512):
        seen = set()
        for bucket in dictionary[base:base+37]:
            if not bucket or bucket in seen:
                continue
            seen.add(bucket)
            offset = base + bucket * 2
            if offset < base + 38 or offset >= base + 512:
                raise ValueError('Invalid dictionary bucket')
            number_at = offset + 1 + dictionary[offset]
            if number_at + 2 > base + 512:
                raise ValueError('Invalid dictionary entry')
            old_page = struct.unpack_from('<H', dictionary, number_at)[0]
            if old_page not in remap:
                raise ValueError('Dictionary refers to unknown member page')
            struct.pack_into('<H', dictionary, number_at, remap[old_page])
    output.extend(dictionary)
    # Preserve auxiliary trailing records; no member object bytes are edited.
    output.extend(library[dictionary_start + blocks * 512:])
    struct.pack_into('<I', output, 3, new_dictionary_start)
    repacked = OmfReader().split_library(bytes(output))
    if [b for _,b in repacked] != [replacement_object if n == module_name else b for n,b in modules]:
        raise ValueError('Library member bytes changed during packaging')
    return bytes(output)
