. "C:\Users\doma\AppData\Local\Temp\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "C:\Users\doma\AppData\Local\Temp\backup_vg_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
# Manual II loads the synthetic velocity-layer tree the old way (no table): folderPath only.
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$tree = "$env:TEMP\vel_test"
$json.manualKeyboards[2].folderPath = $tree
if ($json.manualKeyboards[2].PSObject.Properties['extraFolderPaths']) { $json.manualKeyboards[2].extraFolderPaths = @() }
$pool = @($json.sampleTrees); if ($pool -notcontains $tree) { $json.sampleTrees = @($pool + $tree); $pool = @($json.sampleTrees) }
$treeIndex = [Array]::IndexOf($pool, $tree)
$table = '{"stopBuild": {"active": true, "perspectives": ["default"], "entries": [{"name": "Principal8", "alias": "", "customName": false, "child": false, "folders": [{"perspective": "default", "tree": ' + $treeIndex + ', "path": "Principal8"}]}]}}'
$json.manualKeyboards[2] | Add-Member -NotePropertyName stopBuild -NotePropertyValue (($table | ConvertFrom-Json).stopBuild) -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$env:ORGAN_RELOAD_DIVISION = "2"
$env:ORGAN_EXPAND_VOICING = "Principal"
Remove-Item Env:ORGAN_OPEN_STOP_BUILD -ErrorAction SilentlyContinue
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$t0 = Get-Date
$p = Start-Process -FilePath "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe" -WorkingDirectory "C:\LibreOrgan" -PassThru
$main = $null
while (((Get-Date) - $t0).TotalSeconds -lt 40)
{
    Start-Sleep -Milliseconds 500
    $main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
    if ($main -and ((Get-Date) - $t0).TotalSeconds -gt 14) { break }
}
if ($main)
{
    "main $($main.W)x$($main.H)"
    for ($w = 0; $w -lt 20; $w++)
    {
        $alert = Get-OrganWindows $p.Id | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
        if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)); Start-Sleep -Milliseconds 800; break }
        Start-Sleep -Milliseconds 500
    }
    Save-WindowShot $main.Handle "C:\Users\doma\AppData\Local\Temp\vg_0.png" | Out-Null; 'shot 0 (voicing open)'
    if ($args.Count -ge 2)
    {
        Click-InWindow $main.Handle ([int]$args[0]) ([int]$args[1]); Start-Sleep -Milliseconds 800
        Save-WindowShot $main.Handle "C:\Users\doma\AppData\Local\Temp\vg_1.png" | Out-Null; 'shot 1 (after the click on a layer box)'
    }
    if ($args.Count -ge 6)
    {
        # Stop List (header button), then "Unload folder..." -> the menu with "Unload all folder trees".
        Click-InWindow $main.Handle ([int]$args[2]) ([int]$args[3]); Start-Sleep -Seconds 3
        $dialog = Get-OrganWindows $p.Id | Where-Object { $_.Title -match 'Stop List' } | Select-Object -First 1
        if ($dialog)
        {
            Click-InWindow $dialog.Handle ([int]$args[4]) ([int]$args[5]); Start-Sleep -Milliseconds 1000
            Get-OrganWindows $p.Id | ForEach-Object { "window: '$($_.Title)' $($_.W)x$($_.H)" }
            $menu = Get-OrganWindows $p.Id | Where-Object { $_.Title -notmatch 'LibreOrgan Sampler|Stop List' -and $_.W -gt 150 } | Select-Object -First 1
            if ($menu) { Save-WindowShot $menu.Handle "C:\Users\doma\AppData\Local\Temp\vg_menu.png" | Out-Null; 'menu shot' }
            [W]::keybd_event(0x1B, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60; [W]::keybd_event(0x1B, 0, 2, [UIntPtr]::Zero)
        }
        else { 'no stop list' }
    }
}
else { 'no main window' }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
'--- log ---'
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'rebuilt|Loading|crash|velocity' -CaseSensitive:$false | ForEach-Object { $_.Line.Substring(0, [Math]::Min(160, $_.Line.Length)) }
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
Remove-Item Env:ORGAN_RELOAD_DIVISION -ErrorAction SilentlyContinue
Remove-Item Env:ORGAN_EXPAND_VOICING -ErrorAction SilentlyContinue

