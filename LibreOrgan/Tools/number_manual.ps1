# Outline numbering of USER_MANUAL.md: the parts are "# N. Title"; every "## "
# section below a part gets "N.M " (existing numbers are replaced), the "### "
# option chapters stay as they are (their names are the "?" anchors). The
# "Contents" block (between the line "Contents" and the first "---") is
# rewritten as the full outline: every part and every section, one per line,
# separated by blank lines (each is a link in the program's manual window).
param([string]$Manual = (Join-Path (Split-Path $PSScriptRoot -Parent) 'USER_MANUAL.md'))
$lines = [IO.File]::ReadAllLines($Manual, [Text.Encoding]::UTF8)
$out = New-Object System.Collections.Generic.List[string]
$outline = New-Object System.Collections.Generic.List[string]
$part = 0; $section = 0; $inCode = $false
foreach ($line in $lines)
{
    if ($line.StartsWith('```')) { $inCode = -not $inCode; $out.Add($line); continue }
    if (-not $inCode -and $line -match '^# (\d+)\. (.*)$') { $part = [int]$Matches[1]; $section = 0; $outline.Add($line.Substring(2)); $out.Add($line); continue }
    if (-not $inCode -and $part -gt 0 -and $line -match '^## (?:\d+\.\d+ )?(.*)$')
    {
        $section++
        $numbered = "## $part.$section $($Matches[1])"
        $outline.Add([string][char]0x00A0 * 4 + "$part.$section $($Matches[1])")   # the sections indented under their part (no-break spaces survive the layout)
        $out.Add($numbered); continue
    }
    $out.Add($line)
}
# The contents block.
$text = New-Object System.Collections.Generic.List[string]
$i = 0
while ($i -lt $out.Count)
{
    $text.Add($out[$i])
    if ($out[$i] -eq 'Contents')
    {
        $text.Add('')
        foreach ($entry in $outline) { $text.Add($entry); $text.Add('') }
        $i++
        while ($i -lt $out.Count -and $out[$i] -ne '---') { $i++ }
        continue
    }
    $i++
}
[IO.File]::WriteAllLines($Manual, $text.ToArray(), (New-Object Text.UTF8Encoding($false)))
"numbered: $($outline.Count) outline entries"
