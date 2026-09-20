"""Content-addressed research cache. Acceptance callers must pass enabled=False."""
import hashlib
import json
import shutil
from reconstruct import compile_sources, read_json, sha


def cache_key(root, owner, lock):
    paths = [root / owner['source']] + sorted((root / 'include').glob('*'))
    metadata = {k: v for k, v in owner.items() if k != 'source'}
    payload = {'owner': metadata, 'toolchain': lock,
               'inputs': [(p.name, sha(p.read_bytes())) for p in paths if p.is_file()],
               'compiler_driver': sha((root / 'tools/reconstruct.py').read_bytes()),
               'cache_version': 1}
    return hashlib.sha256(json.dumps(payload, sort_keys=True).encode()).hexdigest()


def compile_cached(root, owners, work, toolchain, runner, lock, enabled=False):
    if not enabled:
        return compile_sources(root, owners, work, toolchain, runner, lock)
    for entry in lock['files']:
        if sha((toolchain / entry['path']).read_bytes()) != entry['sha256']:
            raise ValueError('Pinned compiler/assembler identity differs')
    cache = root / 'build/object-cache'
    cache.mkdir(parents=True, exist_ok=True)
    receipts, missing, keys = {}, [], {}
    for owner in owners:
        key = keys[owner['id']] = cache_key(root, owner, lock)
        obj, meta = cache / (key + '.obj'), cache / (key + '.json')
        if obj.exists() and meta.exists() and read_json(meta).get('sha256') == sha(obj.read_bytes()):
            target = work / 'cached' / (owner['id'] + '.OBJ')
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(obj, target)
            receipts[owner['id']] = {'object': str(target.relative_to(work)), 'cache_hit': True,
                                     'cache_key': key, 'object_sha256': sha(obj.read_bytes())}
        else:
            missing.append(owner)
    session = {'cache_hits': len(receipts), 'command_count': len(missing)}
    if missing:
        built, _ = compile_sources(root, missing, work / 'fresh', toolchain, runner, lock)
        for owner in missing:
            receipt = built[owner['id']]
            source = work / 'fresh' / receipt['object']
            key = keys[owner['id']]
            shutil.copyfile(source, cache / (key + '.obj'))
            (cache / (key + '.json')).write_text(json.dumps({'sha256': sha(source.read_bytes())}))
            receipts[owner['id']] = dict(receipt, object='fresh/' + receipt['object'], cache_hit=False, cache_key=key)
    return receipts, session
