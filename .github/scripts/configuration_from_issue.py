#!/usr/bin/env python3
"""Takes the files attached to an "Organ configuration" issue and puts them into
libreorgans/ as the upload pair "<Name>.zip" + "<Name>.README.md".

The issue body (environment variable ISSUE_BODY) holds the attachments as
GitHub links (https://github.com/user-attachments/files/<id>/<name> or the older
https://github.com/<owner>/<repo>/files/<id>/<name>). Exactly one .zip is
expected - the configuration .json and its README inside (0.8.213; an older
export attaches the README .md beside the zip, still accepted); GitHub puts
dots for spaces in attachment names, so the README is renamed after the zip.
The pair is validated (configuration_tools.py: an organ configuration of
LibreOrgan Sampler, saved portable with the organ loaded, and the README the program
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


def split_readme_out(zip_path):
    """A README (.md) inside the zip is written beside it and removed from the zip
    (the zip is rewritten with its other members); returns the README's path or None."""
    import shutil
    import zipfile
    try:
        with zipfile.ZipFile(zip_path) as z:
            members = [n for n in z.namelist() if not n.endswith('/')]
            readmes = [n for n in members if n.lower().endswith('.md')]
            if not readmes:
                return None
            readme_path = os.path.join(os.path.dirname(zip_path), os.path.basename(readmes[0]))
            with open(readme_path, 'wb') as out:
                out.write(z.read(readmes[0]))
            rest = [(n, z.read(n)) for n in members if n not in readmes]
    except (zipfile.BadZipFile, OSError):
        return None
    tmp = zip_path + '.tmp'
    with zipfile.ZipFile(tmp, 'w', zipfile.ZIP_DEFLATED) as out:
        for name, data in rest:
            out.writestr(name, data)
    shutil.move(tmp, zip_path)
    print('README taken out of the zip: %s' % os.path.basename(readme_path))
    return readme_path


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
    os.makedirs('libreorgans', exist_ok=True)
    for url in urls:
        name = url.rsplit('/', 1)[-1]
        lower = name.lower()
        if not (lower.endswith('.zip') or lower.endswith('.md')):
            continue
        target = os.path.join('issue-files', name)
        os.makedirs('issue-files', exist_ok=True)
        request = urllib.request.Request(url, headers={'User-Agent': 'LibreOrgan-configurations'})
        with urllib.request.urlopen(request, timeout=60) as response, open(target, 'wb') as out:
            out.write(response.read())
        (zips if lower.endswith('.zip') else readmes).append(target)
    # Since 0.8.213 the zip holds the README as well: the upload is the one zip. The
    # README is taken OUT of it here (an attached README beside the zip is still
    # accepted: older exports), and the zip is written anew with the .json alone -
    # the published zip holds nothing else.
    if len(zips) == 1:
        inner_readme = split_readme_out(zips[0])
        if inner_readme and not readmes:
            readmes.append(inner_readme)
    if len(zips) != 1 or len(readmes) != 1:
        print('The issue must carry exactly one zip - the one LibreOrgan Sampler wrote beside the portable configuration, '
              'the configuration .json and its README inside (an older export: the zip and the README .md beside it). '
              'Found: %d zip, %d .md.' % (len(zips), len(readmes)))
        return 1
    # The files' names: GitHub turns the spaces of an attachment's name into dots
    # ("Adlington.Surround.portable.zip"), and the README's download link - written
    # by the program - names the file with its spaces: the zip got a name no link
    # reached (404 on every name with a space). The .json inside the zip keeps the
    # original name: both files are named after it.
    cfg, inner = tools.read_config(zips[0])
    if cfg is None:
        print('The attached zip cannot be used: %s' % inner)
        return 1
    base = os.path.basename(inner)[:-5] if inner.lower().endswith('.json') else os.path.basename(zips[0])[:-4]
    zip_target = os.path.join('libreorgans', base + '.zip')
    readme_target = os.path.join('libreorgans', base + '.README.md')
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
    # A new version of a published organ sent by somebody else than its publisher
    # (or the owner): not refused - the maintainer merges every pull request by
    # hand and that is the authorisation - but said in the pull request and the
    # issue (the owner's second account sent one and was refused, 25 Oct 2026).
    warning = ''
    for folder in replaces:
        publisher = tools.publisher_of_folder(folder)
        if author and owner and author != owner and publisher != author:
            warning = ('libreorgans/%s holds this organ already, published by %s; this version comes from @%s - merge it only if that is right.'
                       % (folder, ('@' + publisher) if publisher else 'somebody before the publisher was recorded', author))
    # The publisher, for the next version and for a removal (invisible on GitHub).
    with open(readme_target, encoding='utf-8') as f:
        readme = f.read()
    if author:
        readme = readme.rstrip('\n') + '\n\n<!-- published by @%s (issue #%s, %s) -->\n' % (author, issue or '?', datetime.date.today().isoformat())
        with open(readme_target, 'w', encoding='utf-8', newline='\n') as f:
            f.write(readme)
    print('libreorgans/%s.zip and libreorgans/%s.README.md -> folder %s%s'
          % (base, base, slug, (' (replaces ' + ', '.join(replaces) + ')') if replaces else ''))
    if warning:
        print('WARNING: ' + warning)
    with open(os.environ.get('GITHUB_OUTPUT', os.devnull), 'a') as out:
        out.write('base=%s\n' % base)
        out.write('slug=%s\n' % slug)
        out.write('replaces=%s\n' % ' '.join(replaces))
        out.write('warning=%s\n' % warning)
    return 0


if __name__ == '__main__':
    sys.exit(main())
