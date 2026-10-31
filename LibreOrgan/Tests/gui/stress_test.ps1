# In-app polyphony stress: loads the Stralsund configuration on the real audio
# devices, engages every stop and holds every key of every division (ORGAN_STRESS),
# then prints the "STRESS" lines of the activity log (polyphony, render load).
#   stress_test.ps1 [-Seconds 8] [-Preset physical|allMinusOne|''] [-Limit 100000]
# -Preset / -Limit are injected into settings.json for the run (the configuration
# may carry its own values, which win when it is loaded). Settings backed up/restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
      [int]$Seconds = 8, [string]$Preset = "", [int]$Limit = 0, [switch]$NoIr, [switch]$Bash, [int]$Threads = 0, [switch]$NoAux, [switch]$NoAuxDirect, [int]$AfterSeconds = 2)
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
$waited = 0
while ((Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) -and $waited -lt 20) { Start-Sleep -Seconds 1; $waited++ }   # a previous run still closing
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_st_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
if ($Preset -ne "") { $json | Add-Member -NotePropertyName renderThreadPreset -NotePropertyValue $Preset -Force; $json | Add-Member -NotePropertyName renderThreads -NotePropertyValue 0 -Force }
if ($Limit -gt 0) { $env:ORGAN_STRESS_LIMIT = "$Limit" } else { $env:ORGAN_STRESS_LIMIT = $null }   # the loaded configuration's limit would win in settings.json
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$log = "$appData\CrashLogs\activity.log"
$before = if (Test-Path $log) { (Get-Content $log).Count } else { 0 }
$env:ORGAN_STRESS = "$Seconds"
if ($NoIr) { $env:ORGAN_STRESS_NO_IR = "1" } else { $env:ORGAN_STRESS_NO_IR = $null }
if ($Bash) { $env:ORGAN_STRESS_BASH = "1" } else { $env:ORGAN_STRESS_BASH = $null }
if ($NoAuxDirect) { $env:ORGAN_STRESS_NO_AUX = "2" } elseif ($NoAux) { $env:ORGAN_STRESS_NO_AUX = "1" } else { $env:ORGAN_STRESS_NO_AUX = $null }
if ($Threads -gt 0) { $env:ORGAN_STRESS_THREADS = "$Threads" } else { $env:ORGAN_STRESS_THREADS = $null }
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$done = $false
while (((Get-Date) - $t0).TotalSeconds -lt 180 -and -not $done)
{
    Start-Sleep -Milliseconds 1000
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    if ((Test-Path $log) -and ((Get-Content $log | Select-Object -Skip $before) -match 'STRESS done')) { $done = $true }
}
Start-Sleep -Seconds $AfterSeconds   # longer to watch what the app does after the keys are released (device watcher etc.)
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
# The auxiliary state of the pedal as the run left it in settings.json (must be none after -NoAux / -NoAuxDirect).
try { $saved = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json; foreach ($p in $saved.manualKeyboards[0].perspectiveAudio) { if ($p.name -eq 'auxiliary') { $x = [string]$p.audioDeviceStateXml; $dev = if ($x -match 'audioOutputDeviceName="([^"]*)"') { $Matches[1] } else { '(empty xml)' }; "pedal auxiliary saved as: [$dev]" } } } catch { "saved state unreadable: $_" }
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }
$env:ORGAN_STRESS = $null; $env:ORGAN_STRESS_LIMIT = $null; $env:ORGAN_STRESS_NO_IR = $null; $env:ORGAN_STRESS_BASH = $null; $env:ORGAN_STRESS_THREADS = $null; $env:ORGAN_STRESS_NO_AUX = $null
"--- limit $Limit noIr=$NoIr bash=$Bash threads=$Threads noAux=$NoAux noAuxDirect=$NoAuxDirect"
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'STRESS|Render threads|render threads|unresponsive|gap|reopening|available again|Applying|auxiliary' | ForEach-Object { $_.Line }
