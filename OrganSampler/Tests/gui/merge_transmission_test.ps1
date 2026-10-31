# Stop-Merging of a transmission: starts with the Stralsund configuration and
# ORGAN_MERGE_ON_START=1 (the program switches the transmission with the largest
# shift on and starts Stop-Merging 10 s after the stop list), waits for the
# result dialog, inspects the merged folder (files, smpl chunk: unity note and
# loops of one file, "rel" folders) and deletes the folder again. Settings are
# backed up/restored. Argument: the exe (default: the Release build).
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan* -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$file = "$appData\settings.json"
Copy-Item $file "$env:TEMP\backup_mt_settings.json" -Force
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$root = "F:\haupwerk\003136"
$existing = @(Get-ChildItem $root -Directory -Recurse -Depth 2 -Filter 'StopMerged*' | ForEach-Object { $_.FullName })
$exe = if ($args.Count -ge 1) { $args[0] } else { "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe" }
$env:ORGAN_MERGE_ON_START = "1"
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$env:ORGAN_MERGE_ON_START = $null
$result = $null
$t0 = Get-Date
for ($i = 0; $i -lt 900 -and -not $result; $i++)
{
    Start-Sleep -Seconds 1
    if ($p.HasExited) { 'APP EXITED'; break }
    $wins = @(Get-OrganWindows $p.Id)
    $result = $wins | Where-Object { $_.Title -eq 'Merge' } | Select-Object -First 1
    if ($i % 30 -eq 0) { "  $i s: " + (($wins | ForEach-Object { "'$($_.Title)'" }) -join ' ') }
}
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Merge on start|Compiling the Stop List: done' | ForEach-Object { '  log: ' + $_.Line.Substring(11) }
if ($result) { Start-Sleep -Seconds 1; Save-WindowShot $result.Handle "$env:TEMP\mt_result.png" | Out-Null; "result dialog after $([int]((Get-Date) - $t0).TotalSeconds) s: $($result.W)x$($result.H)"; Click-InWindow $result.Handle ([int]($result.W / 2)) ($result.H - 42) | Out-Null } else { 'NO RESULT DIALOG' }
Start-Sleep -Seconds 2
$folders = @(Get-ChildItem $root -Directory -Recurse -Depth 2 -Filter 'StopMerged*' | Where-Object { $existing -notcontains $_.FullName })
foreach ($f in $folders)
{
    "merged folder: $($f.FullName)"
    foreach ($sub in Get-ChildItem $f.FullName -Directory)
    {
        $wavs = @(Get-ChildItem $sub.FullName -File -Filter *.wav | Sort-Object Name)
        $rels = @(Get-ChildItem $sub.FullName -Directory | ForEach-Object { "$($_.Name)=$((Get-ChildItem $_.FullName -File -Filter *.wav | Measure-Object).Count)" })
        "  $($sub.Name): $($wavs.Count) wav, first $($wavs[0].Name), last $($wavs[-1].Name); release folders: " + ($rels -join ', ')
        foreach ($w in @($wavs[0], $wavs[[int]($wavs.Count / 2)]))
        {
            if (-not $w) { continue }
            $bytes = [IO.File]::ReadAllBytes($w.FullName)
            $pos = 12; $info = 'no smpl'; $ch = 0; $sr = 0; $bits = 0
            while ($pos + 8 -le $bytes.Length)
            {
                $id = [Text.Encoding]::ASCII.GetString($bytes, $pos, 4); $size = [BitConverter]::ToUInt32($bytes, $pos + 4)
                if ($id -eq 'smpl') { $unity = [BitConverter]::ToUInt32($bytes, $pos + 8 + 12); $frac = [BitConverter]::ToUInt32($bytes, $pos + 8 + 16); $loops = [BitConverter]::ToUInt32($bytes, $pos + 8 + 28); $info = "smpl unity $unity + $([Math]::Round($frac / 4294967296.0, 3)), loops $loops"; if ($loops -gt 0) { $ls = [BitConverter]::ToUInt32($bytes, $pos + 8 + 36 + 8); $le = [BitConverter]::ToUInt32($bytes, $pos + 8 + 36 + 12); $info += ", first $ls..$le" }; break }
                if ($id -eq 'fmt ') { $ch = [BitConverter]::ToUInt16($bytes, $pos + 10); $sr = [BitConverter]::ToUInt32($bytes, $pos + 12); $bits = [BitConverter]::ToUInt16($bytes, $pos + 22) }
                $pos += 8 + $size + ($size % 2)
            }
            "    $($w.Name): $ch ch, $sr Hz, $bits bit, $info, $([Math]::Round($w.Length / 1MB, 1)) MB"
        }
    }
}
$main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
Add-Type -Namespace Win -Name MsgM -MemberDefinition '[DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr w, IntPtr l);'
if ($main) { [void][Win.MsgM]::PostMessage($main.Handle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) }
for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500; $q = Get-OrganWindows $p.Id | Where-Object { $_.Title -match 'Quit LibreOrgan Sampler|unsaved|Configuration' } | Select-Object -First 1; if ($q) { Click-InWindow $q.Handle ([int]($q.W / 2)) ($q.H - 42) | Out-Null } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 3
Copy-Item "$env:TEMP\backup_mt_settings.json" $file -Force
"settings restored identical: " + ((Get-FileHash $file).Hash -eq (Get-FileHash "$env:TEMP\backup_mt_settings.json").Hash)
foreach ($f in $folders) { [IO.Directory]::Delete($f.FullName, $true); "deleted $($f.FullName)" }
