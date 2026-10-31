# How long quitting takes while pitch-shifted copies are still being built:
# starts with the Stralsund configuration (transposeMethod 1 forced), waits until
# the stop list is compiled, closes the main window (WM_CLOSE) and measures the time
# until the process is gone. Settings are backed up and restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_qt_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
$json | Add-Member -NotePropertyName transposeMethod -NotePropertyValue 1 -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$exe = if ($args.Count -ge 1) { $args[0] } else { "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe" }
$t0 = Get-Date
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$main = $null; $compiled = $false
while (((Get-Date) - $t0).TotalSeconds -lt 150)
{
    Start-Sleep -Milliseconds 500
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    $main = $wins | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
    if ((Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Compiling the Stop List: done' -Quiet)) { $compiled = $true }
    if ($main -and $compiled) { Start-Sleep -Seconds 4; break }   # the copy builds are running now
}
"loaded in $([int]((Get-Date) - $t0).TotalSeconds) s, compiled: $compiled"
if ($main)
{
    Add-Type -Namespace Win -Name Msg -MemberDefinition '[DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr w, IntPtr l);'
    $tClose = Get-Date
    [void][Win.Msg]::PostMessage($main.Handle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)   # WM_CLOSE
    $hidden = $null
    while (((Get-Date) - $tClose).TotalSeconds -lt 120)
    {
        Start-Sleep -Milliseconds 100
        $wins = @(Get-OrganWindows $p.Id)
        $question = $wins | Where-Object { $_.Title -match 'Quit LibreOrgan Sampler' } | Select-Object -First 1
        if ($question) { 'quit question: ' + (Click-InWindow $question.Handle ([int]($question.W / 2)) ($question.H - 42)); Start-Sleep -Milliseconds 300 }
        if ($null -eq $hidden -and $wins.Count -eq 0) { $hidden = ((Get-Date) - $tClose).TotalSeconds }
        if ($p.HasExited) { break }
    }
    "windows gone after $([Math]::Round($hidden, 1)) s, process exited after $([Math]::Round(((Get-Date) - $tClose).TotalSeconds, 1)) s (exited: $($p.HasExited))"
}
else { 'no main window' }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
