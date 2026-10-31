# A synthetic sample tree with velocity layers for the self-test (any platform, pwsh or Windows PowerShell):
#   <temp>/vel_test/Principal8/0NN-X.wav      plain sine, amplitude 0.5   (every velocity above the layers)
#   <temp>/vel_test/Principal8/0NN-X_070.wav  the same sine at 0.125 (-12 dB): velocities up to 70
#   036-C_025.wav                              0.03: velocities up to 25
#   049-C#_070.wav only (no plain file): the layer must serve the whole velocity range
$tree = Join-Path ([IO.Path]::GetTempPath()) 'vel_test'
$root = Join-Path $tree 'Principal8'
if (Test-Path $tree) { Remove-Item $tree -Recurse -Force }
New-Item -ItemType Directory -Force $root | Out-Null
$names = 'C','C#','D','D#','E','F','F#','G','G#','A','A#','B'
function Write-Sine($path, $midi, $amplitude, $seconds, $rate)
{
    $freq = 440.0 * [Math]::Pow(2.0, ($midi - 69) / 12.0)
    $frames = [int]($seconds * $rate)
    $data = New-Object byte[] ($frames * 2)
    for ($i = 0; $i -lt $frames; $i++)
    {
        $env1 = [Math]::Min(1.0, $i / ($rate * 0.01))                        # 10 ms fade in
        $env2 = [Math]::Min(1.0, ($frames - 1 - $i) / ($rate * 0.05))        # 50 ms fade out
        $v = [int]([Math]::Round(32767.0 * $amplitude * $env1 * $env2 * [Math]::Sin(2.0 * [Math]::PI * $freq * $i / $rate)))
        $data[2 * $i] = [byte]($v -band 0xFF); $data[2 * $i + 1] = [byte](($v -shr 8) -band 0xFF)
    }
    $fs = [IO.File]::Create($path); $w = New-Object IO.BinaryWriter($fs)
    $w.Write([Text.Encoding]::ASCII.GetBytes('RIFF')); $w.Write([int](36 + $data.Length)); $w.Write([Text.Encoding]::ASCII.GetBytes('WAVE'))
    $w.Write([Text.Encoding]::ASCII.GetBytes('fmt ')); $w.Write([int]16); $w.Write([int16]1); $w.Write([int16]1); $w.Write([int]$rate); $w.Write([int]($rate * 2)); $w.Write([int16]2); $w.Write([int16]16)
    $w.Write([Text.Encoding]::ASCII.GetBytes('data')); $w.Write([int]$data.Length); $w.Write($data)
    $w.Close(); $fs.Close()
}
for ($midi = 36; $midi -le 48; $midi++)
{
    $name = ('{0:000}-{1}' -f $midi, $names[$midi % 12])
    Write-Sine (Join-Path $root "$name.wav") $midi 0.5 1.5 48000
    Write-Sine (Join-Path $root "$($name)_070.wav") $midi 0.125 1.5 48000
}
Write-Sine (Join-Path $root '036-C_025.wav') 36 0.03 1.5 48000
Write-Sine (Join-Path $root '049-C#_070.wav') 49 0.125 1.5 48000
# Releases with velocity layers: plain 0.5 and _070 at 0.125 (no _025 release: the _025 attack takes the _070 release, the nearest).
$rel = Join-Path $root 'rel'
New-Item -ItemType Directory -Force $rel | Out-Null
for ($midi = 36; $midi -le 48; $midi++)
{
    $name = ('{0:000}-{1}' -f $midi, $names[$midi % 12])
    Write-Sine (Join-Path $rel "$name.wav") $midi 0.5 0.4 48000
    Write-Sine (Join-Path $rel "$($name)_070.wav") $midi 0.125 0.4 48000
}
(Get-ChildItem $root | Measure-Object).Count.ToString() + ' files in ' + $root
