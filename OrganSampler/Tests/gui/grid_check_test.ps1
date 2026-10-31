# Starts with the Stralsund configuration (the user's voicer.json as it is),
# waits for the stop list, screenshots the main window (%TEMP%\gc_main.png) and
# reports how many grid columns each division's stops occupy afterwards (from
# the saved voicer.json). Settings are backed up/restored; the voicer state is
# left as the program saved it (a one-column division is laid out afresh).
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan* -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$file = "$appData\settings.json"
Copy-Item $file "$env:TEMP\backup_gc_settings.json" -Force
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$exe = if ($args.Count -ge 1) { $args[0] } else { "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe" }
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$done = $false
for ($i = 0; $i -lt 150 -and -not $done; $i++) { Start-Sleep -Seconds 1; $done = (Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Compiling the Stop List: done' -Quiet) }
"stop list done: $done"
Start-Sleep -Seconds 4
$main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
if ($main) { Save-WindowShot $main.Handle "$env:TEMP\gc_main.png" | Out-Null; "main shot $($main.W)x$($main.H)" }
Add-Type -Namespace Win -Name MsgG -MemberDefinition '[DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr w, IntPtr l);'
if ($main) { [void][Win.MsgG]::PostMessage($main.Handle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) }
for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500; $q = Get-OrganWindows $p.Id | Where-Object { $_.Title -match 'Quit LibreOrgan Sampler' } | Select-Object -First 1; if ($q) { Click-InWindow $q.Handle ([int]($q.W / 2)) ($q.H - 42) | Out-Null } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 3
Copy-Item "$env:TEMP\backup_gc_settings.json" $file -Force
"settings restored identical: " + ((Get-FileHash $file).Hash -eq (Get-FileHash "$env:TEMP\backup_gc_settings.json").Hash)
$v = Get-Content "$appData\voicer.json" -Raw | ConvertFrom-Json
foreach ($d in $v.stopGrid.PSObject.Properties) {
    $cols = @{}
    foreach ($c in $d.Value.PSObject.Properties) { if ($c.Name -match '^(UW|HW|OW|Ped|PEd) ' -and $c.Name -notmatch ' - ') { $col = ($c.Value -split ',')[0]; $cols[$col] = 1 + [int]$cols[$col] } }
    "division $($d.Name) (Jakobi keys): " + (($cols.GetEnumerator() | Sort-Object Name | ForEach-Object { "column $($_.Name): $($_.Value)" }) -join ', ')
}
