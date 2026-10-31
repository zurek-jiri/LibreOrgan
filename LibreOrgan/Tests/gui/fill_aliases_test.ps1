# Fills the aliases of a configuration from the folder names (ORGAN_FILL_ALIASES_SAVE,
# the same as the Stop List's "Aliases from folders") and saves the file back.
# A backup <file>.bak-before-aliases is written first. Then a screenshot of the
# main window (fa_main.png in %TEMP%) shows the result. Settings backed up/restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
      [string]$Configuration = "F:\Suchy_rejstrik\Kobrle\Organ configuration.json")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
$waited = 0
while ((Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) -and $waited -lt 20) { Start-Sleep -Seconds 1; $waited++ }
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$backup = "$Configuration.bak-before-aliases"
if (-not (Test-Path $backup)) { Copy-Item $Configuration $backup }
"backup: $backup"
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_fa_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [IO.File]::ReadAllText($file, [Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue $Configuration -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue 100 -Force
[IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object Text.UTF8Encoding $false))
$log = "$appData\CrashLogs\activity.log"
$before = if (Test-Path $log) { (Get-Content $log).Count } else { 0 }
$env:ORGAN_FILL_ALIASES_SAVE = "1"
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$env:ORGAN_FILL_ALIASES_SAVE = $null
$shot = $false
while (((Get-Date) - $t0).TotalSeconds -lt 80)
{
    Start-Sleep -Milliseconds 1000
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)); continue }
    $main = $wins | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 800 } | Select-Object -First 1
    if ($main -and -not $shot -and ((Get-Date) - $t0).TotalSeconds -ge 40) { Save-WindowShot $main.Handle "$env:TEMP\fa_main.png" | Out-Null; $shot = $true; "shot: $($main.Title)"; break }
}
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Aliases from folders|ORGAN_FILL_ALIASES_SAVE' | ForEach-Object { $_.Line }
$j = [IO.File]::ReadAllText($Configuration, [Text.Encoding]::UTF8) | ConvertFrom-Json
"aliases now: " + (($j.manualKeyboards | ForEach-Object { $_.stopBuild.entries } | Where-Object { $_.alias } | ForEach-Object { $_.alias }) -join ' | ')
