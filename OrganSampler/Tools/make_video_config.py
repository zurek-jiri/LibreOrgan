"""The configuration for the instruction video (0.8.206): a copy of the published Stralsund
configuration whose Oberwerk trees point at a drive that does not exist, so that the video can
show the "Sample folder not found" notice and the one click that finds the trees.
Usage: python -I make_video_config.py <Stralsund portable json> <out json>"""
import json, sys

src, out = sys.argv[1], sys.argv[2]
cfg = json.load(open(src, encoding='utf-8'))
gone = r'X:\Hauptwerk\HauptwerkSampleSetsAndComponents\OrganInstallationPackages'
trees = cfg['sampleTrees']
for i, t in enumerate(trees):
    if t.replace('\\', '/').endswith('/OW'):
        trees[i] = gone + '\\' + t.replace('/', '\\')
for m in cfg['manualKeyboards']:
    rel = m.get('folderRelativePath', '')
    if rel.replace('\\', '/').endswith('/OW'):
        m['folderPath'] = gone + '\\' + rel.replace('/', '\\')
        m['folderRelativePath'] = ''
        extras = [gone + '\\' + e.replace('/', '\\') for e in m.get('extraFolderRelativePaths', []) if e.replace('\\', '/').endswith('/OW')]
        m['extraFolderPaths'] = extras + [e for e in m.get('extraFolderPaths', [])]
        m['extraFolderRelativePaths'] = [e for e in m.get('extraFolderRelativePaths', []) if not e.replace('\\', '/').endswith('/OW')]
# The release crossfade of the published file (70 + 70 ms) is too short for the video (the user, 26 Oct 2026): 120 ms.
cfg['releaseFadeOutMs'] = 120.0
cfg['releaseFadeInMs'] = 120.0
# The stored combinations (piston 8 is recalled in the video) carry the Positive's tremulant: no tremulant in a
# big combination (the user, 26 Oct 2026).
for combination in cfg.get('combinationMemory', {}).get('combinations', []):
    for tremulant in combination.get('tremulants', []):
        tremulant['enabled'] = False
json.dump(cfg, open(out, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
print('written', out, 'trees', trees)
