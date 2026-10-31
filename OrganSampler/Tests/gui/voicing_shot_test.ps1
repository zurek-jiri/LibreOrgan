# Screenshot of the main window with one stop's voicing expanded: loads the
# Stralsund configuration, expands the voicing of the stop named by -Stop
# (ORGAN_EXPAND_VOICING) and saves vs_<tag>_main.png. Settings backed up/restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
      [string]$Stop = "Trompete", [string]$Tag = "new")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_vs_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue 100 -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$env:ORGAN_EXPAND_VOICING = $Stop
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
while (((Get-Date) - $t0).TotalSeconds -lt 70)
{
    Start-Sleep -Milliseconds 700
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
}
$main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
if ($main) { Save-WindowShot $main.Handle ("$env:TEMP\vs_{0}_main.png" -f $Tag) | Out-Null; "main: $($main.W)x$($main.H)" } else { "NO MAIN WINDOW" }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
$env:ORGAN_EXPAND_VOICING = $null
