# Popup menu and alert box at an interface size other than 100 %: starts with
# uiScalePercent = <arg, default 130>, right-clicks the "Stop List" top-bar button
# (its MIDI menu) and screenshots the menu window, then presses Escape.
# Settings are backed up and restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_sm_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$percent = if ($args.Count -ge 1) { [int]$args[0] } else { 130 }
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue $percent -Force
$json | Add-Member -NotePropertyName windowMaximised -NotePropertyValue $false -Force
$json | Add-Member -NotePropertyName windowX -NotePropertyValue 100 -Force
$json | Add-Member -NotePropertyName windowY -NotePropertyValue 60 -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
$t0 = Get-Date
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$main = $null
while (((Get-Date) - $t0).TotalSeconds -lt 40)
{
    Start-Sleep -Milliseconds 500
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    $main = $wins | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
    if ($main -and ((Get-Date) - $t0).TotalSeconds -gt 12) { Start-Sleep -Seconds 2; break }
}
if ($main)
{
    "main $($main.W)x$($main.H) at $($main.X),$($main.Y)"
    Save-WindowShot $main.Handle "$env:TEMP\sm_main.png" | Out-Null
    # "Stop List" button: at 100 % its centre is at client (403, 36); the title bar (31 px) is not scaled.
    $f = $percent / 100.0
    $x = 8 + [int](403 * $f); $y = 31 + [int](36 * $f)
    [void][W]::Activate($main.Handle); Start-Sleep -Milliseconds 400
    $r = New-Object W+RECT; [void][W]::GetWindowRect($main.Handle, [ref]$r)
    [void][W]::SetCursorPos($r.L + $x, $r.T + $y); Start-Sleep -Milliseconds 200
    [W]::mouse_event(0x0008, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 80; [W]::mouse_event(0x0010, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 1200
    $menu = Get-OrganWindows $p.Id | Where-Object { $_.Title -notmatch '^LibreOrgan Sampler' -and $_.W -gt 100 -and $_.H -gt 30 } | Select-Object -First 1
    if ($menu) { "menu window $($menu.W)x$($menu.H) at $($menu.X),$($menu.Y)"; Save-WindowShot $menu.Handle "$env:TEMP\sm_menu.png" | Out-Null }
    else { 'no menu window'; Get-OrganWindows $p.Id | ForEach-Object { "window: '$($_.Title)' $($_.W)x$($_.H)" } }
    [W]::keybd_event(0x1B, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60; [W]::keybd_event(0x1B, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 500
}
else { 'no main window' }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
