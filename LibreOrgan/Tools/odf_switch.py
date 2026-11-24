# Everything a definition says about the switches whose name holds a text: the switch row and every row of
# SwitchLinkage, KeyboardKey, DivisionInput, Stop, Tremulant, KeyAction, Pipe_SoundEngine01, CombinationElement,
# ContinuousControlStageSwitch that names its id (first matches only).
# Usage: python -I odf_switch.py <odf> <name part> [max switches]
import re, sys
data = open(sys.argv[1], 'r', encoding='utf-8', errors='replace').read()
part = sys.argv[2].lower()
limit = int(sys.argv[3]) if len(sys.argv) > 3 else 2

def rows(name):
    m = re.search(r'<ObjectList ObjectType="%s">(.*?)</ObjectList>' % re.escape(name), data, re.S | re.I)
    if not m:
        return []
    found = re.findall(r'<o>(.*?)</o>', m.group(1), re.S)
    return found or [re.sub(r'\s+', '', x) for x in re.findall(r'<%s>(.*?)</%s>' % (name, name), m.group(1), re.S | re.I)]

tables = {name: rows(name) for name in ('Switch', 'SwitchLinkage', 'KeyboardKey', 'DivisionInput', 'Stop', 'Tremulant', 'KeyAction',
                                        'Pipe_SoundEngine01', 'CombinationElement', 'ContinuousControlStageSwitch', 'Keyboard')}
print('rows:', ', '.join('%s %d' % (k, len(v)) for k, v in tables.items()))
print('first KeyboardKey:', tables['KeyboardKey'][0][:200] if tables['KeyboardKey'] else '-')
print('first Keyboard:', tables['Keyboard'][0][:300] if tables['Keyboard'] else '-')
print('first SwitchLinkage:', tables['SwitchLinkage'][0][:200] if tables['SwitchLinkage'] else '-')
shown = 0
for r in tables['Switch']:
    name = re.search(r'<(?:b|Name)>([^<]*)<', r)
    if not name or part not in name.group(1).lower():
        continue
    sid = re.search(r'<(?:a|SwitchID)>(\d+)<', r).group(1)
    print('== switch', sid, ':', r[:260])

    def follow(sid, depth):
        for table_name, table_rows in tables.items():
            if table_name == 'Switch':
                continue
            n = 0
            for other in table_rows:
                if re.search(r'>%s<' % sid, other):
                    n += 1
                    if n <= 3:
                        print('   ' * depth + ' in %s: %s' % (table_name, other[:230]))
                        if table_name == 'SwitchLinkage' and depth < 3:
                            src = re.search(r'<(?:a|SourceSwitchID)>(\d+)<', other)
                            if src and src.group(1) != sid:
                                for s in tables['Switch']:
                                    if re.search(r'<(?:a|SwitchID)>%s<' % src.group(1), s):
                                        print('   ' * depth + '    source switch: ' + s[:200])
                                        break
                                follow(src.group(1), depth + 1)
            if n > 3:
                print('   ' * depth + ' ... %d rows of %s' % (n, table_name))
    follow(sid, 1)
    shown += 1
    if shown >= limit:
        break
