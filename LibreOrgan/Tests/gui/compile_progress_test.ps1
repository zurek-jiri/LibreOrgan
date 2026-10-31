# The loading dialog during "Compiling the Stop List": with the Stralsund
# configuration at start-up, screenshots of the "Loading saved divisions..."
# dialog are taken every 150 ms from the moment the samples are loaded (the log
# says "Committing") until the dialog disappears: cp_<n>.png in %TEMP%.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_cp_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
Get-ChildItem "$env:TEMP\cp_*.png" -ErrorAction SilentlyContinue | Remove-Item -Force
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe"
$t0 = Get-Date
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$shots = 0; $committing = $false; $seenDialog = $false; $tCommit = $null
while (((Get-Date) - $t0).TotalSeconds -lt 150)
{
    Start-Sleep -Milliseconds 150
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert -and $args -notcontains 'noclick') { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    elseif ($alert -and -not $alertSeen) { $alertSeen = $true; "audio alert appeared at $([int]((Get-Date) - $t0).TotalSeconds) s (left open)" }
    $dialog = $wins | Where-Object { $_.Title -match 'Loading saved divisions' } | Select-Object -First 1
    if ($dialog) { $seenDialog = $true }
    if (-not $committing -and (Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Committing' -Quiet)) { $committing = $true; $tCommit = Get-Date; "committing at $([int]((Get-Date) - $t0).TotalSeconds) s" }
    if ($committing -and $dialog -and $shots -lt 40)
    {
        $shots++
        Save-WindowShot $dialog.Handle ("$env:TEMP\cp_{0:00}.png" -f $shots) | Out-Null
        # What is really on the screen (PrintWindow renders even a hidden window): the dialog's area.
        if ($args -contains 'screen')
        {
            Add-Type -AssemblyName System.Drawing
            $bmp = New-Object System.Drawing.Bitmap $dialog.W, $dialog.H
            $g = [System.Drawing.Graphics]::FromImage($bmp)
            $g.CopyFromScreen($dialog.X, $dialog.Y, 0, 0, $bmp.Size); $g.Dispose()
            $bmp.Save(("$env:TEMP\cps_{0:00}.png" -f $shots)); $bmp.Dispose()
        }
    }
    if ($committing -and -not $dialog -and $seenDialog) { "dialog gone after $([Math]::Round(((Get-Date) - $tCommit).TotalSeconds, 1)) s of compiling, $shots shots"; break }
}
# The stop grid a moment later: the copy-build strips on the stop buttons (pitch shift builds in the background).
Start-Sleep -Seconds 3
$main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
if ($main) { Save-WindowShot $main.Handle "$env:TEMP\cp_main.png" | Out-Null; 'main shot (build strips)' }
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Compiling|Committing' | ForEach-Object { '  ' + $_.Line.Substring(0, [Math]::Min(150, $_.Line.Length)) }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
