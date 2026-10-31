# Configuration Editor on the second screen: moves the editor window there, drags
# its bottom-right corner with the mouse (+240, +160 px) and reports the window
# rectangle before and after; then does the same on the primary screen.
# Argument 1: uiScalePercent to start with (default: none = the estimate).
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_ss_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
if ($args.Count -ge 1) { $json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue ([int]$args[0]) -Force }
elseif ($json.PSObject.Properties['uiScalePercent']) { $json.PSObject.Properties.Remove('uiScalePercent') }
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$env:ORGAN_OPEN_CONFIG_EDITOR = "1"
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
$t0 = Get-Date
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$editor = $null
while (((Get-Date) - $t0).TotalSeconds -lt 60)
{
    Start-Sleep -Milliseconds 500
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)); Start-Sleep -Milliseconds 800 }
    $editor = $wins | Where-Object { $_.Title -eq 'Configuration Editor' -and $_.W -gt 300 } | Select-Object -First 1
    if ($editor -and ((Get-Date) - $t0).TotalSeconds -gt 12) { break }
}
if ($editor)
{
    Add-Type -Namespace Win -Name Move -MemberDefinition '[DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hwnd, IntPtr after, int x, int y, int cx, int cy, uint flags);'
    function Drag-Corner($h, $tag)
    {
        [void][W]::Activate($h); Start-Sleep -Milliseconds 400
        $r = New-Object W+RECT; [void][W]::GetWindowRect($h, [ref]$r)
        "$tag before: $($r.L),$($r.T) $($r.R - $r.L)x$($r.B - $r.T)"
        [void][W]::SetCursorPos($r.R - 3, $r.B - 3); Start-Sleep -Milliseconds 200
        [W]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 150
        for ($i = 1; $i -le 12; $i++) { [void][W]::SetCursorPos($r.R - 3 + 20 * $i, $r.B - 3 + [int](13.33 * $i)); Start-Sleep -Milliseconds 40 }
        Start-Sleep -Milliseconds 200
        [W]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 600
        $r2 = New-Object W+RECT; [void][W]::GetWindowRect($h, [ref]$r2)
        "$tag after : $($r2.L),$($r2.T) $($r2.R - $r2.L)x$($r2.B - $r2.T)   (expected about +240 x +160)"
    }
    # Second screen (physical 2560,375 .. 4480,1455): place the editor at 2700,500.
    [void][Win.Move]::SetWindowPos($editor.Handle, [IntPtr]::Zero, 2700, 500, 0, 0, 0x0001 -bor 0x0004); Start-Sleep -Milliseconds 800
    Drag-Corner $editor.Handle 'second screen'
    Save-WindowShot $editor.Handle "$env:TEMP\ss_second.png" | Out-Null
    [void][Win.Move]::SetWindowPos($editor.Handle, [IntPtr]::Zero, 300, 200, 0, 0, 0x0001 -bor 0x0004); Start-Sleep -Milliseconds 800
    Drag-Corner $editor.Handle 'primary screen'
}
else { 'no configuration editor'; Get-OrganWindows $p.Id | ForEach-Object { "window: '$($_.Title)' $($_.W)x$($_.H)" } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
Remove-Item Env:ORGAN_OPEN_CONFIG_EDITOR -ErrorAction SilentlyContinue
