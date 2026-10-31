# Dumps every option of the Configuration Editor (section, label, kind, tooltip)
# to %TEMP%\organ_options.txt through ORGAN_DUMP_OPTIONS - the list the user
# manual's reference chapters are checked against. Loads the Stralsund
# configuration so that the division sections are complete. Settings backed
# up and restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
      [string]$Out = "$env:TEMP\organ_options.txt")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_do_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
if (Test-Path $Out) { [IO.File]::Delete($Out) }
$env:ORGAN_OPEN_CONFIG_EDITOR = "1"
$env:ORGAN_DUMP_OPTIONS = $Out
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$seen = $false
while (((Get-Date) - $t0).TotalSeconds -lt 150)
{
    Start-Sleep -Milliseconds 700
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    if (((Get-Date) - $t0).TotalSeconds -lt 45) { continue }
    $editor = $wins | Where-Object { $_.Title -match '^Configuration Editor' -and $_.W -gt 300 } | Select-Object -First 1
    if ($editor -and -not $seen) { $seen = $true; "editor open at $([int]((Get-Date) - $t0).TotalSeconds) s"; Start-Sleep -Seconds 3; break }
}
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
$env:ORGAN_OPEN_CONFIG_EDITOR = $null; $env:ORGAN_DUMP_OPTIONS = $null
if (Test-Path $Out) { "options dumped: $((Get-Content $Out).Count) lines" } else { "NO DUMP" }
