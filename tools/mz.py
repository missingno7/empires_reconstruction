"""All file/load-module coordinate conversions for the reconstruction."""
from dataclasses import dataclass
import struct


@dataclass
class MZ:
    header_size: int
    declared_size: int
    relocation_offset: int
    relocations: list
    cs: int
    ip: int
    ss: int
    sp: int
    minalloc: int
    maxalloc: int

    @classmethod
    def parse(cls, data):
        if len(data) < 28 or data[:2] != b'MZ':
            raise ValueError('Not a complete MZ header')
        _, last, pages, count, paragraphs, minimum, maximum, ss, sp, _, ip, cs, table, _ = struct.unpack_from('<14H', data)
        size = (pages - 1) * 512 + (last or 512)
        header = paragraphs * 16
        if not pages or last > 511 or not 28 <= header <= size <= len(data):
            raise ValueError('Invalid MZ header/load-image size')
        if table < 28 or table + count * 4 > header:
            raise ValueError('MZ relocation table outside header')
        relocations = []
        for i in range(count):
            offset, segment = struct.unpack_from('<HH', data, table + i * 4)
            target = cls.linear(segment, offset)
            if target + 2 > size - header:
                raise ValueError('MZ relocation target outside load image')
            relocations.append({'segment': segment, 'offset': offset, 'load_offset': target})
        return cls(header, size, table, relocations, cs, ip, ss, sp, minimum, maximum)

    @staticmethod
    def linear(segment, offset):
        return segment * 16 + offset

    def file_offset(self, load_offset):
        return self.header_size + load_offset

    def load_offset(self, file_offset):
        if not self.header_size <= file_offset < self.declared_size:
            raise ValueError('File offset is outside the DOS load image')
        return file_offset - self.header_size

    def load_image(self, data):
        return data[self.header_size:self.declared_size]

    def relocation_bytes(self, data):
        return data[self.relocation_offset:self.relocation_offset + 4 * len(self.relocations)]
