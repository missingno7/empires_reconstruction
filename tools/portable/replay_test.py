"""Deterministic integration replay: run the portable executable headlessly with a
scripted input and compare the presented frames' SHA-256 against a manifest.

usage: replay_test.py <empires.exe> <manifest.json> [--update] [--assets DIR]
Exit 77 (ctest SKIP) when the assets are missing.  --update rewrites the manifest's
expected hashes from the current build (review the frames before committing!).
"""
import hashlib, json, os, subprocess, sys, tempfile, shutil

def main():
    exe, manifest_path = os.path.abspath(sys.argv[1]), os.path.abspath(sys.argv[2])
    update = '--update' in sys.argv
    assets = None
    if '--assets' in sys.argv:
        assets = sys.argv[sys.argv.index('--assets') + 1]
    assets = os.path.abspath(assets or os.path.join(os.path.dirname(manifest_path), '..', '..', '..', 'assets'))
    if not os.path.exists(os.path.join(assets, 'AE000.DAT')):
        print('replay: assets missing, skipping'); return 77
    m = json.load(open(manifest_path))
    work = tempfile.mkdtemp(prefix='empires-replay-')
    try:
        dump = os.path.join(work, 'frame.ppm')
        env = dict(os.environ, SDL_VIDEO_DRIVER='dummy', SDL_AUDIO_DRIVER='dummy')
        env.pop('EMPIRES_TRACE', None)
        env['EMPIRES_NOSOUND'] = '1' if m.get('no_sound', True) else ''
        if not m.get('no_sound', True): env.pop('EMPIRES_NOSOUND')
        # A manifest is one run, or a chain of runs sharing the saves directory
        # ("runs": [{script, virtual_ms, dump_interval_ms}, ...]) for save/resume flows.
        runs = m.get('runs') or [m]
        hashes, final = [], None
        for ri, run in enumerate(runs):
            for f in os.listdir(work):
                if f.startswith('frame.ppm'):
                    os.remove(os.path.join(work, f))
            cmd = [exe, '--deterministic', '--selftest-ms', str(run['virtual_ms']), '--script', run['script'],
                   '--assets', assets, '--saves', work, '--dump-vram', dump, '--dump-interval', str(run['dump_interval_ms'])]
            r = subprocess.run(cmd, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=m.get('timeout_s', 300))
            if r.returncode != 0:
                print('replay: executable failed (run %d)' % ri, r.returncode, r.stderr.decode(errors='replace')[-2000:]); return 1
            frames = sorted(f for f in os.listdir(work) if f.startswith('frame.ppm.'))
            hashes += [hashlib.sha256(open(os.path.join(work, f), 'rb').read()).hexdigest() for f in frames]
            final = hashlib.sha256(open(dump, 'rb').read()).hexdigest()
        if update:
            m['expected'] = {'frames': hashes, 'final': final}
            json.dump(m, open(manifest_path, 'w'), indent=2)
            print('replay: manifest updated with', len(hashes), 'frames'); return 0
        exp = m.get('expected', {})
        ok = exp.get('frames') == hashes and exp.get('final') == final
        if not ok:
            for i, (a, b) in enumerate(zip(exp.get('frames', []), hashes)):
                if a != b: print(f'replay: frame {i} differs: expected {a[:16]} got {b[:16]}')
            if len(exp.get('frames', [])) != len(hashes): print('replay: frame count', len(exp.get('frames', [])), '!=', len(hashes))
            if exp.get('final') != final: print('replay: final frame differs')
            return 1
        print('replay: OK,', len(hashes), 'frames +final match'); return 0
    finally:
        shutil.rmtree(work, ignore_errors=True)

if __name__ == '__main__':
    sys.exit(main())
