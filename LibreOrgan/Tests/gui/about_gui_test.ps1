# Opens the Configuration Editor at start-up (ORGAN_OPEN_CONFIG_EDITOR=1), clicks
# "About & licences..." in its top bar, screenshots the licences dialog with the
# first and the fourth entry selected, then quits. Settings are backed up and restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_about_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$env:ORGAN_OPEN_CONFIG_EDITOR = "1"
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$exe = if ($args.Count -ge 1) { $args[0] } else { "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe" }
$t0 = Get-Date
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$editor = $null
while (((Get-Date) - $t0).TotalSeconds -lt 60)
{
    Start-Sleep -Milliseconds 500
    $alert = Get-OrganWindows $p.Id | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)); Start-Sleep -Milliseconds 800 }
    $editor = Get-OrganWindows $p.Id | Where-Object { $_.Title -eq 'Configuration Editor' -and $_.W -gt 300 } | Select-Object -First 1
    if ($editor -and ((Get-Date) - $t0).TotalSeconds -gt 12) { break }
}
if ($editor)
{
    "editor $($editor.W)x$($editor.H)"
    Start-Sleep -Seconds 2
    Save-WindowShot $editor.Handle "$env:TEMP\about_0.png" | Out-Null; 'shot 0 (editor top bar)'
    $dialog = $null
    foreach ($y in 56, 50, 62)
    {
        'click: ' + (Click-InWindow $editor.Handle ($editor.W - 60) $y)
        Start-Sleep -Milliseconds 1500
        $dialog = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^About LibreOrgan Sampler' } | Select-Object -First 1
        if ($dialog) { break }
    }
    if ($dialog)
    {
        "dialog '$($dialog.Title)' $($dialog.W)x$($dialog.H)"
        Save-WindowShot $dialog.Handle "$env:TEMP\about_1.png" | Out-Null; 'shot 1 (first entry)'
        # Fourth list row (row height ~22 px, list starts 8 px below the title bar).
        'row click: ' + (Click-InWindow $dialog.Handle 100 (31 + 8 + 3 * 22 + 11))
        Start-Sleep -Milliseconds 800
        Save-WindowShot $dialog.Handle "$env:TEMP\about_2.png" | Out-Null; 'shot 2 (fourth entry)'
        'last row click: ' + (Click-InWindow $dialog.Handle 100 (31 + 8 + 6 * 22 + 11))
        Start-Sleep -Milliseconds 800
        Save-WindowShot $dialog.Handle "$env:TEMP\about_3.png" | Out-Null; 'shot 3 (seventh entry)'
        [W]::keybd_event(0x1B, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60; [W]::keybd_event(0x1B, 0, 2, [UIntPtr]::Zero)
        Start-Sleep -Milliseconds 800
        $still = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^About LibreOrgan Sampler' } | Select-Object -First 1
        'dialog closed by Escape: ' + ($null -eq $still)
    }
    else { 'NO LICENCES DIALOG'; Get-OrganWindows $p.Id | ForEach-Object { "window: '$($_.Title)' $($_.W)x$($_.H)" } }
}
else { 'no configuration editor'; Get-OrganWindows $p.Id | ForEach-Object { "window: '$($_.Title)' $($_.W)x$($_.H)" } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
'--- log ---'
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'crash|exception|licen' -CaseSensitive:$false | ForEach-Object { $_.Line.Substring(0, [Math]::Min(160, $_.Line.Length)) }
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
Remove-Item Env:ORGAN_OPEN_CONFIG_EDITOR -ErrorAction SilentlyContinue
