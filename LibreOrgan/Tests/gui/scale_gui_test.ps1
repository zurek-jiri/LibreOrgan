# Interface size: starts the program with uiScalePercent = <arg, default 130> in the
# settings (the main window shot must show everything larger), then once more with the
# key removed (first start: the estimate, 100 % on a 2560 x 1440 screen) and prints the
# "Interface size" line of activity.log. Settings are backed up and restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_sc_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$log = "$appData\CrashLogs\activity.log"
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
$percent = if ($args.Count -ge 1) { [int]$args[0] } else { 130 }
foreach ($pass in 'fixed', 'estimate')
{
    $json = [System.IO.File]::ReadAllText($backups['settings.json'], [System.Text.Encoding]::UTF8) | ConvertFrom-Json
    if ($pass -eq 'fixed') { $json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue $percent -Force }
    elseif ($json.PSObject.Properties['uiScalePercent']) { $json.PSObject.Properties.Remove('uiScalePercent') }
    [System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
    $before = (Get-Content $log).Count
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
    if ($main) { Save-WindowShot $main.Handle "$env:TEMP\sc_$pass.png" | Out-Null; "$pass : main $($main.W)x$($main.H) at $($main.X),$($main.Y)" } else { "$pass : no main window" }
    Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
    Start-Sleep -Seconds 2
    Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Interface size|crash' -CaseSensitive:$false | ForEach-Object { '  ' + $_.Line }
    $saved = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
    '  saved uiScalePercent: ' + $saved.uiScalePercent
}
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
