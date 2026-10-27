#!/usr/bin/env python3
"""Writes ThirdParty/unarr-include/unarr.h from the submodule's unarr.h.in: the version
placeholders (@unarr_VERSION_MAJOR@ ...) filled in from the submodule's CMakeLists.txt
(project(unarr VERSION x.y.z)). The checked-in header serves the CMake build and the
Projucer/Visual Studio build alike (unarr's own CMake is not used: it asks for CMake 3.25
and builds 7z, ZIP and tar besides). Run from the repository root after a submodule
update: python OrganSampler/Tools/make_unarr_header.py
"""
import os
import re
import sys

root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..')
sub = os.path.join(root, 'ThirdParty', 'unarr')
template = os.path.join(sub, 'unarr.h.in')
if not os.path.exists(template):
    sys.exit('ThirdParty/unarr/unarr.h.in not found - is the submodule checked out?')
with open(os.path.join(sub, 'CMakeLists.txt'), encoding='utf-8') as f:
    m = re.search(r'project\s*\(\s*unarr[^)]*?VERSION\s+(\d+)\.(\d+)\.(\d+)', f.read(), re.I | re.S)
if not m:
    sys.exit('the unarr version was not found in ThirdParty/unarr/CMakeLists.txt')
major, minor, patch = m.groups()
with open(template, encoding='utf-8') as f:
    text = f.read()
text = (text.replace('@unarr_VERSION_MAJOR@', major).replace('@unarr_VERSION_MINOR@', minor)
            .replace('@unarr_VERSION_PATCH@', patch).replace('@unarr_VERSION@', '%s.%s.%s' % (major, minor, patch)))
left = re.findall(r'@[A-Za-z_]+@', text)
if left:
    sys.exit('placeholders left: ' + ', '.join(sorted(set(left))))
out_dir = os.path.join(root, 'ThirdParty', 'unarr-include')
os.makedirs(out_dir, exist_ok=True)
out = os.path.join(out_dir, 'unarr.h')
with open(out, 'w', encoding='utf-8', newline='\n') as f:
    f.write(text)
print('%s written (unarr %s.%s.%s)' % (os.path.relpath(out, root), major, minor, patch))
