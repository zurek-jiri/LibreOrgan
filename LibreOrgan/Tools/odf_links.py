import sys, re, collections
path = sys.argv[1]; wanted_div = sys.argv[2].lower() if len(sys.argv) > 2 else ''
data = open(path, 'r', encoding='utf-8', errors='replace').read()
def table(name):
    m = re.search(r'<ObjectList ObjectType="%s">(.*?)</ObjectList>' % re.escape(name), data, re.S)
    return [dict(re.findall(r'<([A-Za-z0-9_]+)>([^<]*)</', r)) for r in re.findall(r'<o>(.*?)</o>', m.group(1), re.S)] if m else []
def gi(r,*ks):
    for k in ks:
        if k in r and r[k] != '': return int(float(r[k]))
    return 0
def gs(r,*ks):
    for k in ks:
        if k in r: return r[k]
    return ''
switches = {gi(r,'SwitchID','a'): gs(r,'Name','b') for r in table('Switch')}
divisions = {gi(r,'DivisionID','a'): gs(r,'Name','b') for r in table('Division')}
keyboards = {gi(r,'KeyboardID','a'): gs(r,'Name','b') for r in table('Keyboard')}
stops = {gi(r,'StopID','a'): (gs(r,'Name','b'), gi(r,'DivisionID','c'), gi(r,'ControllingSwitchID','d')) for r in table('Stop')}
kk = collections.defaultdict(list)
for r in table('KeyboardKey'): kk[gi(r,'KeyboardID','a')].append(gi(r,'SwitchID','b'))
di = {gi(r,'SwitchID','b'): gi(r,'DivisionID','a') for r in table('DivisionInput')}
links = collections.defaultdict(list)
for r in table('SwitchLinkage'): links[gi(r,'SourceSwitchID','a')].append((gi(r,'DestSwitchID','b'), gi(r,'ConditionSwitchID','c')))
stop_by_switch = {sw: (sid, n) for sid, (n, d, sw) in stops.items() if sw}
for kid, name in sorted(keyboards.items()):
    conds = collections.defaultdict(collections.Counter)   # division -> condition switch -> keys
    for sw in kk[kid]:
        seen = set(); stack = [(sw, ())]
        while stack:
            s, path = stack.pop()
            if s in seen: continue
            seen.add(s)
            if s in di and s != sw:
                conds[di[s]][path] += 1
            for dest, c in links.get(s, []): stack.append((dest, path + ((c,) if c else ())))
    for d, cc in sorted(conds.items()):
        if wanted_div and wanted_div not in divisions.get(d, '').lower(): continue
        print('keyboard %d "%s" -> division %d "%s":' % (kid, name, d, divisions.get(d)))
        for path, n in cc.items():
            names = ['%d:%s%s' % (c, switches.get(c, '?'), (' [STOP %s]' % stop_by_switch[c][1]) if c in stop_by_switch else '') for c in path]
            print('     keys %3d via %s' % (n, ' > '.join(names) if names else '(unconditional)'))
