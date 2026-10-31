# The user's way: start empty (loadLastConfigurationAtStartup false), click
# "Load Configuration" in the top bar, type the Stralsund file into the native
# file dialog, then record what is on the SCREEN while the stop list compiles
# (lc_<n>.png every 200 ms, screen captures of the loading dialog's area, plus the
# dialog's PrintWindow shots lcw_<n>.png). Settings backed up and restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_lc_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $false -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue 100 -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
Get-ChildItem "$env:TEMP\lc_*.png", "$env:TEMP\lcw_*.png" -ErrorAction SilentlyContinue | ForEach-Object { [IO.File]::Delete($_.FullName) }
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
# The configuration file is handed to the app (ORGAN_LOAD_CONFIGURATION): the
# "Load Configuration" button then skips the native file dialog - typing a path
# into it fails on a Czech keyboard layout (the test used to wait for a hand).
$configPath = 'F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json'
$t0 = Get-Date
$env:ORGAN_LOAD_CONFIGURATION = $configPath
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$env:ORGAN_LOAD_CONFIGURATION = $null
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
if (-not $main) { 'no main window'; Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false; exit 1 }
"main $($main.W)x$($main.H) at $($main.X),$($main.Y)"
# "Load Configuration": its centre in the top bar of the maximised window (from the screenshots).
$x = [int](1792 * $main.W / 2559.0)
'click Load Configuration: ' + (Click-InWindow $main.Handle $x 67)
# No native file dialog: the app takes the file from the environment and must
# start loading within a few seconds. If it does not, the test fails here
# (nobody must load the file by hand for it).
$taken = $false
for ($i = 0; $i -lt 30 -and -not $taken; $i++)
{
    Start-Sleep -Milliseconds 300
    if (Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'file taken from ORGAN_LOAD_CONFIGURATION' -Quiet) { $taken = $true }
}
if (-not $taken)
{
    'CONFIGURATION NOT TAKEN - the Load Configuration click did not reach the app'
    Get-OrganWindows $p.Id | ForEach-Object { "window: '$($_.Title)' $($_.W)x$($_.H)" }
    Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
    Start-Sleep -Seconds 2
    foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }
    exit 1
}
'configuration file taken by the app'
$tLoad = Get-Date
$shots = 0; $committing = $false; $seenLoader = $false; $tCommit = $null
while (((Get-Date) - $tLoad).TotalSeconds -lt 150)
{
    Start-Sleep -Milliseconds 200
    $wins = @(Get-OrganWindows $p.Id)
    $question = $wins | Where-Object { $_.Title -match 'Load configuration|Quit LibreOrgan Sampler|unsaved' } | Select-Object -First 1
    $loader = $wins | Where-Object { $_.Title -match 'Loading saved divisions' } | Select-Object -First 1
    if ($loader) { $seenLoader = $true }
    if (-not $committing -and (Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Committing' -Quiet)) { $committing = $true; $tCommit = Get-Date; "committing at $([int]((Get-Date) - $tLoad).TotalSeconds) s after the file was chosen" }
    if ($committing -and $loader -and $shots -lt 60)
    {
        $shots++
        $bmp = New-Object System.Drawing.Bitmap $loader.W, $loader.H
        $g = [System.Drawing.Graphics]::FromImage($bmp); $g.CopyFromScreen($loader.X, $loader.Y, 0, 0, $bmp.Size); $g.Dispose()
        $bmp.Save(("$env:TEMP\lc_{0:00}.png" -f $shots)); $bmp.Dispose()
        Save-WindowShot $loader.Handle ("$env:TEMP\lcw_{0:00}.png" -f $shots) | Out-Null
    }
    if ($committing -and -not $loader -and $seenLoader) { "loader gone after $([Math]::Round(((Get-Date) - $tCommit).TotalSeconds, 1)) s of compiling, $shots shots"; break }
}
Start-Sleep -Seconds 2
Get-OrganWindows $p.Id | ForEach-Object { "window at the end: '$($_.Title)' $($_.W)x$($_.H)" }
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Compiling|Committing|Loading|ERROR|missing|alert' | ForEach-Object { '  ' + $_.Line.Substring(0, [Math]::Min(150, $_.Line.Length)) }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
