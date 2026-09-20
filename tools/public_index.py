"""Public address inventory from real final objects and pinned library modules."""
from reconstruct import ROOT, read_json, write_json, read_object
from omf import OmfReader
from pathlib import Path


def collect(root,plan,work):
    owners={o['id']:o for o in read_json(root/'layout/manifest.json')['regions']}
    publics=[]
    for module in plan['modules']:
        obj=read_object((work/'WORK'/module['object']).read_bytes())
        primary={p['symbol'] for p in module['publics']}
        for p in obj.publics_in('_TEXT'):
            publics.append({'symbol':p['name'],'address':module['start']-512+p['offset'],
                            'classification':'KNOWN_FUNCTION_ENTRY' if p['name'] in primary else 'KNOWN_SECONDARY_PUBLIC',
                            'owner':module['id'],'module_members':module['members']})
    lib=dict(OmfReader().split_library((work/'TC/LIB/CC.LIB').read_bytes()))
    for owner in owners.values():
        b=owner.get('build',{})
        if owner['kind']!='KNOWN_TOOLCHAIN_LIBRARY' or b.get('segment')!='_TEXT': continue
        obj=read_object(lib[b['library_module']])
        anchor=next((p['offset'] for p in obj.publics_in('_TEXT') if p['name']==b.get('public')),0)
        base=owner['start']-512-anchor
        for p in obj.publics_in('_TEXT'):
            if owner['start']-512<=base+p['offset']<owner['end']-512:
                publics.append({'symbol':p['name'],'address':base+p['offset'],'classification':'EXACT_PUBLIC',
                                'owner':owner['id'],'module_members':[b['library_module']]})
    startup=read_object((work/'TC/LIB/C0C.OBJ').read_bytes())
    for p in startup.publics_in('_TEXT'):
        publics.append({'symbol':p['name'],'address':p['offset'],'classification':'EXACT_PUBLIC','owner':'C0C','module_members':['C0C']})
    return {'format':'empires-public-index-v1','coordinate':'load-image byte address',
            'evidence':'Final production source objects and pinned CC.LIB/C0C.OBJ; checked against fresh objects at acceptance.',
            'publics':sorted(publics,key=lambda p:(p['address'],p['symbol'],p['owner']))}


if __name__=='__main__':
    report=read_json(ROOT/'build/exe-build-report.json')
    write_json(ROOT/'layout/public-index.json',collect(ROOT,read_json(ROOT/'layout/production-plan.json'),Path(report['session'])))
