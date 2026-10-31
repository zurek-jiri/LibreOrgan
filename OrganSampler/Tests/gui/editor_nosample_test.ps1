# Opens the Configuration Editor with NO sample folders loaded and screenshots it
# (%TEMP%\editor_nosample.png): the "Measure the maximum polyphony" button must
# be greyed out. Settings backed up and restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
$waited = 0
while ((Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) -and $waited -lt 20) { Start-Sleep -Seconds 1; $waited++ }
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_en_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $false -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$env:ORGAN_OPEN_CONFIG_EDITOR = "1"
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$shot = $false
while (((Get-Date) - $t0).TotalSeconds -lt 60 -and -not $shot)
{
    Start-Sleep -Milliseconds 1000
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    $editor = $wins | Where-Object { $_.Title -match 'Configuration Editor' } | Select-Object -First 1
    if ($editor -and ((Get-Date) - $t0).TotalSeconds -gt 12) { Save-WindowShot $editor.Handle "$env:TEMP\editor_nosample.png" | Out-Null; $shot = $true }
}
"screenshot: $shot"
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }
$env:ORGAN_OPEN_CONFIG_EDITOR = $null
