# Portable configurations: (1) loads a configuration and saves it as portable
# through ORGAN_SAVE_PORTABLE into -Output; checks that the file carries no
# audio devices, MIDI or machine settings. (2) Loads that portable file (with
# the settings' interface size set to 150 % first, which must survive) and
# screenshots the "Configuration from another computer" message and the main
# window (pt_notice.png, pt_main.png in %TEMP%). Settings backed up/restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
      [string]$Source = "F:\Suchy_rejstrik\Kobrle\Rural Organ.json",
      [string]$Output = "F:\JUCE_PROJEKTY\LibreOrgan\dist\RuralOrgan\Rural Organ\Rural Organ.json")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
function Wait-Free { $w = 0; while ((Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) -and $w -lt 20) { Start-Sleep -Seconds 1; $w++ }; if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 } }
Wait-Free
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_pt_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$log = "$appData\CrashLogs\activity.log"
function Set-Startup($configuration, $scale) {
    $json = [IO.File]::ReadAllText($file, [Text.Encoding]::UTF8) | ConvertFrom-Json
    $json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
    $json | Add-Member -NotePropertyName configurationFile -NotePropertyValue $configuration -Force
    $json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue $scale -Force
    [IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object Text.UTF8Encoding $false))
}
# (1) save portable
if (Test-Path $Output) { Remove-Item $Output -Force }
Set-Startup $Source 100
$before = (Get-Content $log).Count
$env:ORGAN_SAVE_PORTABLE = $Output
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$env:ORGAN_SAVE_PORTABLE = $null
$t0 = Get-Date
while (((Get-Date) - $t0).TotalSeconds -lt 45) { Start-Sleep -Seconds 1; $wins = @(Get-OrganWindows $p.Id); $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1; if ($alert) { Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42) | Out-Null }; if (Test-Path $Output) { Start-Sleep -Seconds 2; break } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 3
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Portable configuration|ORGAN_SAVE_PORTABLE' | ForEach-Object { $_.Line }
if (-not (Test-Path $Output)) { "NO PORTABLE FILE WRITTEN"; foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }; exit 1 }
$j = [IO.File]::ReadAllText($Output, [Text.Encoding]::UTF8) | ConvertFrom-Json
"portable flag: $($j.portable)  midi: $($j.containsMidiMappings)  trees: " + ($j.sampleTrees -join ' | ')
"machine keys present: " + ((@('windowX','uiScalePercent','renderThreadPreset','maxPolyphony','copyCacheFolder','sampleStorageFloat','configurationFile') | Where-Object { $null -ne $j.$_ }) -join ', ')
$i = 0; foreach ($m in $j.manualKeyboards) { "div $i audioXml=[$($m.audioDeviceStateXml)] ch=$($m.outputChannels -join ',') midi=[$($m.deviceIdentifier)]"; foreach ($r in $m.perspectiveAudio) { "   $($r.name): xml=[$([string]$r.audioDeviceStateXml -replace '\s+',' ')] ch=$($r.outputChannels -join ',') IR=[$($r.reverbIr)] IRrel=[$($r.reverbIrRelative)]" }; $i++ }
# (2) load the portable file with a 150 % interface: the scale must stay 150
Set-Startup $Output 150
$before = (Get-Content $log).Count
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$t0 = Get-Date; $noticeShot = $false; $mainShot = $false
while (((Get-Date) - $t0).TotalSeconds -lt 120)
{
    Start-Sleep -Seconds 1
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42) | Out-Null; continue }
    $notice = $wins | Where-Object { $_.Title -match 'another computer' } | Select-Object -First 1
    if ($notice -and -not $noticeShot) { Start-Sleep -Milliseconds 800; Save-WindowShot $notice.Handle "$env:TEMP\pt_notice.png" | Out-Null; $noticeShot = $true; "notice: $($notice.W)x$($notice.H)"; Click-InWindow $notice.Handle ([int]($notice.W / 2)) ($notice.H - 34) | Out-Null; continue }
    $main = $wins | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 800 } | Select-Object -First 1
    if ($main -and $noticeShot -and -not $mainShot) { Start-Sleep -Seconds 2; Save-WindowShot $main.Handle "$env:TEMP\pt_main.png" | Out-Null; $mainShot = $true; "main: $($main.Title)"; break }
}
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 3
$after = [IO.File]::ReadAllText($file, [Text.Encoding]::UTF8) | ConvertFrom-Json
"settings after load: uiScalePercent=$($after.uiScalePercent) renderThreadPreset=$($after.renderThreadPreset) maxPolyphony=$($after.maxPolyphony)"
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Portable configuration loaded|Audio device started|Windows Audio host opened|not on this computer' | Select-Object -First 12 | ForEach-Object { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) }
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }
"notice shot: $noticeShot, main shot: $mainShot"
