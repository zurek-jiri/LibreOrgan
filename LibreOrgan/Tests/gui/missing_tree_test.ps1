# A configuration whose sample trees are not on this computer: it must load all
# the same, every division header must show "Sample folder not found: ...",
# and a folder of the same name loaded later (ORGAN_LOAD_TREE, as "Load
# folder..." would) must take the missing tree's place and clear the notice of
# that tree. The Stralsund configuration is copied to %TEMP% with its pool paths
# pointed at a drive that does not exist. Screenshots mt_before.png / mt_after.png
# in %TEMP%; the settings are backed up and restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
$waited = 0
while ((Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) -and $waited -lt 20) { Start-Sleep -Seconds 1; $waited++ }
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_mt2_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
# The patched copy: every pool path on a drive that does not exist.
$dir = "$env:TEMP\missing_tree_test"; New-Item -ItemType Directory -Force $dir | Out-Null
$text = [IO.File]::ReadAllText("F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json", [Text.Encoding]::UTF8)
$text = $text.Replace('F:\\haupwerk\\', 'Q:\\nowhere\\')
$config = "$dir\Stralsund_missing.json"
[IO.File]::WriteAllText($config, $text, (New-Object Text.UTF8Encoding $false))
"pool paths replaced: " + ([regex]::Matches($text, 'Q:\\\\nowhere')).Count
$file = "$appData\settings.json"
$json = [IO.File]::ReadAllText($file, [Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue $config -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue 100 -Force
[IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object Text.UTF8Encoding $false))
$log = "$appData\CrashLogs\activity.log"
$before = if (Test-Path $log) { (Get-Content $log).Count } else { 0 }
$env:ORGAN_LOAD_TREE = "0|F:\haupwerk\003136\pipe\PE"
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$env:ORGAN_LOAD_TREE = $null
$shotBefore = $false; $shotAfter = $false
while (((Get-Date) - $t0).TotalSeconds -lt 75)
{
    Start-Sleep -Milliseconds 1000
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    $main = $wins | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 800 } | Select-Object -First 1
    $elapsed = ((Get-Date) - $t0).TotalSeconds
    if ($main -and -not $shotBefore -and $elapsed -ge 18) { Save-WindowShot $main.Handle "$env:TEMP\mt_before.png" | Out-Null; $shotBefore = $true; "before: $($main.Title)" }
    if ($main -and -not $shotAfter -and $elapsed -ge 60) { Save-WindowShot $main.Handle "$env:TEMP\mt_after.png" | Out-Null; $shotAfter = $true; "after: $($main.Title)"; break }
}
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }
Get-Content $log | Select-Object -Skip $before | Select-String -Pattern 'Load Configuration|not on this computer|takes the place|ORGAN_LOAD_TREE|Reloading division|Tree .* joins|already loaded' | ForEach-Object { $_.Line.Substring(0, [Math]::Min(300, $_.Line.Length)) }
