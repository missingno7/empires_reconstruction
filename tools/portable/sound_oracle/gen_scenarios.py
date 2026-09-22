#!/usr/bin/env python3
"""tools/portable/sound_oracle/gen_scenarios.py -- generate
portable/tests/fixtures/sound_scenario_*.txt from the real asm/SOUND.ASM
code (via emu.py's Unicorn oracle) driving REAL game resource records.

Resource facts (task requirement 2; cross-checked against
src/PLRLDPUB.C, src/RESCACHE.C, src/RESOURCE.C -- see emu.py's own header
comment for the segment/offset facts):

* The sound-resource block (what snd_seg:snd_base pointed at) is AE000
  record 0x41 (65): src/PLRLDPUB.C's player_record_load_publish() calls
  `resource_load_record_alloc(0x41, &gc5de)`, and resource_load_record()
  splits its argument as `dir = p >> 12, record = p & 0xFFF` -- 0x41 has
  dir 0, i.e. AE000.DAT (tools/portable/oracle/README.md's own RESOURCE op
  documents dir 0 == AE000.DAT).
* The voice/staging block (snd_seg2:snd_base2, the 0x620-byte gc5da
  buffer) is filled per level by src/RESCACHE.C's
  resource_record_cache_load(v) -- `resource_load_record_into(v, gc5da)`
  in backend mode 0, `(v+1, gc5da)` otherwise. src/LEVEL.C calls
  resource_record_cache_reset(67) and (69) (also 49 from PLAYERSL.C); both
  are dir-0/AE000 records under 0x1000. This oracle uses record 67.
* Records are decoded with tools/resource_codecs.decode_payload() exactly
  as resource_load_record() does (2-byte header: gc0cb type, flags byte;
  decode_payload() undoes the LZ/RLE compression stages) -- RAW payload,
  no sprite fix-up (resource_load_record()'s display_mode==5 branch skips
  that switch entirely, and the sound records' gc0cb byte never matches
  the sprite-marker cases 0/1/0x47 either way).

Each scenario drives the SAME call sequence a real level start makes
(src/RESCACHE.C/LEVEL.C): sound_backend_select_init() (after setting
snd_backend_mode), sound_voice_table_reload(0), a few
stream_control_block_arm(n) cue ids (src/GAME.C, src/PUZZLE.C,
src/INTRO.C), sound_voices_reset(), then N sound_tick_entry() ticks.
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT / 'tools' / 'portable'))
sys.path.insert(0, str(ROOT / 'tools' / 'portable' / 'sound_oracle'))

from resource_codecs import decode_payload  # noqa: E402
import emu  # noqa: E402

AE000_PATH = ROOT / 'assets' / 'AE000.DAT'
FIXTURE_DIR = ROOT / 'portable' / 'tests' / 'fixtures'

SOUND_RESOURCE_RECORD = 0x41   # record 65, dir 0 (AE000.DAT) -- see header comment
VOICE_STAGING_RECORD = 67      # src/LEVEL.C's resource_record_cache_reset(67)
VOICE_STAGING_BLOCK_SIZE = 0x620  # gc5da's real farmalloc size (src/PLRLDPUB.C)


def load_table(data: bytes):
    n0 = struct.unpack_from('<I', data, 0)[0]
    n = n0 // 4
    return struct.unpack_from('<%dI' % n, data, 0)


def decode_record(archive_bytes: bytes, table, index: int) -> bytes:
    o1, o2 = table[index], table[index + 1]
    raw = archive_bytes[o1:o2]
    gc0cb, fl = raw[0], raw[1]
    payload = raw[2:]
    return decode_payload(bytes(payload), fl & 3)


class Call:
    """One entry in a scenario's call sequence."""
    def __init__(self, text: str, run):
        self.text = text  # fixture line, e.g. "arm 2"
        self.run = run    # callable(machine) -> None


def call_select_init():
    return Call('select_init', lambda m: m.run_routine('sound_backend_select_init'))


def call_voice_table_reload(v: int):
    return Call(f'voice_table_reload {v}', lambda m: m.run_routine('sound_voice_table_reload', (v,)))


def call_arm(n: int):
    return Call(f'arm {n}', lambda m: m.run_routine('stream_control_block_arm', (n,)))


def call_voice_cursors_direct():
    return Call('voice_cursors_direct', lambda m: m.poke_voice_cursors_direct())


def call_voices_reset():
    return Call('voices_reset', lambda m: m.run_routine('sound_voices_reset'))


def call_disable_all():
    return Call('disable_all', lambda m: m.run_routine('sound_voices_disable_all'))


def build_machine(backend_mode: int, resource_bytes: bytes | None, voice_bytes: bytes | None):
    m = emu.SoundMachine()
    m.write_word(0x1778, backend_mode & 0xFFFF)  # snd_backend_mode
    m.write_word(0x176E, 1)                      # sound_enabled
    m.write_word(0x1772, 1)                      # music_enabled
    if resource_bytes is not None:
        m.load_resource_block(resource_bytes)
    if voice_bytes is not None:
        m.load_voice_block(voice_bytes, VOICE_STAGING_BLOCK_SIZE)
    return m


def run_scenario(name: str, backend_mode: int, resource_bytes, voice_bytes, calls, ticks: int):
    m = build_machine(backend_mode, resource_bytes, voice_bytes)
    initial_region = m.region_snapshot()
    for call in calls:
        call.run(m)
    if ticks:
        m.tick(ticks)
        calls = list(calls) + [Call(f'ticks {ticks}', None)]
    final_region = m.region_snapshot()
    write_fixture(name, backend_mode, resource_bytes, voice_bytes, calls, ticks, m.events, initial_region, final_region)


def write_fixture(name, backend_mode, resource_bytes, voice_bytes, calls, ticks, events, initial_region, final_region):
    path = FIXTURE_DIR / f'sound_scenario_{name}.txt'
    lines = []
    lines.append(f'# sound_scenario_{name}.txt -- generated by tools/portable/sound_oracle/gen_scenarios.py')
    lines.append('# Do not hand-edit; regenerate with: python tools/portable/sound_oracle/gen_scenarios.py')
    lines.append(f'BACKEND_MODE {backend_mode}')
    lines.append(f'RESOURCE_HEX {(resource_bytes or b"").hex()}')
    lines.append(f'VOICE_HEX {(voice_bytes or b"").hex()}')
    lines.append(f'REGION_INITIAL_HEX {initial_region.hex()}')
    lines.append('CALLS')
    for call in calls:
        if call.run is not None:  # skip the synthetic 'ticks N' marker here
            lines.append(call.text)
    lines.append('END_CALLS')
    lines.append(f'TICKS {ticks}')
    lines.append(f'EVENTS {len(events)}')
    for ev in events:
        lines.append(f"{ev['tick']} {ev['kind']} {ev['a']} {ev['b']}")
    lines.append('END_EVENTS')
    lines.append(f'REGION_FINAL_HEX {final_region.hex()}')
    lines.append('')
    path.write_text('\n'.join(lines), encoding='ascii')
    size = path.stat().st_size
    print(f'{path.relative_to(ROOT)}: {size} bytes, {len(events)} events, backend_mode={backend_mode}')


def main():
    if not AE000_PATH.exists():
        print(f'{AE000_PATH} not found -- cannot generate scenarios (assets/AE000.DAT required)', file=sys.stderr)
        return 1

    archive = AE000_PATH.read_bytes()
    table = load_table(archive)
    resource_bytes = decode_record(archive, table, SOUND_RESOURCE_RECORD)
    voice_bytes = decode_record(archive, table, VOICE_STAGING_RECORD)
    print(f'AE000 record 0x{SOUND_RESOURCE_RECORD:02x}: {len(resource_bytes)} decoded bytes (sound resource)')
    print(f'AE000 record {VOICE_STAGING_RECORD}: {len(voice_bytes)} decoded bytes (voice staging)')

    FIXTURE_DIR.mkdir(parents=True, exist_ok=True)

    # Scenario 1: sanity check (task requirement 4) -- a known routine with
    # a known, unconditional effect: sound_voices_disable_all() in backend
    # mode 0 must emit the port-0x61 gate-off write (SPEAKER_GATE(0,0)).
    # No resource/voice blocks needed (disable_all never reads them).
    run_scenario(
        'sanity_gate_off', backend_mode=0, resource_bytes=None, voice_bytes=None,
        calls=[call_select_init(), call_disable_all()], ticks=0)

    # Scenario 2: backend mode 0 (PC speaker), the default/most common path
    # -- real resource records, the same call order src/RESCACHE.C and
    # src/GAME.C/PUZZLE.C use at level start (arm 2/3 -- src/GAME.C; arm 26
    # -- src/PUZZLE.C's puzzle-solved cue).
    run_scenario(
        'mode0_basic', backend_mode=0, resource_bytes=resource_bytes, voice_bytes=voice_bytes,
        calls=[
            call_select_init(),
            call_voice_table_reload(0),
            call_arm(2),
            call_arm(3),
            call_arm(26),
            call_voices_reset(),
        ],
        ticks=2000)

    # Scenario 3: backend mode 2 (queued OPL bank) -- exercises
    # opl_init()'s call chain (F_D99B et al, still inside the same _TEXT
    # mapping) from inside sound_backend_select_init(), then the same cue
    # arms as scenario 2. Uses voice_cursors_direct instead of
    # voice_table_reload(0): backend mode 2's reload path calls
    # record_panel_rebuild() (F_D8F0), an unrelated UI record-panel
    # renderer neither this oracle nor the plain sound-driver test
    # environment models (real crash observed on both sides -- the ASM
    # under Unicorn with no VRAM/other resource blocks mapped, and the C
    # port's own record_panel_rebuild() with no framebuffer/gc5da state
    # set up); see SoundMachine.poke_voice_cursors_direct()'s docstring.
    run_scenario(
        'mode2_queued_opl', backend_mode=2, resource_bytes=resource_bytes, voice_bytes=voice_bytes,
        calls=[
            call_select_init(),
            call_voice_cursors_direct(),
            call_arm(2),
            call_arm(3),
            call_arm(26),
            call_voices_reset(),
        ],
        ticks=2000)

    return 0


if __name__ == '__main__':
    raise SystemExit(main())
