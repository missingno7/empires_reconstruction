"""Emit typed source documents for the final eight small raw EXE extents."""
from pathlib import Path
import struct

from reconstruct import read_json, write_json
from typed_data import FORMAT, bind_typed_data

ROOT = Path(__file__).resolve().parents[1]


def field(name, kind, values):
    return {'name': name, 'type': kind, 'values': values}


def pointer(name, target, addend=0):
    return {'name': name, 'type': 'pointer32', 'target': target, 'addend': addend}


def document(owner, fields):
    return {'format': FORMAT, 'owner': owner, 'fields': fields}


def descriptor_document(data, pointers):
    fields, cursor, literal = [], 0, 0
    for offset, target, addend in pointers:
        chunk = data[cursor:offset]
        if chunk:
            if len(chunk) % 2:
                raise ValueError('Descriptor literal chunk is not word aligned')
            fields.append(field(f'descriptor_words_{literal:02d}', 'u16',
                                list(struct.unpack('<' + 'H' * (len(chunk) // 2), chunk))))
            literal += 1
        fields.append(pointer(f'descriptor_pointer_{offset:03d}', target, addend))
        cursor = offset + 4
    if cursor != len(data) - 1:
        raise ValueError('Descriptor trailing layout differs')
    fields.append(field('terminal_control', 'u8', [data[-1]]))
    return document('DATA_010924_MENU_DESCRIPTORS', fields)


def documents():
    raw = lambda name: (ROOT / 'raw' / name).read_bytes()
    menu = raw('010924-0109AF.bin')
    menu_pointers = [
        (6, 'DATA_0107EA_HELP_TITLE', 0), (14, 'DATA_0107F4_HELP_TOPICS', 0),
        (18, 'DATA_010848_FILE_F2_TABLE', 0), (26, 'DATA_010848_FILE_F2_TEXT', 0),
        (34, 'DATA_01085A_MENU_TOPICS', 0), (38, 'DATA_0108A7_MENU_TABLE_A', 0),
        (46, 'DATA_0108A7_MENU_OPTIONS', 0), (54, 'DATA_0108FE_OPTION_TOPICS', 0),
        (58, 'DATA_010924_MENU_DESCRIPTORS', 0),
        (68, 'DATA_010924_MENU_DESCRIPTORS', 6),
        (72, 'DATA_0107EA_HELP_TITLE', 0), (80, 'DATA_0107F4_HELP_TOPICS', 0),
        (84, 'DATA_010848_FILE_F2_TABLE', 0), (92, 'DATA_010848_FILE_F2_TEXT', 0),
        (100, 'DATA_0108A7_MENU_HALL', 0), (104, 'DATA_0108A7_MENU_TABLE_B', 0),
        (112, 'DATA_0108A7_MENU_OPTIONS', 0), (120, 'DATA_0108FE_OPTION_TOPICS', 0),
        (124, 'DATA_010924_MENU_DESCRIPTORS', 0),
        (134, 'DATA_010924_MENU_DESCRIPTORS', 72),
    ]
    return {
        'DATA_01075A_FILE_ERROR_CONTROL': document('DATA_01075A_FILE_ERROR_CONTROL', [
            field('enabled', 'u16', [1]), field('control_bytes', 'u8', [0, 0, 0, 0, 2]),
            pointer('message', 'DATA_01068E_PROGRAM_DISK'),
            field('terminator', 'u8', [0] + [255] * 8)]),
        'DATA_010924_MENU_DESCRIPTORS': descriptor_document(menu, menu_pointers),
        'DATA_0109BB_CONTROL_CODES': document('DATA_0109BB_CONTROL_CODES', [
            field('codes', 'u8', [0x17, 0x18])]),
        'DATA_01129F_LEVEL_CONTROL': document('DATA_01129F_LEVEL_CONTROL', [
            field('header', 'u8', [0, 1, 0, 0, 0, 0, 0, 2]),
            pointer('level_complete_text', 'DATA_0112D6_LEVEL_COMPLETE'),
            field('sentinel_prefix', 'u8', [0] + [255] * 8),
            field('control_words', 'i16', [4, 6, 8, 10, 6, 4, 8, 10, 4, 8, 10, 6, -1, 6, 8, 10, -1])]),
        'DATA_011F25_KEYBOARD_CONTROL': document('DATA_011F25_KEYBOARD_CONTROL', [
            pointer('runtime_buffer', 'GAME_BSS', 36044), field('mode', 'u8', [0]),
            field('control_words', 'u16', [11, 14, 285, 11])]),
        'DATA_011F86_USER_CONTROL': document('DATA_011F86_USER_CONTROL', [
            field('enabled', 'u16', [1]), pointer('message', 'DATA_011F9A_NEW_USER_MESSAGE'),
            field('mode', 'u16', [2]), field('state_words', 'i16', [0, 0, -1, -1, -1, -1])]),
        'DATA_011FAE_CACHED_INDEX': document('DATA_011FAE_CACHED_INDEX', [
            field('cached_index', 'i16', [-1])]),
        'DATA_01352C_RUNTIME_LIMITS': document('DATA_01352C_RUNTIME_LIMITS', [
            field('limits', 'u16', [404, 404, 411])]),
    }


def run():
    for owner, source in documents().items():
        path = ROOT / 'src/data' / (owner + '.json')
        write_json(path, source)
        print(f'Wrote {path}')


if __name__ == '__main__':
    run()
