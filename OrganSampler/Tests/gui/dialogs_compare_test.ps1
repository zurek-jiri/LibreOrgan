# Screenshots of the main dialogs of a given build for comparison (JUCE 8 vs 9):
#   dialogs_compare_test.ps1 <exe> <tag>
# Loads the Stralsund configuration at start-up and opens Divisions (manual I.),
# the Configuration Editor and the Stop List (division 1) through the dev
# variables; saves dc_<tag>_<dialog>.png in %TEMP%. Settings backed up/restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe", [string]$Tag = "new", [string]$Division = "1")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_dc_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue 100 -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$env:ORGAN_OPEN_KEYBOARDS = $Division
$env:ORGAN_OPEN_CONFIG_EDITOR = "1"
$env:ORGAN_OPEN_STOP_BUILD = "1"
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$wanted = @{ 'Divisions' = 'divisions'; 'Configuration Editor' = 'editor'; 'Stop List' = 'stoplist' }
$done = @{}
while (((Get-Date) - $t0).TotalSeconds -lt 150)
{
    Start-Sleep -Milliseconds 700
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    if (((Get-Date) - $t0).TotalSeconds -lt 50) { continue }   # the configuration loads first
    foreach ($w in $wins)
    {
        foreach ($key in $wanted.Keys)
        {
            if ($w.Title -match ('^' + [regex]::Escape($key)) -and -not $done[$key] -and $w.W -gt 300)
            {
                Start-Sleep -Milliseconds 800
                Save-WindowShot $w.Handle ("$env:TEMP\dc_{0}_{1}.png" -f $Tag, $wanted[$key]) | Out-Null
                "$key : $($w.W)x$($w.H)"; $done[$key] = $true
            }
        }
    }
    if ($done.Count -ge 3) { break }
}
$main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
if ($main) { Save-WindowShot $main.Handle ("$env:TEMP\dc_{0}_main.png" -f $Tag) | Out-Null; "main: $($main.W)x$($main.H)" }
foreach ($key in $wanted.Keys) { if (-not $done[$key]) { "MISSING: $key" } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
$env:ORGAN_OPEN_KEYBOARDS = $null; $env:ORGAN_OPEN_CONFIG_EDITOR = $null; $env:ORGAN_OPEN_STOP_BUILD = $null
