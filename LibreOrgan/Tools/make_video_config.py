"""The configuration for the instruction video (0.8.206): a copy of the published Stralsund
configuration whose Oberwerk trees point at a drive that does not exist, so that the video can
show the "Sample folder not found" notice and the one click that finds the trees.
Usage: python -I make_video_config.py <Stralsund portable json> <out json>"""
import json, sys

src, out = sys.argv[1], sys.argv[2]
cfg = json.load(open(src, encoding='utf-8'))
trees = cfg['sampleTrees']
# 0.9.37: the Oberwerk trees stay RELATIVE (an absolute tree never passes the fingerprint test, and the
# lookup of 0.8.190-0.9.5 found the X: trees of the old version by the tails of their path anyway); a
# FINGERPRINT naming a stop folder the real tree does not hold refuses every candidate (0.9.5: "folder(s)
# of that name with OTHER samples refused") - the division says "not found", the relocation of the video's
# click (which checks no fingerprint) brings the trees in.
prints = cfg.setdefault('treeFingerprints', {})
for t in trees:
    if t.replace('\\', '/').endswith('/OW'):
        prints[t.replace('\\', '/')] = 'video_missing_folder|1|0|-'
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
