#!/usr/bin/env python3
"""Takes the files attached to an "Organ configuration" issue and puts them into
configurations/ as the upload pair "<Name>.zip" + "<Name>.README.md".

The issue body (environment variable ISSUE_BODY) holds the attachments as
GitHub links (https://github.com/user-attachments/files/<id>/<name> or the older
https://github.com/<owner>/<repo>/files/<id>/<name>). Exactly one .zip (the
configuration .json inside) and one .md (the README) are expected; GitHub puts
dots for spaces in attachment names, so the README is renamed after the zip.
The pair is validated (configuration_tools.py: an organ configuration of
OrganSampler, saved portable with the organ loaded, and the README the program
wrote for it); a new version of an organ already published replaces its folder
after the merge - only its publisher or the repository's owner may send one
(ISSUE_AUTHOR, REPO_OWNER). The README gets the publisher's login appended as
an HTML comment. Prints what it did; exits 1 with a message for the issue when
something is wrong. Run from the repository root.
"""
import datetime
import os
import re
import sys
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import configuration_tools as tools  # noqa: E402


def main():
    body = os.environ.get('ISSUE_BODY', '')
    author = os.environ.get('ISSUE_AUTHOR', '').strip()
    owner = os.environ.get('REPO_OWNER', '').strip()
    issue = os.environ.get('ISSUE_NUMBER', '').strip()
    # GitHub attachments, or any https link straight to a .zip or .md file.
    urls = re.findall(r'https://github\.com/(?:user-attachments/files|[^/\s]+/[^/\s]+/files)/\d+/[^\s)\]>"\']+', body)
    urls += re.findall(r'https://[^\s)\]>"\']+\.(?:zip|md)\b', body)
    urls = list(dict.fromkeys(urls))   # unique, in order
    zips, readmes = [], []
    os.makedirs('configurations', exist_ok=True)
    for url in urls:
        name = url.rsplit('/', 1)[-1]
        lower = name.lower()
        if not (lower.endswith('.zip') or lower.endswith('.md')):
            continue
        target = os.path.join('issue-files', name)
        os.makedirs('issue-files', exist_ok=True)
        request = urllib.request.Request(url, headers={'User-Agent': 'OrganSampler-configurations'})
        with urllib.request.urlopen(request, timeout=60) as response, open(target, 'wb') as out:
            out.write(response.read())
        (zips if lower.endswith('.zip') else readmes).append(target)
    if len(zips) != 1 or len(readmes) != 1:
        print('The issue must carry exactly one zip (the configuration .json inside) and one README .md - '
              'the two files OrganSampler wrote beside the portable configuration. Found: %d zip, %d .md.'
              % (len(zips), len(readmes)))
        return 1
    base = os.path.basename(zips[0])[:-4]
    zip_target = os.path.join('configurations', base + '.zip')
    readme_target = os.path.join('configurations', base + '.README.md')
    os.replace(zips[0], zip_target)
    os.replace(readmes[0], readme_target)
    problems = tools.validate(zip_target, readme_target)
    if problems:
        print('The files are not a publishable organ configuration:')
        for p in problems:
            print('- ' + p)
        return 1
    cfg, _ = tools.read_config(zip_target)
    slug = tools.slug_for(zip_target, cfg)
    replaces = tools.same_organ(zip_target, cfg)
    # A new version of a published organ: its publisher or the owner only.
    for folder in replaces:
        publisher = tools.publisher_of_folder(folder)
        if author and owner and author != owner and publisher and publisher != author:
            print('configurations/%s holds this organ already, published by @%s - only they or the repository\'s owner can send a new version of it.'
                  % (folder, publisher))
            return 1
        if author and owner and author != owner and not publisher:
            print('configurations/%s holds this organ already (published before the publisher was recorded) - only the repository\'s owner can replace it.' % folder)
            return 1
    # The publisher, for the next version and for a removal (invisible on GitHub).
    with open(readme_target, encoding='utf-8') as f:
        readme = f.read()
    if author:
        readme = readme.rstrip('\n') + '\n\n<!-- published by @%s (issue #%s, %s) -->\n' % (author, issue or '?', datetime.date.today().isoformat())
        with open(readme_target, 'w', encoding='utf-8', newline='\n') as f:
            f.write(readme)
    print('configurations/%s.zip and configurations/%s.README.md -> folder %s%s'
          % (base, base, slug, (' (replaces ' + ', '.join(replaces) + ')') if replaces else ''))
    with open(os.environ.get('GITHUB_OUTPUT', os.devnull), 'a') as out:
        out.write('base=%s\n' % base)
        out.write('slug=%s\n' % slug)
        out.write('replaces=%s\n' % ' '.join(replaces))
    return 0


if __name__ == '__main__':
    sys.exit(main())
