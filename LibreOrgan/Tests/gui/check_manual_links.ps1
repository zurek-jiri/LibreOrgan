# Checks that every option of the Configuration Editor (from the dump written by
# dump_options_test.ps1) has its chapter in USER_MANUAL.md, with the same rule
# the "?" button uses: a heading named exactly like the option, else a heading
# containing its name, else the heading of its section. Prints what each option
# would open; options that only reach their section (or nothing) are flagged.
param([string]$Dump = "$env:TEMP\organ_options.txt",
      [string]$Manual = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\USER_MANUAL.md")
# Explicit help sections set in the code (addRow helpSection overrides the label).
$overrides = @{
    'Filled pipes: time stretch of the release (upwards)' = 'Filled pipes: time stretch of the release'
    'Filled pipes cache folder (default)' = 'Filled pipes cache folder'
}
$headings = Get-Content $Manual -Encoding UTF8 | Where-Object { $_ -match '^#' } | ForEach-Object { ($_ -replace '^#+', '').Trim() }
$exactOnly = 0; $contains = 0; $fallback = 0; $none = 0
foreach ($line in Get-Content $Dump -Encoding UTF8)
{
    $parts = $line -split "`t"
    if ($parts.Count -lt 3) { continue }
    $section = $parts[0]; $label = $parts[1]
    $wanted = $label
    if ($label -match '^Filled pipes cache: .* in ') { $wanted = 'Filled pipes cache: Empty the cache' }
    elseif ($label -match '^Perspective ".*" output channels$') { $wanted = 'Perspective output channels' }
    elseif ($label -match '^Perspective ".*" volume$') { $wanted = 'Perspective volume' }
    elseif ($label -match '^Perspective ".*" channel separation$') { $wanted = 'Perspective channel separation' }
    elseif ($overrides.ContainsKey($label)) { $wanted = $overrides[$label] }
    $hit = $headings | Where-Object { $_ -ieq $wanted } | Select-Object -First 1
    if ($hit) { $exactOnly++; continue }
    $hit = $headings | Where-Object { $_.ToLower().Contains($wanted.ToLower()) } | Select-Object -First 1
    if ($hit) { $contains++; "CONTAINS  [$section] $label -> $hit"; continue }
    $hit = $headings | Where-Object { $_ -ieq $section } | Select-Object -First 1
    if (-not $hit) { $hit = $headings | Where-Object { $_.ToLower().Contains($section.ToLower()) } | Select-Object -First 1 }
    if ($hit) { $fallback++; "SECTION   [$section] $label -> $hit"; continue }
    $none++; "NONE      [$section] $label"
}
"exact: $exactOnly, contains: $contains, section fallback: $fallback, none: $none"
