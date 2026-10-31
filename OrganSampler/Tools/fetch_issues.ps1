# Downloads the GitHub issues of LibreOrgan Sampler - text, comments and every
# attachment (log zips, screenshots, configuration files) - into a local
# folder, one subfolder per issue, so they can be read and analysed offline
# (by a person or by a coding agent working in this repository).
#
#   fetch_issues.ps1                       open issues, into .\issues\
#   fetch_issues.ps1 -State all            closed ones too
#   fetch_issues.ps1 -Label bug -Since 2026-10-01
#   fetch_issues.ps1 -Number 12            one issue
#   fetch_issues.ps1 -Output D:\reports    another folder
#
# Needs the GitHub CLI (gh), logged in (gh auth login) - or pass -Gh <path to gh.exe>
# and set GH_CONFIG_DIR when the login lives elsewhere. Attachments of a public
# repository download without a token; private ones use gh's token.
#
# The result per issue:  issues\<number>\issue.md      title, labels, author, dates, body, every comment
#                         issues\<number>\issue.json    the raw data
#                         issues\<number>\files\        the attachments (zips are unpacked beside them)
#                         issues\index.md               one line per issue
param(
    [string]$Repo = "zurek-jiri/LibreOrgan",
    [string]$State = "open",
    [string]$Label = "",
    [string]$Since = "",
    [int]$Number = 0,
    [string]$Output = (Join-Path (Get-Location) "issues"),
    [string]$Gh = "gh",
    [int]$Limit = 200
)
$ErrorActionPreference = "Stop"
if (-not (Get-Command $Gh -ErrorAction SilentlyContinue)) { throw "gh not found ($Gh). Install the GitHub CLI or pass -Gh <path>." }
New-Item -ItemType Directory -Force -Path $Output | Out-Null

$fields = "number,title,state,labels,author,createdAt,updatedAt,body,url,comments"
$issues = @()
if ($Number -gt 0) {
    $issues = @(& $Gh issue view $Number --repo $Repo --json $fields | ConvertFrom-Json)
} else {
    $args = @("issue", "list", "--repo", $Repo, "--state", $State, "--limit", $Limit, "--json", $fields)
    if ($Label -ne "") { $args += @("--label", $Label) }
    $list = & $Gh @args | ConvertFrom-Json
    foreach ($issue in $list) {
        if ($Since -ne "" -and [datetime]$issue.updatedAt -lt [datetime]$Since) { continue }
        $issues += $issue
    }
}
"$($issues.Count) issue(s)"

# Attachments are links in the markdown: github.com/user-attachments/..., user-images.githubusercontent.com/..., or /assets/ of the repo.
$attachmentPattern = 'https://(github\.com/user-attachments/(assets|files)/[^\s\)"<>]+|user-images\.githubusercontent\.com/[^\s\)"<>]+|github\.com/[^\s\)"<>]+/assets/[^\s\)"<>]+)'
$token = ""
try { $token = (& $Gh auth token 2>$null) } catch {}

function Save-Attachment($url, $folder, $index) {
    $name = ($url -split "/")[-1] -replace '[^\w\.\-]', '_'
    if ($name -eq "" -or $name.Length -gt 80) { $name = "attachment$index" }
    $path = Join-Path $folder $name
    try {
        $headers = @{ "User-Agent" = "LibreOrgan-fetch_issues" }
        if ($token -ne "") { $headers["Authorization"] = "Bearer $token" }
        $response = Invoke-WebRequest -Uri $url -Headers $headers -MaximumRedirection 5 -OutFile $path -PassThru
        # A content type tells the extension when the URL has none (user-attachments/assets).
        $type = [string]$response.Headers["Content-Type"]
        if ([IO.Path]::GetExtension($path) -eq "") {
            $ext = switch -Wildcard ($type) { "image/png*" { ".png" } "image/jpeg*" { ".jpg" } "image/gif*" { ".gif" } "application/zip*" { ".zip" } "application/json*" { ".json" } "text/*" { ".txt" } default { "" } }
            if ($ext -ne "") { $new = $path + $ext; Move-Item $path $new -Force; $path = $new }
        }
        if ([IO.Path]::GetExtension($path) -eq ".zip") {
            $unpack = Join-Path $folder ([IO.Path]::GetFileNameWithoutExtension($path))
            try { Expand-Archive -Path $path -DestinationPath $unpack -Force } catch { "  cannot unpack $path : $_" }
        }
        return $path
    } catch {
        "  download failed: $url ($_)"
        return $null
    }
}

$index = @()
foreach ($issue in $issues) {
    $folder = Join-Path $Output ([string]$issue.number)
    $files = Join-Path $folder "files"
    New-Item -ItemType Directory -Force -Path $files | Out-Null
    $issue | ConvertTo-Json -Depth 20 | Set-Content (Join-Path $folder "issue.json") -Encoding utf8
    $labels = ($issue.labels | ForEach-Object { $_.name }) -join ", "
    $md = @()
    $md += "# #$($issue.number): $($issue.title)"
    $md += ""
    $md += "- state: $($issue.state)   labels: $labels"
    $md += "- author: $($issue.author.login)   created: $($issue.createdAt)   updated: $($issue.updatedAt)"
    $md += "- url: $($issue.url)"
    $md += ""
    $md += "## Body"
    $md += ""
    $md += $issue.body
    $texts = @([string]$issue.body)
    foreach ($comment in $issue.comments) {
        $md += ""
        $md += "## Comment by $($comment.author.login) ($($comment.createdAt))"
        $md += ""
        $md += $comment.body
        $texts += [string]$comment.body
    }
    $urls = @()
    foreach ($text in $texts) { foreach ($m in [regex]::Matches($text, $attachmentPattern)) { $urls += $m.Value } }
    $urls = $urls | Sort-Object -Unique
    $md += ""
    $md += "## Attachments"
    $n = 0
    foreach ($url in $urls) {
        $n++
        $saved = Save-Attachment $url $files $n
        if ($saved) { $md += "- $url -> files/$([IO.Path]::GetFileName($saved))" } else { $md += "- $url (not downloaded)" }
    }
    $md -join "`n" | Set-Content (Join-Path $folder "issue.md") -Encoding utf8
    $index += "- #$($issue.number) [$($issue.state)] $($issue.title)  ($labels; $($urls.Count) attachment(s); updated $($issue.updatedAt))"
    "#$($issue.number) $($issue.title): $($urls.Count) attachment(s)"
}
$index -join "`n" | Set-Content (Join-Path $Output "index.md") -Encoding utf8
"written to $Output"
