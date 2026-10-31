# Stralsund configuration at start-up, then the voicing of the stop named in
# ORGAN_EXPAND_VOICING (default "UW Mixtur") is expanded and the main window is
# screenshotted: the "Velocity layers:" row must show the _070 / _128 boxes.
# Optional argument: another stop name part. Settings are backed up and restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_mv_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
Remove-Item Env:ORGAN_RELOAD_DIVISION -ErrorAction SilentlyContinue
Remove-Item Env:ORGAN_OPEN_STOP_BUILD -ErrorAction SilentlyContinue
Remove-Item Env:ORGAN_OPEN_KEYBOARDS -ErrorAction SilentlyContinue
$env:ORGAN_EXPAND_VOICING = if ($args.Count -ge 1) { $args[0] } else { "UW Mixtur" }
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$t0 = Get-Date
$p = Start-Process -FilePath "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe" -WorkingDirectory "C:\LibreOrgan" -PassThru
"process $($p.Id)"
$main = $null
while (((Get-Date) - $t0).TotalSeconds -lt 110)
{
    Start-Sleep -Milliseconds 500
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    $main = $wins | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
    if ($main -and ((Get-Date) - $t0).TotalSeconds -gt 80) { Start-Sleep -Seconds 3; break }
}
if ($main) { Save-WindowShot $main.Handle "$env:TEMP\mv_main.png" | Out-Null; "main shot $($main.W)x$($main.H)" }
else { 'no main window; process alive: ' + (-not $p.HasExited); Get-OrganWindows $p.Id | ForEach-Object { "window: '$($_.Title)' $($_.W)x$($_.H)" } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
'--- log ---'
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Crash handler|rebuilt|crash|velocity|voicing' -CaseSensitive:$false | Select-Object -Last 8 | ForEach-Object { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) }
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
Remove-Item Env:ORGAN_EXPAND_VOICING -ErrorAction SilentlyContinue
