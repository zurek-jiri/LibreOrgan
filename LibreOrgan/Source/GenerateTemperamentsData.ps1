# Pre-build step: embeds Source\temperaments.csv into the executable as the C string
# kTemperamentsCsv (Source\TemperamentsData.h), so the program needs no external file.
# The CSV may be UTF-8 (with or without BOM) or Windows-1252 ("ANSI"); the header is
# written with UTF-8 byte escapes only, so it compiles identically with any source charset.
param(
    [string]$Csv = (Join-Path $PSScriptRoot 'temperaments.csv'),
    [string]$Out = (Join-Path $PSScriptRoot 'TemperamentsData.h')
)

$bytes = [IO.File]::ReadAllBytes($Csv)
$strictUtf8 = New-Object System.Text.UTF8Encoding($false, $true)
try { $text = $strictUtf8.GetString($bytes) }
catch { $text = [System.Text.Encoding]::GetEncoding(1252).GetString($bytes) }
$text = $text.TrimStart([char]0xFEFF)

$sb = New-Object System.Text.StringBuilder
[void]$sb.AppendLine('// Generated from temperaments.csv by GenerateTemperamentsData.ps1 (pre-build step). Do not edit.')
[void]$sb.AppendLine('#pragma once')
[void]$sb.AppendLine('')
[void]$sb.AppendLine('static const char* const kTemperamentsCsv =')
foreach ($line in ($text -split "`r?`n"))
{
    if ($line.Trim().Length -eq 0) { continue }
    $escaped = New-Object System.Text.StringBuilder
    foreach ($ch in $line.ToCharArray())
    {
        $code = [int]$ch
        if ($ch -eq '\') { [void]$escaped.Append('\\') }
        elseif ($ch -eq '"') { [void]$escaped.Append('\"') }
        elseif ($code -ge 32 -and $code -lt 127) { [void]$escaped.Append($ch) }
        else
        {
            foreach ($b in [System.Text.Encoding]::UTF8.GetBytes([string]$ch)) { [void]$escaped.Append(('\x{0:X2}' -f $b)) }
            [void]$escaped.Append('""')   # ends the hex escape before a following hex-like letter
        }
    }
    [void]$sb.AppendLine('    "' + $escaped.ToString() + '\n"')
}
[void]$sb.AppendLine('    ;')
# The licence texts are embedded the same way (About & licences dialog), and the
# user manual (the "?" buttons of the Configuration Editor open it at a section).
& (Join-Path $PSScriptRoot 'GenerateLicencesData.ps1') | Out-Null
& (Join-Path $PSScriptRoot 'GenerateManualData.ps1') | Out-Null

$content = $sb.ToString()
if ((Test-Path $Out) -and ([IO.File]::ReadAllText($Out) -eq $content)) { exit 0 }   # unchanged: keep the timestamp
[IO.File]::WriteAllText($Out, $content, (New-Object System.Text.UTF8Encoding($false)))
Write-Host "TemperamentsData.h regenerated from temperaments.csv"
