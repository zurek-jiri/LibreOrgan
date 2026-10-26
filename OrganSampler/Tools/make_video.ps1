# The instruction video (0.8.206): runs OrganSampler with the video script (ORGAN_VIDEO_SCRIPT), which
# loads the configuration, drives the console and writes the frames (PNG with their times) and the sound
# (audio.wav) into the output folder; then ffmpeg joins them into an MP4 (30 fps, H.264 + AAC) and a GIF.
# Usage: make_video.ps1 -Exe <OrganSampler.exe> -Config <cfg.json> [-Script Tools\video_stralsund.txt] [-Out <folder>] [-Seconds 400]
# The run must not overlap the user's own OrganSampler (the audio devices); settings.json is backed up and restored.
param(
    [string]$Exe = "F:\JUCE_PROJEKTY\OrganSamplerC\OrganSampler\Builds\VisualStudio2026\x64\Release\App\OrganSampler.exe",
    [string]$Config = "$env:TEMP\video\cfg\Stralsund_Jakobi_Wegscheider.portable.json",
    [string]$Script = "F:\JUCE_PROJEKTY\OrganSamplerC\OrganSampler\Tools\video_stralsund.txt",
    [string]$Out = "$env:TEMP\video\out",
    [int]$Seconds = 420,
    [string]$Ffmpeg = "C:\ffmpeg\bin\ffmpeg.exe",
    [string]$Python = "C:\Users\doma\AppData\Local\Programs\Python\Python39\python.exe",
    [string]$Voice = "en-GB-SoniaNeural",
    [string]$VoiceRate = "-4%"
)
if (Get-Process | Where-Object { $_.ProcessName -like "OrganSampler*" }) { "OrganSampler is running - not started"; exit 1 }
$settings = "$env:APPDATA\OrganSampler\settings.json"
$backup = "$env:TEMP\settings_backup_video.json"
Copy-Item $settings $backup -Force
if (Test-Path $Out) { Remove-Item $Out -Recurse -Force -Confirm:$false }
New-Item -ItemType Directory -Force $Out | Out-Null
$log = "$env:APPDATA\OrganSampler\CrashLogs\activity.log"
$mark = (Get-Item $log).Length
$env:ORGAN_LOAD_CONFIGURATION = $Config
$env:ORGAN_DUMP_STOPLIST = "$Out\stoplist.md"
$env:ORGAN_VIDEO_SCRIPT = $Script
$env:ORGAN_VIDEO_OUT = $Out
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
# The narration: the "say" sentences of the script (narration.txt) spoken by a neural voice (edge-tts, online) into
# narration.wav; the organ is ducked a little under the speech and both are mixed. Without narration.txt the organ alone.
$narrated = $false
if ((Test-Path "$Out\narration.txt") -and (Get-Item "$Out\narration.txt").Length -gt 0) {
    & $Python -I "F:\JUCE_PROJEKTY\OrganSamplerC\OrganSampler\Tools\make_narration.py" "$Out\narration.txt" $Out $Ffmpeg $Voice $VoiceRate
    $narrated = Test-Path "$Out\narration.wav"
}
# The organ normalised to -18 LUFS (the master sits at -27 dB on the user's console: the raw recording peaks at -27 dB), the speech at -15.
if ($narrated) {
    & $Ffmpeg -y -loglevel error -f concat -safe 0 -i frames.txt -i audio.wav -i narration.wav -filter_complex "[1:a]loudnorm=I=-18:TP=-1.5:LRA=11,aformat=channel_layouts=stereo[org];[2:a]loudnorm=I=-15:TP=-1.5:LRA=9,aformat=channel_layouts=stereo,asplit=2[nar1][nar2];[org][nar1]sidechaincompress=threshold=0.02:ratio=4:attack=40:release=700[duck];[duck][nar2]amix=inputs=2:duration=first:dropout_transition=0:normalize=0[a]" -map 0:v -map "[a]" -c:v libx264 -preset slow -crf 20 -pix_fmt yuv420p -vf "scale=1920:-2" -r 30 -c:a aac -b:a 192k -shortest organsampler.mp4
} else {
    & $Ffmpeg -y -loglevel error -f concat -safe 0 -i frames.txt -i audio.wav -c:v libx264 -preset slow -crf 20 -pix_fmt yuv420p -vf "scale=1920:-2" -r 30 -af "loudnorm=I=-16:TP=-1.5:LRA=11" -c:a aac -b:a 192k -shortest organsampler.mp4
}
& $Ffmpeg -y -loglevel error -i organsampler.mp4 -vf "fps=8,scale=960:-1:flags=lanczos,split[s0][s1];[s0]palettegen=max_colors=128[p];[s1][p]paletteuse=dither=bayer:bayer_scale=3" -loop 0 organsampler.gif
Pop-Location
Get-ChildItem $Out -Filter "organsampler.*" | Select-Object Name, Length
