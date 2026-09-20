"""Content identity of all construction inputs; independent of analysis packages."""
import json
from reconstruct import ROOT, read_json, sha


def input_fingerprint(root=ROOT, runtime_sha=None):
    paths=[]
    for folder in ('src','asm','include','layout','recipes','tools'):
        paths += [p for p in (root/folder).rglob('*') if p.is_file() and p.suffix.lower() in ('.py','.json','.c','.h','.asm')
                  and p.name not in ('task-state.json',)]
    for part in read_json(root/'recipes/data/game-initialized.json')['components']:
        if 'source' in part and (root/part['source']).exists(): paths.append(root/part['source'])
    data=[]
    for p in sorted(set(paths)):
        name=p.relative_to(root).as_posix()
        digest=runtime_sha if name=='asm/RUNTIME_BLOCK.ASM' and runtime_sha else sha(p.read_bytes())
        data.append((name,digest))
    return sha(json.dumps(data,separators=(',',':')).encode())
