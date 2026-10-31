. "C:\Users\doma\AppData\Local\Temp\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "C:\Users\doma\AppData\Local\Temp\backup_mx_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
# The user's Stralsund configuration loads at start-up; the Settings dialog of manual 1 (UW) opens.
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
Remove-Item Env:ORGAN_RELOAD_DIVISION -ErrorAction SilentlyContinue
Remove-Item Env:ORGAN_EXPAND_VOICING -ErrorAction SilentlyContinue
Remove-Item Env:ORGAN_OPEN_STOP_BUILD -ErrorAction SilentlyContinue
$env:ORGAN_OPEN_KEYBOARDS = "1"
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$t0 = Get-Date
$p = Start-Process -FilePath "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe" -WorkingDirectory "C:\LibreOrgan" -PassThru
$main = $null; $dialog = $null
while (((Get-Date) - $t0).TotalSeconds -lt 120)
{
    Start-Sleep -Milliseconds 500
    $wins = @(Get-OrganWindows $p.Id)
    $main = $wins | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
    $dialog = $wins | Where-Object { $_.Title -match 'Division|Settings|Manual' -and $_.Title -notmatch '^LibreOrgan Sampler' } | Select-Object -First 1
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    $dialog = $wins | Where-Object { $_.Title -notmatch '^LibreOrgan Sampler' -and $_.W -gt 600 } | Select-Object -First 1
    if ($main -and $dialog -and ((Get-Date) - $t0).TotalSeconds -gt 40) { Start-Sleep -Seconds 3; break }
}
if ($main) { Save-WindowShot $main.Handle "C:\Users\doma\AppData\Local\Temp\mx_main.png" | Out-Null; "main shot $($main.W)x$($main.H)" }
if ($dialog) { Save-WindowShot $dialog.Handle "C:\Users\doma\AppData\Local\Temp\mx_dialog.png" | Out-Null; "dialog shot $($dialog.W)x$($dialog.H)" }
if ($main -and $dialog -and $args.Count -ge 6)
{
    # Switch the stop on (main window click), hold a key of the on-screen keyboard (dialog click) and watch the NOW counter.
    Click-InWindow $main.Handle ([int]$args[2]) ([int]$args[3]); Start-Sleep -Milliseconds 600
    Save-WindowShot $main.Handle "C:\Users\doma\AppData\Local\Temp\mx_stop_on.png" | Out-Null
    [void][W]::Activate($dialog.Handle); Start-Sleep -Milliseconds 400
    if ([W]::GetForegroundWindow() -eq $dialog.Handle)
    {
        $r = New-Object W+RECT; [void][W]::GetWindowRect($dialog.Handle, [ref]$r)
        [void][W]::SetCursorPos($r.L + [int]$args[4], $r.T + [int]$args[5]); Start-Sleep -Milliseconds 150
        [W]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 1500
        Save-WindowShot $main.Handle "C:\Users\doma\AppData\Local\Temp\mx_key_held_main.png" | Out-Null
        Save-WindowShot $dialog.Handle "C:\Users\doma\AppData\Local\Temp\mx_key_held_dialog.png" | Out-Null
        [W]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
        'key held and released'
    }
    else { 'dialog NOT IN FOREGROUND' }
    Start-Sleep -Seconds 2
}
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
'--- log ---'
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Audio levels|rebuilt|crash|velocity|intercepted|Settings|dialog' -CaseSensitive:$false | Select-Object -Last 8 | ForEach-Object { $_.Line.Substring(0, [Math]::Min(200, $_.Line.Length)) }
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
Remove-Item Env:ORGAN_OPEN_KEYBOARDS -ErrorAction SilentlyContinue
