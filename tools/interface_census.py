"""Conservative declaration census for Turbo C's compact-model source subset."""
from collections import defaultdict
import re
from reconstruct import ROOT, read_json, write_json


def clean(text):
    text = re.sub(r'/\*.*?\*/|//[^\n]*', lambda m:'\n'*m[0].count('\n'), text, flags=re.S)
    # Blank string/char literal contents (keeping length/newlines) so embedded ',' ';' '{' '}'
    # never get mistaken for statement structure.
    return re.sub(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                   lambda m: re.sub(r'[^\n]', ' ', m[0]), text, flags=re.S)


def norm(text):
    text = re.sub(r'\b(extern|static|register|auto)\b', '', text)
    return ' '.join(text.replace('*', ' * ').split()) or 'int'


def argument_type(arg):
    arg = arg.strip()
    if arg in ('void', '...'): return arg
    if re.fullmatch(r'(?:unsigned|signed)(?:\s+(?:char|int|short|long))?|(?:char|int|short|long|float|double)|(?:struct|union|enum)\s+\w+', arg): return norm(arg)
    return norm(re.sub(r'\b[A-Za-z_]\w*\s*(\[[^]]*\])?$', lambda m:' *' if m[1] else '', arg))


def object_declarations(statement):
    """Simple C declarators, preserving pointer placement per comma member.

    A near/far/huge qualifier BEFORE the last '*' describes the pointer VALUE
    (what it points to / how big the pointer itself is: near=2 bytes, far/huge=4).
    A near/far qualifier AFTER the last '*' (between '*' and the identifier)
    describes where the OBJECT itself lives and never changes its size; for a
    non-pointer object any near/far qualifier is storage-only (pointer_shape stays None).
    """
    match = re.fullmatch(r'\s*(?:(?:extern|static|register|auto)\s+)*(struct\s+\w+|union\s+\w+|(?:unsigned|signed)(?:\s+(?:char|short|int|long))?|long(?:\s+int)?|short(?:\s+int)?|char|int|float|double|void)\b(.*)', statement, re.S)
    if not match: return []
    base, tail = match.groups(); result=[]
    for member in tail.split(','):
        member=member.split('=',1)[0].strip()
        item=re.fullmatch(r'((?:(?:near|far|huge|const|volatile)\s*|\*\s*)*)([A-Za-z_]\w*)\s*((?:\[[^]]*\]\s*)*)',member)
        if not item: return []
        prefix=item[1]; array=item[3].strip() or None
        if '*' in prefix:
            before,after=prefix.rsplit('*',1)
            pointer_shape='near' if re.search(r'\bnear\b',before) else 'far'
            storage='near' if re.search(r'\bnear\b',after) else 'far' if re.search(r'\bfar\b',after) else None
            prefix=before+'*'+re.sub(r'\b(near|far)\b','',after)
        else:
            pointer_shape=None
            storage='near' if re.search(r'\bnear\b',prefix) else 'far' if re.search(r'\bfar\b',prefix) else None
            prefix=re.sub(r'\b(near|far)\b','',prefix)
        # `type` is the value type; the object's own near/far placement is `storage`.
        typ=norm(base+' '+prefix)
        result.append({'symbol':item[2],'type':typ,'array':array,'pointer_shape':pointer_shape,
                       'storage':storage,'size':type_size(typ) if not array else None})
    return result


def struct_layout(body):
    """Field offsets/sizes for a brace-delimited struct body ('int w0; int w2')."""
    offset, fields, valid = 0, [], True
    for field in body.split(';'):
        if not field.strip(): continue
        decls=object_declarations(field)
        if not decls: valid=False; break
        for d in decls:
            size=type_size(d['type'])
            counts=re.findall(r'\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]',d['array'] or '')
            if size is None or d['array'] and len(counts)!=d['array'].count('['): valid=False; break
            for count in counts: size*=int(count,0)
            fields.append({'name':d['symbol'],'type':d['type'],'offset':offset,'bytes':size});offset+=size
        if not valid: break
    return offset, fields, valid


def declarations(text, path):
    text = clean(text)
    functions, globals_, records, unsupported = [], [], [], []
    def location(pos): return {'file': path, 'line': text.count('\n',0,pos)+1}
    # Anonymous struct object declarations ('struct { ... } name[];') never get a tag, so the
    # generic brace-flattening/statement splitter would split them into unrelated fragments.
    # Resolve them here (synthesizing a per-site tag) and blank the span out of `text` so the
    # rest of the pipeline never sees it.
    for m in list(re.finditer(r'\bstruct\s*\{([^{}]*)\}\s*([A-Za-z_]\w*)\s*((?:\[[^\]]*\]\s*)*);', text, re.S)):
        body, name, arr = m.groups(); pos=m.start(); loc=location(pos)
        offset, fields, valid = struct_layout(body)
        tag=f"<anonymous>@{loc['file']}:{loc['line']}"
        records.append({'name':tag,'bytes':offset if valid else None,'fields':fields,
                        'confidence':'MEDIUM' if valid else 'UNKNOWN',**loc})
        array=arr.strip() or None
        globals_.append({'symbol':name,'type':'struct <anonymous>','array':array,'pointer_shape':None,
                         'storage':None,'size':offset if valid and not array else None,
                         'confidence':'MEDIUM' if valid else 'UNKNOWN',**loc})
        text=text[:m.start()]+re.sub(r'[^\n]',' ',m[0])+text[m.end():]
    top, depth = [], 0
    for c in text:
        if c == '{':
            top.append(c if depth == 0 else ' '); depth += 1
        elif c == '}':
            depth -= 1; top.append(';' if depth == 0 else ' ')
        else: top.append(c if depth == 0 or c == '\n' else ' ')
    flattened = ''.join(top)
    flattened = re.sub(r'^\s*#.*$', lambda m:' '*len(m[0]), flattened, flags=re.M)
    def function(symbol, ret, args, definition, pos, confidence='MEDIUM'):
        return {'symbol':symbol,'definition':definition,'return_type':ret,
                'argument_count':None if args is None else len(args),'argument_types':args,
                'near_far':'far' if re.search(r'\bfar\b',ret) and '*' not in ret else 'near',
                'calling_convention':'pascal' if 'pascal' in ret else 'cdecl',
                'evidence':'Turbo C -mc: near code, far default data pointers; () has unspecified arguments',
                'confidence':confidence,**location(pos)}
    start, pending = 0, None
    for match in re.finditer(r'[;{]', flattened):
        fragment=flattened[start:match.start()]
        statement=fragment.strip(); pos=start+len(fragment)-len(fragment.lstrip()); start=match.end()
        if pending:
            pending['types'].update({d['symbol']:d['type'] for d in object_declarations(statement)})
            if match[0]=='{':
                functions.append(function(pending['symbol'],pending['return'],
                    [pending['types'].get(name,'int') for name in pending['names']],True,pending['pos']))
                pending=None
            continue
        if not statement: continue
        if re.fullmatch(r'(struct|union|enum)\s+\w+',statement): continue
        if statement.startswith('typedef '):
            unsupported.append({'declaration':statement,'reason':'typedef requires preprocessor/type environment',**location(pos)})
            continue
        if re.search(r'\(\s*\*',statement):
            # Pointer-to-function declarator: '(*name)(args)', '(*name[])(args)' or, when the
            # name itself is followed by its own parens, a function RETURNING a function pointer
            # ('(*getvect())()').
            fnptr=re.fullmatch(r'(.*?)\(\s*\*\s*((?:near|far)\s*)?([A-Za-z_]\w*)\s*((?:\[[^\]]*\]\s*)*)(\([^()]*\))?\s*\)\s*\(([^()]*)\)',statement,re.S)
            if fnptr:
                pre,qual,name,arr,innerparens,_outerargs=fnptr.groups()
                interrupt=bool(re.search(r'\binterrupt\b',pre))
                code_far=interrupt or bool(re.search(r'\bfar\b',pre))
                if innerparens:
                    functions.append(function(name,'UNKNOWN_FUNCTION_POINTER',None,match[0]=='{',pos,'UNKNOWN'))
                else:
                    storage=qual.strip() if qual else None
                    globals_.append({'symbol':name,'type':norm(statement),'array':arr.strip() or None,
                                     'pointer_shape':'function','code_model':'far' if code_far else 'near',
                                     'storage':storage,'interrupt':interrupt,
                                     'size':4 if code_far else 2,'confidence':'MEDIUM',**location(pos)})
                continue
            unsupported.append({'declaration':statement,'reason':'function pointer declarator; signature is not inferred',**location(pos)})
            name=re.search(r'\(\s*\*\s*(?:(?:near|far)\s*)?(\w+)(\s*\()?',statement)
            if name and name[2]:
                functions.append(function(name[1],'UNKNOWN_FUNCTION_POINTER',None,match[0]=='{',pos,'UNKNOWN'))
            elif name:
                globals_.append({'symbol':name[1],'type':statement,'array':None,'pointer_shape':'UNKNOWN',
                                 'size':None,'confidence':'UNKNOWN',**location(pos)})
            continue
        found = list(re.finditer(r'([A-Za-z_]\w*)\s*\(([^()]*)\)', statement))
        if found:
            ret = norm(statement[:found[0].start()])
            if '=' in ret: continue
            # K&R declaration list extends across semicolons up to the body.
            first=found[0]; suffix=statement[first.end():].strip()
            if suffix and not suffix.startswith(',') and re.fullmatch(r'\w+(?:\s*,\s*\w+)*',first[2]):
                pending={'symbol':first[1],'return':ret,'names':[n.strip() for n in first[2].split(',')],
                         'types':{d['symbol']:d['type'] for d in object_declarations(suffix)},'pos':pos}
                continue
            for f in found:
                args=f[2].strip()
                types=None if not args else [] if args=='void' else [argument_type(a) for a in args.split(',')]
                functions.append(function(f[1],ret,types,match[0]=='{',pos))
        else:
            objs=object_declarations(statement)
            globals_.extend({**d,'confidence':'MEDIUM',**location(pos)} for d in objs)
            if not objs:
                unsupported.append({'declaration':statement,'reason':'unsupported top-level declaration',**location(pos)})
    for struct in re.finditer(r'\bstruct\s+(\w+)\s*\{([^{}]*)\}', text, re.S):
        offset, fields, valid = struct_layout(struct[2])
        records.append({'name':struct[1],'bytes':offset if valid else None,'fields':fields,
                        'confidence':'MEDIUM' if valid else 'UNKNOWN',**location(struct.start())})
    return functions, globals_, records, unsupported


def type_size(typ):
    if '*' in typ:
        before=typ.rsplit('*',1)[0]
        return 2 if re.search(r'\bnear\b',before) else 4
    if re.search(r'\bchar\b',typ): return 1
    if re.search(r'\blong\b',typ): return 4
    if re.search(r'\b(int|short|unsigned)\b',typ): return 2
    return None


def census(root=ROOT):
    funcs, objs, records, unsupported = [], [], [], []
    for path in sorted(list((root/'src').glob('*.C')) + list((root/'include').glob('*.H'))):
        f,g,r,u=declarations(path.read_text(),path.relative_to(root).as_posix())
        funcs+=f; objs+=g; records+=r; unsupported+=u
    conflicts=[]
    def group(items, kind, keys):
        grouped=defaultdict(list)
        for item in items: grouped[item['symbol']].append(item)
        result=[]
        for symbol, decls in sorted(grouped.items()):
            differences=[key for key in keys if len({str(d[key]) for d in decls if d[key] is not None})>1]
            if differences:
                conflicts.append({'id':kind+':'+symbol, 'kind':kind, 'symbol':symbol, 'fields':differences,
                                  'priority':100 if any('far' in str(d) or 'near' in str(d) for d in differences) else 50,
                                  'difficulty':'MEDIUM', 'declarations':decls, 'action':'Compare declaration with definition; require byte-exact FAST proof before editing.'})
            result.append({'symbol':symbol,'declarations':decls,'definitions':[d for d in decls if d.get('definition')]})
        return result
    functions=group(funcs,'FUNCTION_ABI',['return_type','argument_count','argument_types','near_far','calling_convention'])
    globals_=group(objs,'GLOBAL_TYPE',['type','array','pointer_shape'])
    layouts=defaultdict(list)
    for record in records:
        if record['bytes'] is not None:
            # Exact field boundaries/types, independent of field names.
            signature=tuple((f['offset'],f['bytes'],f['type']) for f in record['fields'])
            layouts[signature].append(record)
    equivalents=[{'evidence':'identical compact-model field offsets, widths and types', 'bytes':rs[0]['bytes'],
                  'difficulty':'CHEAP', 'layouts':rs} for rs in layouts.values() if len(rs)>1]
    # Same size is a suggestion only, never claimed equivalent (C470 vs record27).
    size27=[r for r in records if r['bytes']==27]
    names=defaultdict(list)
    for r in records: names[r['name']].append(r)
    for name, rs in names.items():
        if len({str([(f['offset'],f['bytes'],f['type']) for f in r['fields']]) for r in rs})>1:
            conflicts.append({'id':'RECORD:'+name,'kind':'RECORD_LAYOUT','symbol':name,'priority':80,
                              'difficulty':'SUPERVISOR','layouts':rs,'action':'Local tags may be unrelated; compare storage evidence before consolidation.'})
    # Attach canonical storage bindings, aliases and containing BSS records.
    storage=defaultdict(list)
    manifest=read_json(root/'layout/manifest.json'); owners={o['id']:o for o in manifest['regions']}
    for o in owners.values():
        for symbol,b in o.get('build',{}).get('bindings',{}).items():
            if b.get('coordinate')=='DGROUP_offset':
                offset=b.get('offset')
                if offset is None and b.get('owner') in owners:
                    offset=owners[b['owner']]['start']-512-manifest['frames']['DGROUP']+b.get('addend',0)
                item={'offset':offset,'owner':b.get('owner'),'evidence':b.get('evidence'),'caller':o['id']}
                if item not in storage[symbol.lstrip('_')]: storage[symbol.lstrip('_')].append(item)
    existing_globals={g['symbol'] for g in globals_}
    globals_ += [{'symbol':symbol,'declarations':[],'definitions':[],'type_confidence':'UNKNOWN'}
                 for symbol in sorted(storage) if symbol not in existing_globals]
    globals_.sort(key=lambda g:g['symbol'])
    for obj in globals_:
        obj['storage_evidence']=storage[obj['symbol']]
        tags={m[1] for d in obj['declarations'] for m in [re.search(r'\bstruct\s+(\w+)',d['type'])] if m}
        obj['struct_membership']=[r for r in records if r['name'] in tags]
        obj['known_storage_ranges']=[]
        for declaration in obj['declarations']:
            if declaration['size'] is None:
                size=type_size(declaration['type'])
                matching=[r['bytes'] for r in obj['struct_membership'] if r['bytes'] is not None]
                if size is None and len(set(matching))==1: size=matching[0]
                array=declaration['array']
                count=re.fullmatch(r'\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]',array or '')
                if array and count: declaration['size']=size*int(count[1],0) if size is not None else None
                elif not array: declaration['size']=size
            if declaration['size'] is not None:
                for evidence in obj['storage_evidence']:
                    if evidence['offset'] is not None:
                        span=[evidence['offset'],evidence['offset']+declaration['size']]
                        if span not in obj['known_storage_ranges']: obj['known_storage_ranges'].append(span)
        obj['aliases']=sorted(s for s,ev in storage.items() if s!=obj['symbol'] and any(
            a['offset'] is not None and a['offset']==b['offset'] for a in ev for b in obj['storage_evidence']))
    indexpath=root/'layout/public-index.json'
    if indexpath.exists():
        byname={f['symbol']:f for f in functions}
        for public in read_json(indexpath)['publics']:
            symbol=public['symbol'].lstrip('_')
            if symbol not in byname:
                entry={'symbol':symbol,'declarations':[],'definitions':[], 'signature_confidence':'UNKNOWN'}
                functions.append(entry); byname[symbol]=entry
            byname[symbol].setdefault('omf_public_evidence',[]).append(public)
    functions.sort(key=lambda f:f['symbol'])
    return {'format':'empires-interface-census-v1','functions':functions,'globals':globals_,
            'records':records,'conflicts':sorted(conflicts,key=lambda c:(-c['priority'],c['id'])),
            'consolidation_candidates':equivalents,'record27_candidates':size27,'unsupported':unsupported,
            'limitations':['Syntactic census, not a C preprocessor or type checker.',
                          'Empty () is unspecified, never zero arguments.',
                          'Record sizes assume observed byte packing; complex declarators remain explicit unknowns.',
                          'Same-size records are suggestions only. Source compilation and exact bytes decide acceptance.']}


if __name__=='__main__':
    result=census(); (ROOT/'docs/current').mkdir(exist_ok=True)
    write_json(ROOT/'docs/current/interface-conflicts.json',result)
    print(f"ABI: {len(result['functions'])} function symbols; {len(result['globals'])} globals; {len(result['conflicts'])} conflicts")
