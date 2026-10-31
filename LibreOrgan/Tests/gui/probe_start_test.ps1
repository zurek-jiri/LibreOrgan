# Watches a start with the Stralsund configuration at an interface size (<arg1>,
# default 160) and ORGAN_OPEN_KEYBOARDS=1: process alive, windows, log, crash files.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$file = "$appData\settings.json"
Copy-Item $file "$env:TEMP\backup_probe_settings.json" -Force
$percent = if ($args.Count -ge 1) { [int]$args[0] } else { 160 }
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue $percent -Force
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$d = "$appData\CrashLogs"
$before = (Get-Content "$d\activity.log").Count
$env:ORGAN_OPEN_KEYBOARDS = "1"
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
$t0 = Get-Date
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
for ($i = 0; $i -lt 45; $i++)
{
    Start-Sleep -Seconds 2
    $w = @(Get-OrganWindows $p.Id)
    $line = "$([int]((Get-Date) - $t0).TotalSeconds) s: exited=$($p.HasExited) windows=" + (($w | ForEach-Object { "'$($_.Title)' $($_.W)x$($_.H) at $($_.X),$($_.Y)" }) -join ' | ')
    if ($i % 4 -eq 0 -or $p.HasExited) { $line }
    $dialog = $w | Where-Object { $_.Title -eq 'Divisions' } | Select-Object -First 1
    if ($dialog) { Start-Sleep -Seconds 2; Save-WindowShot $dialog.Handle "$env:TEMP\ds_dialog.png" | Out-Null; "dialog shot $($dialog.W)x$($dialog.H) at $($dialog.X),$($dialog.Y)"; break }
    if ($p.HasExited) { "exit code $($p.ExitCode)"; break }
}
$env:ORGAN_OPEN_KEYBOARDS = $null
Get-Content "$d\activity.log" | Select-Object -Skip $before | Where-Object { $_ -notmatch 'Tuning |Audio levels' } | Select-Object -Last 12 | ForEach-Object { $_.Substring(0, [Math]::Min(170, $_.Length)) }
Get-ChildItem "$d\crash-*.txt" | Where-Object { $_.LastWriteTime -gt (Get-Date).AddMinutes(-3) } | ForEach-Object { 'CRASH FILE ' + $_.Name; Get-Content $_.FullName -TotalCount 50 }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
Copy-Item "$env:TEMP\backup_probe_settings.json" $file -Force
'settings restored: ' + ((Get-FileHash $file).Hash -eq (Get-FileHash "$env:TEMP\backup_probe_settings.json").Hash)
