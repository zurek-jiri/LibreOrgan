# The Divisions dialog at a large interface size (<arg, default 160 %): with the
# Stralsund configuration, ORGAN_OPEN_KEYBOARDS=1 opens the Settings of manual I.
# The dialog must fit the screen and show scrollbars rather than hide its lower
# controls. Screenshots of the dialog; settings backed up and restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_ds_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$percent = if ($args.Count -ge 1) { [int]$args[0] } else { 160 }
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue $percent -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$env:ORGAN_OPEN_KEYBOARDS = "1"
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
$t0 = Get-Date
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$dialog = $null
while (((Get-Date) - $t0).TotalSeconds -lt 120)
{
    Start-Sleep -Milliseconds 500
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    $dialog = $wins | Where-Object { $_.Title -eq 'Divisions' -and $_.W -gt 300 } | Select-Object -First 1
    if ($dialog -and ((Get-Date) - $t0).TotalSeconds -gt 40) { Start-Sleep -Seconds 3; break }
}
if ($dialog)
{
    "dialog $($dialog.W)x$($dialog.H) at $($dialog.X),$($dialog.Y)  (screen work area 2560x1392)"
    Save-WindowShot $dialog.Handle "$env:TEMP\ds_dialog.png" | Out-Null
}
else { 'no Divisions dialog'; Get-OrganWindows $p.Id | ForEach-Object { "window: '$($_.Title)' $($_.W)x$($_.H)" } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
Remove-Item Env:ORGAN_OPEN_KEYBOARDS -ErrorAction SilentlyContinue
