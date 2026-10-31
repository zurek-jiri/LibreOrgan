# The filled-pipes cache on the Stralsund configuration: starts the program with
# the configuration loading at start-up and the cache enabled, waits until the
# copy builds are over (no "Copy build" line for 15 s) and reports how long they
# took and how many pipes came from the cache. Run twice: the first run fills
# the cache (or finds it), the second must read everything.
# Argument: 'clear' empties the cache folder first. Settings backed up/restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_cr_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
$json | Add-Member -NotePropertyName copyCacheEnabled -NotePropertyValue $true -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$cacheDir = "$appData\FilledPipesCache"
if ($args -contains 'clear' -and (Test-Path $cacheDir)) { Get-ChildItem $cacheDir -File | ForEach-Object { [IO.File]::Delete($_.FullName) }; 'cache emptied first' }
$filesBefore = if (Test-Path $cacheDir) { (Get-ChildItem $cacheDir -File | Measure-Object).Count } else { 0 }
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
$t0 = Get-Date
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$tDone = $null; $lastBuild = $null; $tFirstBuild = $null; $seenBuilds = 0
$limit = if ($args -contains 'long') { 2400 } else { 900 }
while (((Get-Date) - $t0).TotalSeconds -lt $limit)
{
    Start-Sleep -Seconds 1
    if ($p.HasExited) { "APP EXITED at $([int]((Get-Date) - $t0).TotalSeconds) s (closed by someone else?)"; break }
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42) | Out-Null }
    $lines = Get-Content $log | Select-Object -Skip $before
    if (-not $tDone -and ($lines | Select-String -Pattern 'Compiling the Stop List: done' -Quiet)) { $tDone = Get-Date; "stop list compiled at $([int]((Get-Date) - $t0).TotalSeconds) s" }
    $starts = @($lines | Select-String -Pattern 'Copy build of .*: \d+ copies').Count
    $ends = @($lines | Select-String -Pattern 'Copy build of .* ended at').Count
    if ($starts + $ends -gt $seenBuilds) { if (-not $tFirstBuild) { $tFirstBuild = Get-Date }; $lastBuild = Get-Date; $seenBuilds = $starts + $ends }   # a NEW build line (start or end)
    # Over: every started build ended and nothing new for 20 s (a single build may take a minute).
    if ($tDone -and $lastBuild -and $starts -eq $ends -and ((Get-Date) - $lastBuild).TotalSeconds -gt 20) { break }
    if ($tDone -and -not $lastBuild -and ((Get-Date) - $tDone).TotalSeconds -gt 40) { break }
}
"run time $([int]((Get-Date) - $t0).TotalSeconds) s"
$lines = Get-Content $log | Select-Object -Skip $before
$ended = @($lines | Select-String -Pattern 'Copy build of (.*) ended at (\d+)/(\d+) \((\d+) pipes from the cache\)')
$totalPipes = 0; $fromCache = 0
foreach ($m in $ended) { $totalPipes += [int]$m.Matches[0].Groups[3].Value; $fromCache += [int]$m.Matches[0].Groups[4].Value }
$span = if ($tFirstBuild -and $lastBuild) { [Math]::Round(($lastBuild - $tFirstBuild).TotalSeconds, 1) } else { 0 }
"builds: $($ended.Count) stops, $totalPipes pipes planned, $fromCache from the cache, build span $span s"
$lines | Select-String -Pattern 'cache|unresponsive|responsive again|resumed' | ForEach-Object { '  ' + $_.Line.Substring(11, [Math]::Min(140, $_.Line.Length - 11)) }
$ranStops = @($lines | Select-String -Pattern 'Copy build of (.*): \d+ copies' | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique)
"stops built: $($ranStops.Count) -> " + ($ranStops -join ', ')
$filesAfter = if (Test-Path $cacheDir) { (Get-ChildItem $cacheDir -File | Measure-Object).Count } else { 0 }
$mb = if (Test-Path $cacheDir) { [Math]::Round(((Get-ChildItem $cacheDir -File | Measure-Object Length -Sum).Sum) / 1MB, 0) } else { 0 }
"cache files before $filesBefore, after $filesAfter ($mb MB)"
Add-Type -Namespace Win -Name Msg2 -MemberDefinition '[DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr w, IntPtr l);'
$main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
if ($main) { [void][Win.Msg2]::PostMessage($main.Handle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) }   # a clean quit (settings saved)
for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500; $q = Get-OrganWindows $p.Id | Where-Object { $_.Title -match 'Quit LibreOrgan Sampler' } | Select-Object -First 1; if ($q) { Click-InWindow $q.Handle ([int]($q.W / 2)) ($q.H - 42) | Out-Null } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 3
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
