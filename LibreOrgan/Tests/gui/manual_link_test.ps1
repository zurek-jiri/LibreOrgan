# The "?" buttons: starts the program empty with the Configuration Editor open,
# screenshots the editor (ml_editor.png), clicks the "?" of one row (window-relative
# coordinates given as parameters), screenshots the About dialog that opens
# (ml_about_click.png). With -Section it also opens the manual through
# ORGAN_OPEN_MANUAL (ml_about_env.png). Settings backed up and restored.
param([string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
      [int]$ClickX = -1, [int]$ClickY = -1, [string]$Section = "", [int]$ChapterSteps = 0, [string]$Find = "", [int]$LinkX = -1, [int]$LinkY = -1, [switch]$Back)
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
if (Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$backups = @{}
foreach ($name in "settings.json", "voicer.json", "memories.json") { $src = "$appData\$name"; if (Test-Path $src) { $bak = "$env:TEMP\backup_ml_$name"; Copy-Item $src $bak -Force; $backups[$name] = $bak } }
$file = "$appData\settings.json"
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $false -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue 100 -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$env:ORGAN_OPEN_CONFIG_EDITOR = "1"
if ($Section -ne "") { $env:ORGAN_OPEN_MANUAL = $Section }
$t0 = Get-Date
$p = Start-Process -FilePath $Exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$editor = $null
while (((Get-Date) - $t0).TotalSeconds -lt 40)
{
    Start-Sleep -Milliseconds 700
    $wins = @(Get-OrganWindows $p.Id)
    $alert = $wins | Where-Object { $_.Title -match 'Audio devices not available' } | Select-Object -First 1
    if ($alert) { 'audio alert: ' + (Click-InWindow $alert.Handle ([int]($alert.W / 2)) ($alert.H - 42)) }
    if (((Get-Date) - $t0).TotalSeconds -lt 12) { continue }
    $editor = $wins | Where-Object { $_.Title -match '^Configuration Editor' -and $_.W -gt 300 } | Select-Object -First 1
    if ($editor) { break }
}
if (-not $editor) { "NO EDITOR"; Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false }
else
{
    "editor: $($editor.W)x$($editor.H) at $($editor.X),$($editor.Y)"
    if ($Section -ne "")
    {
        Start-Sleep -Seconds 2
        $about = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^About' } | Select-Object -First 1
        if ($about)
        {
            Save-WindowShot $about.Handle "$env:TEMP\ml_about_env.png" | Out-Null; "about (env): $($about.W)x$($about.H)"
            if ($ChapterSteps -gt 0)
            {
                # The chapter box: open it, step down, Enter - the chosen chapter must come to the top, orange.
                "chapter box: " + (Click-InWindow $about.Handle 600 55)
                Start-Sleep -Milliseconds 700
                for ($i = 0; $i -lt $ChapterSteps; $i++) { [W]::keybd_event(0x28, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60; [W]::keybd_event(0x28, 0, 2, [UIntPtr]::Zero); Start-Sleep -Milliseconds 120 }
                [W]::keybd_event(0x0D, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60; [W]::keybd_event(0x0D, 0, 2, [UIntPtr]::Zero)
                Start-Sleep -Milliseconds 1500
                Save-WindowShot $about.Handle "$env:TEMP\ml_about_combo.png" | Out-Null; "about (combo): saved"
            }
            if ($LinkX -ge 0)
            {
                # A hyperlink in the text (window-relative coordinates): the chapter it names must come to the top.
                "link: " + (Click-InWindow $about.Handle $LinkX $LinkY)
                Start-Sleep -Milliseconds 1500
                Save-WindowShot $about.Handle "$env:TEMP\ml_about_link.png" | Out-Null; "about (link): saved"
                if ($Back)
                {
                    # "< Back" (left of the chapter box): the place read before the link must return.
                    "back: " + (Click-InWindow $about.Handle 390 55)
                    Start-Sleep -Milliseconds 1500
                    Save-WindowShot $about.Handle "$env:TEMP\ml_about_back.png" | Out-Null; "about (back): saved"
                }
            }
            if ($Find -ne "")
            {
                # The Find box (right part of the top line): type the word (letters only - the keyboard layout is Czech), Enter.
                "find box: " + (Click-InWindow $about.Handle ($about.W - 250) 55)
                Start-Sleep -Milliseconds 500
                foreach ($ch in $Find.ToUpper().ToCharArray()) { $vk = [byte][int][char]$ch; [W]::keybd_event($vk, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 40; [W]::keybd_event($vk, 0, 2, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60 }
                [W]::keybd_event(0x0D, 0, 0, [UIntPtr]::Zero); Start-Sleep -Milliseconds 60; [W]::keybd_event(0x0D, 0, 2, [UIntPtr]::Zero)
                Start-Sleep -Milliseconds 1200
                Save-WindowShot $about.Handle "$env:TEMP\ml_about_find.png" | Out-Null; "about (find): saved"
            }
            Click-InWindow $about.Handle ($about.W - 60) ($about.H - 24) | Out-Null; Start-Sleep -Milliseconds 800
        }
        else { "NO ABOUT (env)" }
    }
    Save-WindowShot $editor.Handle "$env:TEMP\ml_editor.png" | Out-Null
    # The empty main window too (the hint with the manual button must show there).
    $mainWin = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
    if ($mainWin) { Click-InWindow $mainWin.Handle 1000 700 | Out-Null; Start-Sleep -Milliseconds 600; Save-WindowShot $mainWin.Handle "$env:TEMP\ml_main.png" | Out-Null; "main: $($mainWin.W)x$($mainWin.H)" }
    if ($ClickX -ge 0)
    {
        "click: " + (Click-InWindow $editor.Handle $ClickX $ClickY)
        Start-Sleep -Seconds 2
        $about = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^About' } | Select-Object -First 1
        if ($about) { Save-WindowShot $about.Handle "$env:TEMP\ml_about_click.png" | Out-Null; "about (click): $($about.W)x$($about.H) title '$($about.Title)'" }
        else { "NO ABOUT (click)" }
    }
}
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 2
foreach ($name in $backups.Keys) { Copy-Item $backups[$name] "$appData\$name" -Force; "$name restored identical: " + ((Get-FileHash "$appData\$name").Hash -eq (Get-FileHash $backups[$name]).Hash) }
$env:ORGAN_OPEN_CONFIG_EDITOR = $null; $env:ORGAN_OPEN_MANUAL = $null
Select-String -Path "$appData\CrashLogs\activity.log" -Pattern 'Manual: no chapter' | Select-Object -Last 2 | ForEach-Object { $_.Line }
