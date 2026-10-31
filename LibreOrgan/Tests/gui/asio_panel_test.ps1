# "Driver Control Panel" on the pedal's ASIO route (Stralsund configuration):
# presses our button in the Divisions dialog, drives the RME "Fireface USB
# Settings" dialog through UI Automation (buffer size to another value, OK),
# and checks the activity log for the reopen with the new buffer size; then a
# second cycle puts the original value back. Settings are backed up/restored.
. "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tests\gui\organ_gui.ps1"
Add-Type -AssemblyName UIAutomationClient; Add-Type -AssemblyName UIAutomationTypes
if (Get-Process LibreOrgan* -ErrorAction SilentlyContinue) { 'APP RUNNING - abort'; exit 1 }
$appData = "$env:APPDATA\LibreOrgan"
$file = "$appData\settings.json"
Copy-Item $file "$env:TEMP\backup_panel_settings.json" -Force
$json = [System.IO.File]::ReadAllText($file, [System.Text.Encoding]::UTF8) | ConvertFrom-Json
$json | Add-Member -NotePropertyName loadLastConfigurationAtStartup -NotePropertyValue $true -Force
$json | Add-Member -NotePropertyName configurationFile -NotePropertyValue "F:\haupwerk\003136\Stralsund_Jakobi_Wegscheider.json" -Force
$json | Add-Member -NotePropertyName uiScalePercent -NotePropertyValue 100 -Force
[System.IO.File]::WriteAllText($file, ($json | ConvertTo-Json -Depth 30), (New-Object System.Text.UTF8Encoding($false)))
$log = "$appData\CrashLogs\activity.log"
$before = (Get-Content $log).Count
$env:ORGAN_OPEN_KEYBOARDS = "0"
$exe = if ($args.Count -ge 1) { $args[0] } else { "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe" }
$p = Start-Process -FilePath $exe -WorkingDirectory "C:\LibreOrgan" -PassThru
$env:ORGAN_OPEN_KEYBOARDS = $null
$dialog = $null
for ($i = 0; $i -lt 120 -and -not $dialog; $i++) { Start-Sleep -Seconds 1; $dialog = Get-OrganWindows $p.Id | Where-Object { $_.Title -eq 'Divisions' } | Select-Object -First 1 }
if (-not $dialog) { 'NO DIVISIONS DIALOG'; Stop-Process -Id $p.Id -Force; Copy-Item "$env:TEMP\backup_panel_settings.json" $file -Force; exit 1 }
Start-Sleep -Seconds 3
"dialog $($dialog.W)x$($dialog.H) at $($dialog.X),$($dialog.Y)"
Save-WindowShot $dialog.Handle "$env:TEMP\ap_dialog.png" | Out-Null

function Find-Panel {
    $root = [System.Windows.Automation.AutomationElement]::RootElement
    foreach ($w in $root.FindAll([System.Windows.Automation.TreeScope]::Children, [System.Windows.Automation.Condition]::TrueCondition)) {
        $n = $w.Current.Name
        if ($n -match 'Settings' -and $n -match 'Fireface') { return $w }
    }
    return $null
}
function Get-LogLines { Get-Content $log | Select-Object -Skip $before }
function Wait-LogLine($pattern, $seconds) {
    for ($i = 0; $i -lt $seconds * 2; $i++) { $m = Get-LogLines | Select-String -Pattern $pattern | Select-Object -Last 1; if ($m) { return $m.Line }; Start-Sleep -Milliseconds 500 }
    return $null
}
function Set-Buffer($panel, $wanted) {
    # The buffer combo: the one whose value looks like a sample count.
    $combos = $panel.FindAll([System.Windows.Automation.TreeScope]::Descendants, (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::ComboBox)))
    foreach ($c in $combos) {
        $value = ''; $vp = $null
        if ($c.TryGetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern, [ref]$vp)) { $value = $vp.Current.Value }
        "  combo '$($c.Current.Name)' value '$value'"
        if ($value -notmatch '^\s*\d+\s*(Samples)?\s*$' -and $c.Current.Name -notmatch 'Buffer|Latency') { continue }
        $ec = $null
        if (-not $c.TryGetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern, [ref]$ec)) { continue }
        $ec.Expand(); Start-Sleep -Milliseconds 400
        $items = $c.FindAll([System.Windows.Automation.TreeScope]::Descendants, (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::ListItem)))
        $names = @(); foreach ($it in $items) { $names += $it.Current.Name }
        "  items: " + ($names -join ', ')
        foreach ($it in $items) {
            if ($it.Current.Name -match "^\s*$wanted\b") {
                $sp = $it.GetCurrentPattern([System.Windows.Automation.SelectionItemPattern]::Pattern); $sp.Select(); Start-Sleep -Milliseconds 300
                $ec.Collapse(); return "selected '$($it.Current.Name)'"
            }
        }
        $ec.Collapse()
    }
    return 'BUFFER COMBO NOT FOUND'
}
function Press-Ok($panel) {
    $buttons = $panel.FindAll([System.Windows.Automation.TreeScope]::Descendants, (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::Button)))
    $names = @(); foreach ($b in $buttons) { $names += $b.Current.Name }
    "  buttons: " + ($names -join ', ')
    foreach ($b in $buttons) { if ($b.Current.Name -match '^(OK|Close|Schlie|Zav)') { $ip = $b.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern); $ip.Invoke(); return "pressed '$($b.Current.Name)'" } }
    return 'NO OK BUTTON'
}
$results = @()
$current = $null
foreach ($cycle in 1, 2) {
    "=== cycle $cycle"
    # The dialog is rebuilt after a reopen (new window handle): find it again.
    $dialog = $null
    for ($i = 0; $i -lt 20 -and -not $dialog; $i++) { $dialog = Get-OrganWindows $p.Id | Where-Object { $_.Title -eq 'Divisions' } | Select-Object -First 1; if (-not $dialog) { Start-Sleep -Milliseconds 500 } }
    if (-not $dialog) { 'NO DIVISIONS DIALOG'; break }
    # "Driver Control Panel" sits at (755,817) of the 1156x939 dialog at 100 %.
    Click-InWindow $dialog.Handle ([int](755 * $dialog.W / 1156)) ([int](817 * $dialog.H / 939))
    $opened = Wait-LogLine 'Opening control panel of' 8
    "log: $opened"
    $panel = $null
    for ($i = 0; $i -lt 20 -and -not $panel; $i++) { Start-Sleep -Milliseconds 500; $panel = Find-Panel }
    if (-not $panel) { 'NO RME SETTINGS WINDOW'; break }
    "panel: '$($panel.Current.Name)'"
    Start-Sleep -Seconds 1
    $detected = Get-LogLines | Select-String -Pattern 'Control panel window detected|ignoring the window' | ForEach-Object { $_.Line.Substring(20) }
    $detected | ForEach-Object { "  $_" }
    if ($cycle -eq 1) {
        if ($opened -match 'buffer (\d+)') { $current = [int]$Matches[1] }
        $target = if ($current -eq 512) { 256 } else { 512 }
    }
    else { $target = $current }
    "setting buffer $target"
    Set-Buffer $panel $target
    Press-Ok $panel
    $reopened = Wait-LogLine 'Control panel closed: reopened|Control panel finished: reopened' 25
    "reopen: $reopened"
    $results += $reopened
    Start-Sleep -Seconds 3
}
$main = Get-OrganWindows $p.Id | Where-Object { $_.Title -match '^LibreOrgan Sampler' -and $_.W -gt 300 } | Select-Object -First 1
Add-Type -Namespace Win -Name MsgP -MemberDefinition '[DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, uint msg, IntPtr w, IntPtr l);'
if ($main) { [void][Win.MsgP]::PostMessage($main.Handle, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) }
for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500; $q = Get-OrganWindows $p.Id | Where-Object { $_.Title -match 'Quit LibreOrgan Sampler' } | Select-Object -First 1; if ($q) { Click-InWindow $q.Handle ([int]($q.W / 2)) ($q.H - 42) | Out-Null } }
Get-Process LibreOrgan Sampler -ErrorAction SilentlyContinue | Stop-Process -Force -Confirm:$false
Start-Sleep -Seconds 3
Copy-Item "$env:TEMP\backup_panel_settings.json" $file -Force
"settings restored identical: " + ((Get-FileHash $file).Hash -eq (Get-FileHash "$env:TEMP\backup_panel_settings.json").Hash)
'--- log (control panel lines)'
Get-LogLines | Select-String -Pattern 'Control panel|reopened|anomaly' | ForEach-Object { $_.Line.Substring(11, [Math]::Min(170, $_.Line.Length - 11)) }
