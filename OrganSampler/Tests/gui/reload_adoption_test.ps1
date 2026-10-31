# A division loaded again must keep its samples and copies: starts with the
# Stralsund configuration and ORGAN_RELOAD_AFTER_START=<division> (default 1),
# which reloads that division 15 s after the stop list. Reports the log lines
# of the reload: "kept their samples", copy builds (there must be none for the
# unchanged stops) and the time the reload took. Settings backed up/restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan* -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$file = "$appData\settings.json"
Copy-Item $file "$env:TEMP\backup_ra_settings.json" -Force
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$division = if ($args.Count -ge 1) { $args[0] } else { '1' }
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
$env:ORGAN_RELOAD_AFTER_START = $division
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$env:ORGAN_RELOAD_AFTER_START = $null
# Wait for the first stop list, then for the reload's "Division(s) ... rebuilt" line.
$done = $false; $reloaded = $false
for ($i = 0; $i -lt 300 -and -not $reloaded; $i++)
{
    Start-Sleep -Seconds 1
    if ($p.HasExited) { 'APP EXITED'; break }
    $lines = Get-Content $log | Select-Object -Skip $before
    if (-not $done -and ($lines | Select-String -Pattern 'Compiling the Stop List: done' -Quiet)) { $done = $true; "stop list done at $i s" }
    if ($lines | Select-String -Pattern 'Division\(s\) .* rebuilt' -Quiet) { $reloaded = $true; "division rebuilt at $i s" }
}
Start-Sleep -Seconds 8
$lines = Get-Content $log | Select-Object -Skip $before
$start = $lines | Select-String -Pattern 'Reload after start' | Select-Object -First 1
$startIndex = if ($start) { $start.LineNumber } else { 0 }
$after = $lines | Select-Object -Skip $startIndex
'--- reload log'
$after | Select-String -Pattern 'Reload after start|Loading division|Committing|kept their samples|Division\(s\)|Copy build of .*: \d+ copies|Copy build of .* ended|done' | ForEach-Object { '  ' + $_.Line.Substring(11, [Math]::Min(150, $_.Line.Length - 11)) } | Select-Object -First 40
"copy builds after the reload: " + (($after | Select-String -Pattern 'Copy build of .*: \d+ copies' | Measure-Object).Count)
$main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
Add-Type -Namespace Win -Name MsgR -MemberDefinition '[DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr w, IntPtr l);'
if ($main) { [void][Win.MsgR]::PostMessage($main.Handle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) }
for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500; $q = Get-OrganWindows $p.Id | Where-Object { $_.Title -match 'Quit LibreOrgan Sampler|unsaved|Configuration' } | Select-Object -First 1; if ($q) { Click-InWindow $q.Handle ([int]($q.W / 2)) ($q.H - 42) | Out-Null } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 3
Copy-Item "$env:TEMP\backup_ra_settings.json" $file -Force
"settings restored identical: " + ((Get-FileHash $file).Hash -eq (Get-FileHash "$env:TEMP\backup_ra_settings.json").Hash)
