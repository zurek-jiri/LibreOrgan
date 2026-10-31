# Runs the polyphony measurement ("Measure the maximum polyphony") on the real
# Stralsund configuration via ORGAN_MEASURE_POLYPHONY=1, waits for its end,
# screenshots the report window, prints the measurement lines of the log and
# restores the user's settings. Usage: measure_test.ps1 [-Exe path]
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
      [string]$Configuration = "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
$waited = 0
while ((Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) -and $waited -lt 20) { Start-Sleep -Seconds 1; $waited++ }
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_mt_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue $Configuration -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$log = "$appData\CrashLogs\activity.log"
$before = if (Test-Path $log) { (Get-Content $log).Count } else { 0 }
$env:ORGAN_MEASURE_POLYPHONY = "1"
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$done = $false; $shotProgress = $false; $shotProgress2 = $false; $progressSeen = Get-Date
while (((Get-Date) - $t0).TotalSeconds -lt 300 -and -not $done)
{
    Start-Sleep -Milliseconds 1000
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    $progress = $wins | Where-Object { $_.Title -match 'Measuring the maximum polyphony' } | Select-Object -First 1
    if ($progress -and -not $shotProgress) { Start-Sleep -Milliseconds 1500; Save-WindowShot $progress.Handle "$env:TEMP\measure_progress.png" | Out-Null; $shotProgress = $true; $progressSeen = Get-Date }
    # A second shot once the run is under way (the summary and the target lines).
    if ($progress -and $shotProgress -and -not $shotProgress2 -and ((Get-Date) - $progressSeen).TotalSeconds -ge 14) { Save-WindowShot $progress.Handle "$env:TEMP\measure_progress2.png" | Out-Null; $shotProgress2 = $true }
    $report = $wins | Where-Object { $_.Title -match 'Maximum polyphony measured' } | Select-Object -First 1
    if ($report)
    {
        Start-Sleep -Milliseconds 800; Save-WindowShot $report.Handle "$env:TEMP\measure_report.png" | Out-Null
        # The end of the report (the advice): click into the text, Ctrl+End, shoot again.
        Click-InWindow $report.Handle 200 120 | Out-Null; Start-Sleep -Milliseconds 300
        [W]::keybd_event(0x11, 0, 0, [UIntPtr]::Zero); [W]::keybd_event(0x23, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60
        [W]::keybd_event(0x23, 0, 2, [UIntPtr]::Zero); [W]::keybd_event(0x11, 0, 2, [UIntPtr]::Zero); Start-Sleep -Milliseconds 600
        Save-WindowShot $report.Handle "$env:TEMP\measure_report_end.png" | Out-Null
        $done = $true
    }
}
"took $([int]((Get-Date) - $t0).TotalSeconds) s, report window: $done"
Start-Sleep -Seconds 1
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
$after = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
"settings.json maxPolyphony after the run: " + $after.maxPolyphony
"settings.json divisions saved: " + $after.manualKeyboards.Count + " (manualCount " + $after.manualCount + ")"
$i = 0; foreach ($m in $after.manualKeyboards) { $x = [string]$m.audioDeviceStateXml; $dev = if ($x -match 'audioOutputDeviceName="([^"]*)"') { $Matches[1] } else { '(no state)' }; "  division $i own device [$dev], routes: " + (($m.perspectiveAudio | ForEach-Object { $_.name + $(if ($_.reverbIr) { '(IR!)' } else { '' }) }) -join ', '); $i++ }
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }
$env:ORGAN_MEASURE_POLYPHONY = $null
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Polyphony measurement|gap' | ForEach-Object { $_.Line }
