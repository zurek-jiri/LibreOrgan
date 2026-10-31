# Screenshots for the website (site/assets): the main window with a loaded
# configuration, the Divisions dialog, the Configuration Editor and the Stop
# List. Loads the given configuration at start-up, opens the dialogs through
# the development variables and saves main/divisions/editor/stoplist.png.
# Settings are backed up and restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
      [string]$Configuration = "F:\Suchy_rejstrik\Kobrle\Organ configuration.json",
      [string]$Output = "F:\JUCE_PROJEKTY\LibreOrgan\site\assets",
      [string]$Division = "1")
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
$waited = 0
while ((Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) -and $waited -lt 20) { Start-Sleep -Seconds 1; $waited++ }
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_ss_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [IO.File]::ReadAllText($file, [Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue $Configuration -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue 100 -Force
[IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object Text.UTF8Encoding $false))
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$done = @{}
$mainShot = $false
# The main window first (the configuration loads for up to a minute), then the dialogs one by one.
while (((Get-Date) - $t0).TotalSeconds -lt 200)
{
    Start-Sleep -Milliseconds 800
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)); continue }
    $elapsed = ((Get-Date) - $t0).TotalSeconds
    $main = $wins | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 800 } | Select-Object -First 1
    $loading = $wins | Where-Object { $_.Title -match 'Loading|Compiling' } | Select-Object -First 1
    if ($main -and -not $mainShot -and $elapsed -ge 70 -and -not $loading)
    {
        [void][W]::Activate($main.Handle); Start-Sleep -Milliseconds 500
        Save-WindowShot $main.Handle "$Output\main.png" | Out-Null; $mainShot = $true; "main: $($main.W)x$($main.H)"
        $env:ORGAN_DUMMY = "1"
        # The dialogs: opened through the hooks of a second instance is not possible (single instance),
        # so they are opened by clicking the top-bar buttons of this one.
        continue
    }
    if ($mainShot)
    {
        $wanted = @{ 'Divisions' = 'divisions'; 'Configuration Editor' = 'editor'; 'Stop List' = 'stoplist' }
        $next = @('Stop List', 'Configuration Editor', 'Divisions') | Where-Object { -not $done[$_] } | Select-Object -First 1
        if (-not $next) { break }
        $w = $wins | Where-Object { $_.Title -match ('^' + [regex]::Escape($next)) -and $_.W -gt 300 } | Select-Object -First 1
        if ($w)
        {
            Start-Sleep -Milliseconds 1200
            Save-WindowShot $w.Handle ("$Output\{0}.png" -f $wanted[$next]) | Out-Null
            "$next : $($w.W)x$($w.H)"; $done[$next] = $true
            # Close it (Escape) before the next one.
            [void][W]::Activate($w.Handle); Start-Sleep -Milliseconds 300
            [W]::keybd_event(0x1B, 0, 0, [UIntPtr]::Zero); [W]::keybd_event(0x1B, 0, 2, [UIntPtr]::Zero); Start-Sleep -Milliseconds 800
            continue
        }
        if (-not $done["opening:$next"])
        {
            # Top-bar buttons at 100 % scale on the 2559-px window: Stop List (366,52), Configuration Editor (1744,52) in the
            # 2000-px screenshot scale -> x1.28; Divisions: the "Settings" button of division I (719,214).
            $click = switch ($next) { 'Stop List' { @(468, 67) } 'Configuration Editor' { @(2232, 67) } 'Divisions' { @(920, 274) } }
            'open ' + $next + ': ' + (Click-InWindow $main.Handle $click[0] $click[1])
            $done["opening:$next"] = $true
        }
    }
}
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force }
Get-ChildItem $Output -Filter *.png | Select-Object Name, Length
