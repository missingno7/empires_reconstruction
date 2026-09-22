"""tools/portable/sound_oracle/emu.py -- Unicorn-based differential oracle
harness for asm/SOUND.ASM (module M_C1A0_CB48, layout/production-plan.json).

Executes the REAL 8086 code bytes of asm/SOUND.ASM straight out of
assets/AEPROG.EXE under Unicorn (UC_ARCH_X86 / UC_MODE_16), so the oracle's
"expected" output is the actual historical binary, not a re-implementation
of it.  See docs/portable/architecture.md ("Audio model", "Testing") and
docs/current/sound-state.md for the state-machine background this module
assumes.

---------------------------------------------------------------------------
Segment/offset facts (all cross-checked against layout/manifest.json and
layout/production-plan.json; re-derive from those files, not from this
comment, if the historical tree ever changes)
---------------------------------------------------------------------------

* assets/AEPROG.EXE is a small-model DOS .EXE: one code segment (_TEXT,
  manifest frame 0) and one data segment (DGROUP, manifest frame 0xFA30 ==
  64048).  The MZ header + relocations occupy file offset [0, 0x200); _TEXT
  starts at file offset 0x200 (manifest['frames']['_TEXT'] + 0x200) and its
  bytes run CS-relative from CS:0000.  DGROUP DATA starts at file offset
  manifest['frames']['DGROUP'] + 0x200 == 0x1FC30 and is DATA_LEN==0x3902
  (14594) bytes; BSS (uninitialized, zero at load) follows immediately at
  DS:0x3902 for BSS_LEN==37250 bytes (tools/portable/datagen.py).  Total
  DGROUP window: DS:0000..DS:0xCA84 (51844 bytes) -- comfortably inside one
  64KiB real-mode segment, so DS==SS==ES throughout and the stack lives
  above BSS in the same segment, exactly like the historical small-model
  layout.

* asm/SOUND.ASM (module M_C1A0_CB48) occupies file offset [50080, 52572),
  i.e. CS-relative [0xC1A0, 0xCD5C) (50080 - 0x200 == 0xC1A0, matching the
  task's given historical CS offset).  Every public routine's CS offset is
  0xC1A0 + <module-relative 'offset'> from layout/production-plan.json's
  module build.publics list (PUBLIC_OFFSETS below).

* Unicorn UC_MODE_16 quirk (verified empirically, see the two throwaway
  probes this file's author ran before writing it): uc.emu_start()'s
  begin/until arguments are CS-relative IP offsets, NOT physical addresses
  -- the CPU computes the physical fetch address as CS_base*16 + IP
  internally from the CS register already programmed via reg_write.  So
  run_routine() below writes code bytes at the segment's physical base
  (seg*16) but calls emu_start(ip, until_ip).

* The sound-resource block (what snd_seg:snd_base pointed at historically)
  and the voice/staging block (snd_seg2:snd_base2) are heap blocks with no
  fixed historical address; this oracle owns the allocation and always
  places each at offset 0 of its own fresh segment (matching a paragraph-
  aligned DOS allocator handing back a whole segment for a block this
  size) and writes snd_base=snd_base2=0 accordingly -- see SoundMachine.
  This is a documented modeling choice, not a historical fact: nothing in
  asm/SOUND.ASM computes an address any differently for offset 0 vs any
  other in-segment offset, since every access is ES:DI-relative.
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

from unicorn import Uc, UC_ARCH_X86, UC_MODE_16, UC_HOOK_INSN, UcError
from unicorn.x86_const import (
    UC_X86_INS_IN, UC_X86_INS_OUT,
    UC_X86_REG_CS, UC_X86_REG_DS, UC_X86_REG_ES, UC_X86_REG_SS,
    UC_X86_REG_IP, UC_X86_REG_SP, UC_X86_REG_BP,
    UC_X86_REG_AX, UC_X86_REG_AL, UC_X86_REG_AH,
)

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT / 'tools' / 'portable'))

import datagen  # noqa: E402  (tools/portable/datagen.py -- DATA image builder)

EXE_PATH = ROOT / 'assets' / 'AEPROG.EXE'
MANIFEST_PATH = ROOT / 'layout' / 'manifest.json'
PRODUCTION_PLAN_PATH = ROOT / 'layout' / 'production-plan.json'

# ---------------------------------------------------------------------------
# Layout constants (see header comment)
# ---------------------------------------------------------------------------

FILE_TEXT_BASE = 0x200          # _TEXT byte 0 == this AEPROG.EXE file offset
DATA_LEN = datagen.DATA_LEN     # 0x3902
BSS_LEN = datagen.BSS_LEN       # 37250
DGROUP_LEN = datagen.DGROUP_LEN  # 0xCA84
TEXT_LEN_PARAS = 0xFA3          # _TEXT length in paragraphs (64048 / 16, exact)
LOAD_IMAGE_LEN = 64048 + DATA_LEN  # 78642 -- _TEXT+DATA, contiguous in the file

SOUND_MODULE_ID = 'M_C1A0_CB48'
SOUND_MODULE_CS_BASE = 0xC1A0   # historical CS offset of asm/SOUND.ASM's first public

# Real-mode segment values (paragraphs).  Physical = seg*16 + off.
#
# CS_SEG doubles as the DOS "load segment" for relocation purposes (see
# SoundMachine._apply_relocations()): manifest['frames']['_TEXT']==0 means
# _TEXT starts at the very first byte of the load image, i.e. CS == load
# segment + 0. DGROUP's manifest frame (64048 bytes == 0xFA3 paragraphs,
# exact) is DGROUP-relative-to-load-segment, so DS_SEG MUST be exactly
# CS_SEG + 0xFA3 for the standard DOS relocation formula
# (`patched_word = raw_word_in_file + load_segment`) to land pointers in
# the right place -- picking DS_SEG independently (as an early version of
# this file did) breaks every far call/pointer the compiler emitted
# in-segment (discovered via a real crash: backend mode 2's
# sound_backend_select_init() -> opl_init() makes a `lcall 0, 0xF2A7`
# whose segment half is a to-be-relocated placeholder). Because of this,
# CS and DS together cover one CONTIGUOUS physical range
# [CS_SEG*16, DS_SEG*16 + 0x10000) that is mapped as a single region.
CS_SEG = 0x1000
DS_SEG = CS_SEG + TEXT_LEN_PARAS    # == SS == ES: DGROUP (0x1FA3)
_LOAD_REGION_PHYS = CS_SEG * 16
_LOAD_REGION_LEN = (DS_SEG - CS_SEG) * 16 + 0x10000  # covers all of DS_SEG's own 64KiB too
_LOAD_REGION_MAPPED_LEN = ((_LOAD_REGION_LEN + 0xFFF) // 0x1000) * 0x1000  # page-rounded

# Synthetic allocations (not part of the historical image, so any
# non-overlapping segment works): placed just past the combined,
# page-rounded CS/DS mapping so their own 64KiB windows never overlap it.
RES_SEG = (_LOAD_REGION_PHYS + _LOAD_REGION_MAPPED_LEN) // 16       # sound_resource_block, offset 0
VOICE_SEG = RES_SEG + 0x1000  # sound_voice_block (staging record), offset 0

STACK_TOP = 0xFFF0        # SP starts here (grows down), inside DS_SEG, above BSS
# near-call return address run_routine() stops at: a HLT byte is planted
# here (see SoundMachine.__init__) so emulation stops the instant a `ret`
# lands on it, deterministically -- relying on emu_start()'s own `until`
# IP comparison proved unreliable once IP itself reaches this high in a
# 16-bit segment (empirically: Unicorn kept fetching past IP==until and
# eventually walked off the end of the 64KiB CS mapping into the next
# segment -- HLT sidesteps the whole question).
RETURN_SENTINEL_IP = 0xFFFE


def _load_production_plan_publics():
    import json
    plan = json.loads(PRODUCTION_PLAN_PATH.read_text())
    for module in plan['modules']:
        if module['id'] == SOUND_MODULE_ID:
            build = module['build']
            publics = module['publics']
            assert build['public'] == '_sound_tick_entry' and publics[0]['offset'] == 0
            offsets = {}
            for pub in publics:
                # publics carry a leading underscore (OMF/C name-mangling
                # convention); expose both spellings for convenience.
                name = pub['symbol'].lstrip('_')
                offsets[name] = pub['offset']
            return offsets, module['start'], module['end']
    raise KeyError(f'{SOUND_MODULE_ID} not found in {PRODUCTION_PLAN_PATH}')


PUBLIC_OFFSETS, _MODULE_FILE_START, _MODULE_FILE_END = _load_production_plan_publics()
assert _MODULE_FILE_START - FILE_TEXT_BASE == SOUND_MODULE_CS_BASE, (
    f'module file start {_MODULE_FILE_START:#x} - 0x200 != {SOUND_MODULE_CS_BASE:#x}; '
    'layout/production-plan.json changed underneath this file -- re-derive the constants.')


def public_cs_offset(name: str) -> int:
    """CS-relative entry offset of a public asm/SOUND.ASM routine, e.g.
    'sound_tick_entry' -> 0xC1A0, 'stream_control_block_arm' -> 0xCAF1."""
    return SOUND_MODULE_CS_BASE + PUBLIC_OFFSETS[name]


# ---------------------------------------------------------------------------
# Sound-region field table (docs/portable/state-map.md field map).
#
# Each entry places one historically-named DGROUP object (or, for
# voice_retune_base_table, one driver-owned static -- see
# portable/audio/sound_driver.c) at (abs_offset - 0x175E) inside a 0x73A
# (1850)-byte "historical-layout byte image" buffer: bytes [0, 0x738) mirror
# DS:175E..DS:1E96 verbatim (with two intentionally-blank ranges, see
# below), and the last 2 bytes mirror DS:237C (sound_request_count).
#
# Two ranges are left ZERO on both sides of the comparison instead of
# copied: sound_dispatch_182C (DS:182C, 4 bytes) and sound_dispatch_1832
# (DS:1832, 72 bytes) are compile-time CONSTANT pointer-table arrays --
# portable/generated/game_data.c represents them as real C pointers (void*)
# into the decoded lookup_XXXX blobs, which is not byte-comparable with the
# historical raw DS-offset words asm/SOUND.ASM itself reads (see
# sound_driver_internal.h's header comment).  The mechanism they select is
# still exercised and compared indirectly, through its observable effect
# (OPL nibble-port write bytes) rather than through this raw byte capture.
# ---------------------------------------------------------------------------

SOUND_REGION_BASE = 0x175E
SOUND_REGION_SPAN = 0x1E96 - 0x175E  # 0x738 == 1848
SOUND_REQUEST_COUNT_ABS = 0x237C
SOUND_SNAPSHOT_SIZE = SOUND_REGION_SPAN + 2  # 1850, matches sound.h's SOUND_DRIVER_SNAPSHOT_SIZE

# (name, absolute DS offset, byte length)
SOUND_FIELD_TABLE = [
    ('snd_base', 0x175E, 2),
    ('snd_seg', 0x1760, 2),
    ('snd_base2', 0x1762, 2),
    ('snd_seg2', 0x1764, 2),
    ('voice_retune_base_table', 0x1766, 8),   # driver-owned static, no data public
    ('sound_enabled', 0x176E, 2),
    ('snd_on', 0x1770, 2),
    ('music_enabled', 0x1772, 2),
    ('mus_flag', 0x1774, 2),
    ('snd_flag2', 0x1776, 2),
    ('snd_backend_mode', 0x1778, 2),
    ('snd_nvoices', 0x177A, 2),
    ('v_b', 0x177C, 8),
    ('snd_mode', 0x1784, 2),
    ('snd_hi', 0x1786, 2),
    ('g1788', 0x1788, 2),
    ('g178a', 0x178A, 2),
    ('voice_stream_cursor_table', 0x178C, 8),
    ('voice_stream_base_table', 0x1794, 8),
    ('v_ctr', 0x179C, 8),
    ('g17a4', 0x17A4, 8),
    ('v_a', 0x17AC, 8),
    ('v_hold', 0x17B4, 8),
    ('v_len', 0x17BC, 8),
    ('sound_region_17C4', 0x17C4, 40),
    ('voice_rest_table', 0x17EC, 8),
    ('g17f4', 0x17F4, 8),
    ('notetab', 0x17FC, 24),               # constant table, included for sanity
    ('note_divisors_octave', 0x1814, 24),  # constant table, included for sanity
    # 0x182C..0x1834 (sound_dispatch_182C) -- SKIPPED, see header comment.
    ('opl_port', 0x1830, 2),
    # 0x1832..0x187A (sound_dispatch_1832) -- SKIPPED, see header comment.
    # 0x187A..0x1E84 -- unrelated other-subsystem DGROUP state, left zero.
    ('g1e84', 0x1E84, 2),
    ('g1e86', 0x1E86, 2),
    ('mus_ptr', 0x1E88, 2),
    ('mus_arg', 0x1E8A, 2),
    ('g1e8c', 0x1E8C, 2),
    ('snd_len', 0x1E8E, 2),
    ('snd_delay', 0x1E90, 2),
    ('stream_note_delay', 0x1E92, 2),
    ('snd_one', 0x1E94, 2),
]
for _n, _o, _l in SOUND_FIELD_TABLE:
    assert SOUND_REGION_BASE <= _o and _o + _l <= SOUND_REGION_BASE + SOUND_REGION_SPAN, _n


def region_snapshot_from_dgroup(dgroup: bytes) -> bytes:
    """Build the 1850-byte historical-layout snapshot from a full DGROUP
    image (as read back from the emulator) using SOUND_FIELD_TABLE -- the
    Python-side twin of sound_driver_snapshot() (portable/audio/
    sound_driver.c)."""
    buf = bytearray(SOUND_SNAPSHOT_SIZE)
    for _name, abs_off, length in SOUND_FIELD_TABLE:
        rel = abs_off - SOUND_REGION_BASE
        buf[rel:rel + length] = dgroup[abs_off:abs_off + length]
    buf[SOUND_REGION_SPAN:SOUND_REGION_SPAN + 2] = dgroup[
        SOUND_REQUEST_COUNT_ABS:SOUND_REQUEST_COUNT_ABS + 2]
    return bytes(buf)


# ---------------------------------------------------------------------------
# Event normalisation -- mirrors sound.h's enum sound_event_kind exactly
# (0=OPL write, 1=PIT divisor, 2=speaker gate, 3=nibble write) so the two
# event logs (this oracle's and sound_event_log_*'s) compare directly.
# ---------------------------------------------------------------------------

EV_OPL_WRITE = 0
EV_PIT_DIVISOR = 1
EV_SPEAKER_GATE = 2
EV_NIBBLE_WRITE = 3

_KIND_NAMES = {0: 'OPL_WRITE', 1: 'PIT_DIVISOR', 2: 'SPEAKER_GATE', 3: 'NIBBLE_WRITE'}


class SoundMachine:
    """One disposable Unicorn machine running the real asm/SOUND.ASM code.

    Construct fresh per scenario (state is cheap: one 64KB code map + one
    64KB DGROUP map + two small resource maps); do not reuse across
    scenarios that need independent starting DGROUP state.
    """

    def __init__(self):
        exe_bytes = EXE_PATH.read_bytes()
        load_image = exe_bytes[FILE_TEXT_BASE:FILE_TEXT_BASE + LOAD_IMAGE_LEN]
        assert len(load_image) == LOAD_IMAGE_LEN

        self.uc = Uc(UC_ARCH_X86, UC_MODE_16)
        # One combined mapping for CS+DS (see the CS_SEG/DS_SEG comment
        # above: DS_SEG's own 64KiB window physically overlaps the tail of
        # a naive "map CS's 64KiB, map DS's 64KiB" scheme, exactly like
        # real DOS small-model memory -- so this is ONE region, not two).
        self.uc.mem_map(_LOAD_REGION_PHYS, _LOAD_REGION_MAPPED_LEN)
        self.uc.mem_map(RES_SEG * 16, 0x10000)
        self.uc.mem_map(VOICE_SEG * 16, 0x10000)

        # _TEXT+DATA load straight from the file, byte-for-byte (this is a
        # REAL-code oracle: no reconstructed/regenerated bytes anywhere in
        # the executed image). BSS (the rest of _LOAD_REGION_LEN) is
        # already zero from mem_map, matching a fresh DOS load.
        self.uc.mem_write(_LOAD_REGION_PHYS, load_image)
        self._apply_relocations()

        self.uc.mem_write(CS_SEG * 16 + RETURN_SENTINEL_IP, b'\xf4')  # HLT

        # Stub record_panel_rebuild() (F_D8F0, called from
        # sound_voice_table_reload()'s backend-mode-2 branch, src/RESCACHE.C
        # module cluster) with a bare `ret`: it is the UI record-panel
        # renderer (reads/draws through gc5da/the video framebuffer), not
        # part of the sound state machine, and this oracle maps neither the
        # video VRAM nor the other UI resource blocks it touches. Confirmed
        # safe to stub: its call site (`push es / push di / call 0xd8f0 /
        # pop di / pop es`) only saves/restores ES:DI around the call --
        # record_panel_rebuild() takes no stack arguments (its own prologue
        # is a plain `push bp / mov bp,sp`, C void-argument shape) -- so a
        # bare `ret` leaves the stack exactly as a real call/return would.
        RECORD_PANEL_REBUILD_CS_OFFSET = 0xD8F0
        self.uc.mem_write(CS_SEG * 16 + RECORD_PANEL_REBUILD_CS_OFFSET, b'\xc3')  # ret

        self.uc.reg_write(UC_X86_REG_CS, CS_SEG)
        self.uc.reg_write(UC_X86_REG_DS, DS_SEG)
        self.uc.reg_write(UC_X86_REG_ES, DS_SEG)
        self.uc.reg_write(UC_X86_REG_SS, DS_SEG)
        self.uc.reg_write(UC_X86_REG_SP, STACK_TOP)

        self.events = []       # list of {'tick': int, 'kind': int, 'a': int, 'b': int}
        self._tick = 0
        self._port_shadow = {}   # last OUT value per port, for the IN hook
        self._pit_pending = None  # lo byte awaiting its hi-byte OUT 0x42 partner
        self._opl_pending = None  # (reg,) awaiting its data-byte OUT partner

        self.uc.hook_add(UC_HOOK_INSN, self._hook_out, None, 1, 0, UC_X86_INS_OUT)
        self.uc.hook_add(UC_HOOK_INSN, self._hook_in, None, 1, 0, UC_X86_INS_IN)

    def _apply_relocations(self):
        """Standard DOS EXE relocation: every entry in layout/mz-header.json
        ['relocations'] names a 2-byte segment-word location (as
        entry.segment*16 + entry.offset, relative to the load image's own
        byte 0) whose value the real loader patches to
        `raw_word_in_file + load_segment`. Skipping this breaks every far
        call/pointer the compiler emitted in-image (discovered via a real
        crash -- see the CS_SEG/DS_SEG comment above); CS_SEG doubles as
        our load_segment since manifest['frames']['_TEXT']==0."""
        import json
        relocs = json.loads(MANIFEST_PATH.with_name('mz-header.json').read_text())['relocations']
        for entry in relocs:
            phys = _LOAD_REGION_PHYS + entry['segment'] * 16 + entry['offset']
            raw = struct.unpack('<H', self.uc.mem_read(phys, 2))[0]
            self.uc.mem_write(phys, struct.pack('<H', (raw + CS_SEG) & 0xFFFF))

    # -- raw DGROUP accessors (task requirement 1: read_dgroup()/write_dgroup()) --

    def read_dgroup(self) -> bytes:
        return bytes(self.uc.mem_read(DS_SEG * 16, DGROUP_LEN))

    def write_dgroup(self, offset: int, data: bytes) -> None:
        self.uc.mem_write(DS_SEG * 16 + offset, bytes(data))

    def read_word(self, ds_offset: int) -> int:
        return struct.unpack_from('<H', self.uc.mem_read(DS_SEG * 16 + ds_offset, 2))[0]

    def write_word(self, ds_offset: int, value: int) -> None:
        self.uc.mem_write(DS_SEG * 16 + ds_offset, struct.pack('<H', value & 0xFFFF))

    def region_snapshot(self) -> bytes:
        return region_snapshot_from_dgroup(self.read_dgroup())

    # -- resource/voice block setup (task requirement 2) --

    def load_resource_block(self, data: bytes) -> None:
        """Place the decoded AE000 record-0x41 sound-resource bytes at
        RES_SEG:0 and publish snd_seg/snd_base (DS:1760/175E)."""
        if len(data) > 0x10000:
            raise ValueError('resource block too large for one segment')
        self.uc.mem_write(RES_SEG * 16, data)
        self.write_word(0x175E, 0)         # snd_base
        self.write_word(0x1760, RES_SEG)   # snd_seg

    def load_voice_block(self, data: bytes, block_size: int = 0x620) -> None:
        """Place the decoded staging-record bytes at VOICE_SEG:0, zero-padded
        to block_size (0x620, matching gc5da's real farmalloc size), and
        publish snd_seg2/snd_base2 (DS:1764/1762)."""
        if len(data) > block_size:
            raise ValueError('voice/staging block bytes exceed block_size')
        buf = bytes(data) + b'\xff' * (block_size - len(data))
        self.uc.mem_write(VOICE_SEG * 16, buf)
        self.write_word(0x1762, 0)          # snd_base2
        self.write_word(0x1764, VOICE_SEG)  # snd_seg2

    # -- calling convention: near cdecl, args right-to-left, caller cleans --

    def run_routine(self, name: str, args=(), max_count: int = 2_000_000) -> int:
        """Call a public asm/SOUND.ASM routine by name with 16-bit word
        args, run to completion, return AX."""
        entry_ip = public_cs_offset(name)
        sp = self.uc.reg_read(UC_X86_REG_SP)
        for value in reversed(args):
            sp -= 2
            self.uc.mem_write(DS_SEG * 16 + sp, struct.pack('<H', value & 0xFFFF))
        sp -= 2
        self.uc.mem_write(DS_SEG * 16 + sp, struct.pack('<H', RETURN_SENTINEL_IP))
        self.uc.reg_write(UC_X86_REG_SP, sp)

        try:
            # until=0x10000: past every reachable 16-bit IP, so only the
            # HLT byte at RETURN_SENTINEL_IP (or the instruction-count
            # safety cap) can end this call -- see RETURN_SENTINEL_IP's
            # comment for why relying on `until` IP-equality was unsafe.
            self.uc.emu_start(entry_ip, 0x10000, timeout=0, count=max_count)
        except UcError as exc:
            raise RuntimeError(
                f'emulation fault calling {name}{tuple(args)}: {exc} '
                f'(IP={self.uc.reg_read(UC_X86_REG_IP):#06x})') from exc

        final_ip = self.uc.reg_read(UC_X86_REG_IP)
        if final_ip != RETURN_SENTINEL_IP + 1:  # HLT is 1 byte; IP already advanced past it
            raise RuntimeError(
                f'{name}{tuple(args)} did not return within {max_count} instructions '
                f'(stopped at IP={final_ip:#06x} -- looks like a guard-loop divergence)')

        # Resolve any still-pending PIT-lo-byte / standalone-nibble-write
        # classification at this call boundary -- every asm/SOUND.ASM
        # routine's OUT sequences are fully linear within one call (only
        # settling IN reads, never a call-boundary, sit between a PIT
        # lo/hi pair or an OPL reg/data pair), and the C port's matching
        # call emits its events atomically per call too, so this is the
        # right place to guarantee both logs line up one-for-one.
        self.flush_pending()

        # caller cleans the stack in cdecl -- our SP is already back where it
        # started (the callee's own `ret` popped only its own return address).
        sp = self.uc.reg_read(UC_X86_REG_SP)
        sp += 2 * len(args)
        self.uc.reg_write(UC_X86_REG_SP, sp)
        ax = self.uc.reg_read(UC_X86_REG_AX)
        return ax

    def poke_voice_cursors_direct(self) -> None:
        """Set voice_stream_cursor_table[i]=voice_stream_base_table[i]=0
        for all 4 voices directly, bypassing sound_voice_table_reload().
        Used only for the backend-mode-2 scenario: F_C7CB's mode-2 branch
        calls record_panel_rebuild() (F_D8F0), an unrelated UI record-
        panel renderer this oracle does not model (see the
        RECORD_PANEL_REBUILD_CS_OFFSET stub in __init__ and this
        function's C-side twin, dispatch_call()'s "voice_cursors_direct"
        case, in portable/tests/test_sound_parity.c) -- this achieves the
        same net per-voice state a non-crashing reload would have (cursor
        == base == offset 0 of the voice block) without exercising that
        dependency on either side of the comparison."""
        for voice in range(4):
            self.write_word(0x178C + voice * 2, 0)  # voice_stream_cursor_table[voice]
            self.write_word(0x1794 + voice * 2, 0)  # voice_stream_base_table[voice]

    def tick(self, n: int = 1) -> None:
        for _ in range(n):
            self._tick += 1
            self.run_routine('sound_tick_entry')

    # -- IN/OUT hooks: normalise to sound.h's four backend events --

    def _push_event(self, kind: int, a: int, b: int) -> None:
        self.events.append({'tick': self._tick, 'kind': kind, 'a': a & 0xFFFF, 'b': b & 0xFFFF})

    def _hook_in(self, uc, port, size, _ud):
        # "Read back the last thing written" -- the only IN pattern
        # asm/SOUND.ASM's own routines use (port 0x61 read-modify-write
        # gate bits; the OPL settling reads in F_C898/F_C8D4's callers
        # discard the value entirely, so any stable value is safe there).
        return self._port_shadow.get(port, 0)

    def _hook_out(self, uc, port, size, value, _ud):
        value &= 0xFF if size == 1 else 0xFFFF
        prev = self._port_shadow.get(port, 0)
        self._port_shadow[port] = value
        opl_port = self.read_word(0x1830)

        # Resolve any pending classification that a write to an unrelated
        # port proves must have been a standalone nibble write / PIT lo
        # byte (asm/SOUND.ASM never interleaves an unrelated OUT between a
        # PIT lo/hi pair or an OPL reg/data pair -- only IN settling reads,
        # which never reach this hook -- so reaching here with something
        # still pending means the pairing was never coming).
        if port not in (0x42,) and self._pit_pending is not None:
            self._push_event(EV_PIT_DIVISOR, self._pit_pending, 0)
            self._pit_pending = None
        if port not in (opl_port, opl_port + 1) and self._opl_pending is not None:
            self._push_event(EV_NIBBLE_WRITE, self._opl_pending, 0)
            self._opl_pending = None

        if port == 0x42:
            if self._pit_pending is None:
                self._pit_pending = value
            else:
                divisor = self._pit_pending | (value << 8)
                self._pit_pending = None
                self._push_event(EV_PIT_DIVISOR, divisor, 0)
            return

        if port == 0x61:
            # The C port's sound_backend_speaker_gate(enabled, tandy_mode)
            # is an UNCONDITIONAL per-call event, not a change-triggered
            # one -- asm/SOUND.ASM's three read-modify-write sites
            # (`or al,3` / `or al,60h` / `and al,0FCh`, immediately before
            # this `out 61h,al`) fire every time regardless of whether the
            # resulting byte actually differs from the port's last value,
            # so classify by which two-byte ALU op precedes this OUT
            # (IP still points at the `out` opcode itself here -- verified
            # empirically, see this file's Unicorn-quirk notes) rather
            # than by comparing before/after values.
            ip = self.uc.reg_read(UC_X86_REG_IP)
            prefix = bytes(self.uc.mem_read(CS_SEG * 16 + ip - 2, 2))
            if prefix == b'\x0c\x03':       # or al,3
                self._push_event(EV_SPEAKER_GATE, 1, 0)
            elif prefix == b'\x0c\x60':     # or al,60h
                self._push_event(EV_SPEAKER_GATE, 1, 1)
            elif prefix == b'\x24\xfc':     # and al,0FCh
                self._push_event(EV_SPEAKER_GATE, 0, 0)
            else:
                self._push_event(-1, port, value)
            return

        if port == opl_port:
            # Ambiguous in isolation: _opl_register_write (F_C898) writes
            # opl_port then, after settling reads, opl_port+1 (a REG+DATA
            # pair -> one OPL_WRITE event); _opl_port_write_byte (F_C8D4)
            # writes opl_port alone with no follow-up -> one NIBBLE_WRITE
            # event.  Defer the classification until the NEXT OUT proves
            # which one this was (handled by the flush logic above, and by
            # the opl_port+1 branch just below). A second consecutive write
            # to this same port (two standalone nibble writes back to
            # back, e.g. voice_disable's backend-mode-1 reset byte followed
            # by sound_pit_divisor_program's two nibbles) flushes the first
            # one now, since it can no longer be the register half of a
            # pair once superseded.
            if self._opl_pending is not None:
                self._push_event(EV_NIBBLE_WRITE, self._opl_pending, 0)
            self._opl_pending = value
            return

        if port == opl_port + 1 and self._opl_pending is not None:
            self._push_event(EV_OPL_WRITE, self._opl_pending, value)
            self._opl_pending = None
            return

        # Unknown port: keep the raw event so a divergence is still visible
        # instead of silently dropped.
        self._push_event(-1, port, value)

    def flush_pending(self) -> None:
        """Call after a routine sequence completes (or before diffing
        events) to resolve any still-pending PIT-lo-byte / standalone
        nibble write that never got a chance to be disambiguated by a
        following OUT within the same call."""
        if self._pit_pending is not None:
            self._push_event(EV_PIT_DIVISOR, self._pit_pending, 0)
            self._pit_pending = None
        if self._opl_pending is not None:
            self._push_event(EV_NIBBLE_WRITE, self._opl_pending, 0)
            self._opl_pending = None


def kind_name(kind: int) -> str:
    return _KIND_NAMES.get(kind, f'UNKNOWN({kind})')
