# The instruction video (0.8.206): runs LibreOrgan Sampler with the video script (ORGAN_VIDEO_SCRIPT), which
# loads the configuration, drives the console and writes the frames (PNG with their times) and the sound
# (audio.wav) into the output folder; then ffmpeg joins them into an MP4 (30 fps, H.264 + AAC) and a GIF.
# Usage: make_video.ps1 -Exe <LibreOrgan.exe> -Config <cfg.json> [-Script Tools\video_stralsund.txt] [-Out <folder>] [-Seconds 400]
# The run must not overlap the user's own LibreOrgan Sampler (the audio devices); settings.json is backed up and restored.
param(
    [string]$Exe = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Builds\VisualStudio2026\x64\Release\App\LibreOrgan.exe",
    [string]$Config = "$env:TEMP\video\cfg\Stralsund_Jakobi_Wegscheider.portable.json",
    [string]$Script = "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tools\video_stralsund.txt",
    [string]$Out = "$env:TEMP\video\out",
    [int]$Seconds = 420,
    [string]$Ffmpeg = "C:\ffmpeg\bin\ffmpeg.exe",
    [string]$Python = "C:\Users\doma\kokoro-venv\Scripts\python.exe",   # Kokoro's venv (1 Nov 2026); the edge-tts path needs Python39 + "--engine edge"
    [string]$Voice = "bm_lewis",
    [string]$VoiceRate = "1.0"
)
# Kokoro's phonemiser falls back to eSpeak NG for words outside its dictionary.
$env:PATH = "C:\Program Files\eSpeak NG;" + $env:PATH
$env:PHONEMIZER_ESPEAK_LIBRARY = "C:\Program Files\eSpeak NG\libespeak-ng.dll"
if (Get-Process | Where-Object { $_.ProcessName -like "LibreOrgan*" }) { "LibreOrgan Sampler is running - not started"; exit 1 }
$settings = "$env:APPDATA\LibreOrgan\settings.json"
$backup = "$env:TEMP\settings_backup_video.json"
Copy-Item $settings $backup -Force
if (Test-Path $Out) { Remove-Item $Out -Recurse -Force -Confirm:$false }
New-Item -ItemType Directory -Force $Out | Out-Null
$log = "$env:APPDATA\LibreOrgan\CrashLogs\activity.log"
$mark = (Get-Item $log).Length
# A script that loads the organ itself ("load FILE": the video begins with the loading) runs from the program's start;
# the auto-load of the last configuration is switched off for the run (settings.json is restored afterwards).
$scriptLoads = (Get-Content $Script | Where-Object { $_ -match '^load ' }).Count -gt 0
if ($scriptLoads) {
    $text = [IO.File]::ReadAllText($settings)
    $text = $text -replace '"loadLastConfigurationAtStartup"\s*:\s*true', '"loadLastConfigurationAtStartup": false'
    [IO.File]::WriteAllText($settings, $text, (New-Object System.Text.UTF8Encoding $false))
} else {
    $env:ORGAN_LOAD_CONFIGURATION = $Config
    $env:ORGAN_DUMP_STOPLIST = "$Out\stoplist.md"
}
$env:ORGAN_VIDEO_SCRIPT = $Script
$env:ORGAN_VIDEO_OUT = $Out
# A run of ours killed while a device opened leaves the crash handler's marker: the next start would show the
# "Safe start ... default buffer" box in the video. The marker is cleared before the run.
Remove-Item "$env:APPDATA\LibreOrgan\CrashLogs\opening-device.txt" -Force -ErrorAction SilentlyContinue
$p = Start-Process -FilePath $Exe -PassThru
$deadline = (Get-Date).AddSeconds($Seconds)
while ((Get-Date) -lt $deadline -and -not $p.HasExited) {
    Start-Sleep 3
    if (Test-Path "$Out\frames.txt") { Start-Sleep 4; break }
}
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -Confirm:$false; Start-Sleep 2 }
Remove-Item Env:ORGAN_LOAD_CONFIGURATION, Env:ORGAN_DUMP_STOPLIST, Env:ORGAN_VIDEO_SCRIPT, Env:ORGAN_VIDEO_OUT -ErrorAction SilentlyContinue
Copy-Item $backup $settings -Force
$fs = [IO.File]::OpenRead($log); $fs.Seek($mark, 'Begin') | Out-Null
$sr = New-Object IO.StreamReader($fs); $new = $sr.ReadToEnd(); $sr.Close(); $fs.Close()
$pattern = 'ORGAN_VIDEO_SCRIPT|Video: |Video step: (record|drop|click|stoplist|temperament)|Load Configuration "|not on this computer|crash'
$new -split "`n" | Where-Object { $_ -match $pattern } | ForEach-Object { if ($_.Length -gt 200) { $_.Substring(0,200) } else { $_ } }
if (-not (Test-Path "$Out\frames.txt")) { "NO FRAMES"; exit 1 }
$frames = (Get-ChildItem $Out -Filter "f*.png").Count
"frames: $frames, audio: $(if (Test-Path "$Out\audio.wav") { (Get-Item "$Out\audio.wav").Length } else { 'NONE' }) bytes"
Push-Location $Out
# The recording must be clean (1 Nov 2026, the user: "zvuk hlavně musí být bez glitchů"): the program's own
# detector must have stayed silent during the run, and the file is scanned for discontinuities; either finding
# ends the run - record again.
if (($new -split "`n" | Where-Object { $_ -match 'Output discontinuities' }).Count -gt 0) { "THE PROGRAM REPORTED OUTPUT DISCONTINUITIES DURING THE RUN - record again"; Pop-Location; exit 1 }
& $Python -I "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tools\check_recording.py" "$Out\audio.wav"
if ($LASTEXITCODE -ne 0) { "THE RECORDING HAS DISCONTINUITIES - record again"; Pop-Location; exit 1 }
# The narration: the "say" sentences of the script (narration.txt) spoken by Kokoro (local) into narration.wav with
# our pauses. Without narration.txt the organ alone.
$narrated = $false
$narrationEnd = 0.0
if ((Test-Path "$Out\narration.txt") -and (Get-Item "$Out\narration.txt").Length -gt 0) {
    $narrLog = & $Python -I "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Tools\make_narration.py" "$Out\narration.txt" $Out $Ffmpeg $Voice $VoiceRate 2>&1 | Where-Object { $_ -notmatch 'Warning|warn|Defaulting|repo_id|symlink' }
    $narrLog
    $m = $narrLog | Select-String 'ends at ([0-9.]+) s' | Select-Object -Last 1
    if ($m) { $narrationEnd = [double]$m.Matches[0].Groups[1].Value }
    $narrated = Test-Path "$Out\narration.wav"
}
# The levels (the user's rules of 1 Nov 2026): NO ducking, NO compressor, NO limiter - the organ keeps its own
# dynamics (only the shutters move it), one plain gain brings it to -18 LUFS (ebur128 integrated) and one brings the
# soft voice to -24 LUFS (3 dB under the previous video); a true peak above -1 dBFS lowers BOTH by the same amount.
# The video runs to the end of the last sentence + 2 s: the last frame is held that long (no -shortest).
function Measure-Lufs($file) {
    $o = & $Ffmpeg -nostdin -nostats -i $file -af ebur128=framelog=quiet -f null - 2>&1 | Select-String "I:\s+(-?[0-9.]+) LUFS" | Select-Object -Last 1
    if ($o) { [double]$o.Matches[0].Groups[1].Value } else { -23.0 }
}
function Measure-Peak($filter) {
    # the sample peak (dBFS) of the mixed signal with the given gains, measured before the final render
    $o = & $Ffmpeg -nostdin -nostats -i audio.wav -i narration.wav -filter_complex "$filter,astats=metadata=1:measure_overall=Peak_level:measure_perchannel=none" -f null - 2>&1 | Select-String "Peak level dB:\s+(-?[0-9.]+)" | Select-Object -Last 1
    if ($o) { [double]$o.Matches[0].Groups[1].Value } else { -3.0 }
}
# The frames list: the last frame held so the picture lasts to the narration's end + 2 s.
$frameLines = Get-Content frames.txt
$lastFile = ($frameLines | Where-Object { $_ -match '^file ' } | Select-Object -Last 1)
$videoEnd = 0.0
foreach ($l in $frameLines) { if ($l -match '^duration ([0-9.]+)') { $videoEnd += [double]$matches[1] } }
if ($narrationEnd + 2.0 -gt $videoEnd -and $lastFile) {
    $extra = [math]::Round($narrationEnd + 2.0 - $videoEnd, 2)
    Add-Content frames.txt "$lastFile`nduration $extra`n$lastFile"
    "video held $extra s longer: the narration ends at $narrationEnd s, the frames at $videoEnd s"
}
if ($narrated) {
    $organLufs = Measure-Lufs "audio.wav"; $voiceLufs = Measure-Lufs "narration.wav"
    if ($organLufs -lt -45.0) { "THE RECORDING IS (NEARLY) SILENT: $organLufs LUFS - no stops drawn? Record again"; Pop-Location; exit 1 }
    $organGain = [math]::Round(-18.0 - $organLufs, 1); $voiceGain = [math]::Round(-24.0 - $voiceLufs, 1)
    # No apad here: apad + duration=longest never ends and the measurement ran for ever (the first opening test).
    $mixFilter = "[0:a]volume=${organGain}dB,aformat=channel_layouts=stereo[org];[1:a]volume=${voiceGain}dB,aformat=channel_layouts=stereo[nar];[org][nar]amix=inputs=2:duration=longest:dropout_transition=0:normalize=0"
    $peak = Measure-Peak $mixFilter
    if ($peak -gt -1.0) { $organGain = [math]::Round($organGain - ($peak + 1.0), 1); $voiceGain = [math]::Round($voiceGain - ($peak + 1.0), 1); "true peak $peak dBFS: both gains lowered by $([math]::Round($peak + 1.0, 1)) dB" }
    "levels: organ $organLufs LUFS -> gain $organGain dB; voice $voiceLufs LUFS -> gain $voiceGain dB; no ducking, no limiter"
    $length = [math]::Round([math]::Max($videoEnd, $narrationEnd + 2.0), 2)
    & $Ffmpeg -nostdin -y -loglevel error -f concat -safe 0 -i frames.txt -i audio.wav -i narration.wav -filter_complex "[1:a]volume=${organGain}dB,aformat=channel_layouts=stereo,apad[org];[2:a]volume=${voiceGain}dB,aformat=channel_layouts=stereo,apad[nar];[org][nar]amix=inputs=2:duration=longest:dropout_transition=0:normalize=0[a]" -map 0:v -map "[a]" -c:v libx264 -preset slow -crf 20 -pix_fmt yuv420p -vf "scale=1920:-2" -r 30 -c:a aac -b:a 320k -ar 48000 -t $length libreorgan.mp4
} else {
    & $Ffmpeg -nostdin -y -loglevel error -f concat -safe 0 -i frames.txt -i audio.wav -c:v libx264 -preset slow -crf 20 -pix_fmt yuv420p -vf "scale=1920:-2" -r 30 -af "loudnorm=I=-16:TP=-1.5:LRA=11" -c:a aac -b:a 320k -ar 48000 -shortest libreorgan.mp4
}
& $Ffmpeg -y -loglevel error -i libreorgan.mp4 -vf "fps=8,scale=960:-1:flags=lanczos,split[s0][s1];[s0]palettegen=max_colors=128[p];[s1][p]paletteuse=dither=bayer:bayer_scale=3" -loop 0 libreorgan.gif
Pop-Location
Get-ChildItem $Out -Filter "libreorgan.*" | Select-Object Name, Length
