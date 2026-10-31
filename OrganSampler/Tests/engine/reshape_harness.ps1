# Standalone check of SamplerEngine.cpp's reshapeRelease (the release copies of
# the pitch shift): the function is cut out of the source, compiled with cl.exe
# against small shims of the JUCE helpers it uses, and run on a synthetic
# release (harmonics + noise with a known decay, 50 ms end fade) for three
# cases: +12 with a steep room (the gain cap is reached), +12 gentle, -24.
# Prints the level profile (dB re the first 100 ms) of the source, the
# resampled material and the copy: the copy must follow the source within a
# few dB. No app involved - safe to run any time.
$src = Get-Content "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Source\SamplerEngine.cpp" -Raw
$start = $src.IndexOf('    void reshapeRelease('); $end = $src.IndexOf('    std::unique_ptr<Sample> makeResampledPitchCopy(')
if ($start -lt 0 -or $end -lt 0) { 'reshapeRelease not found'; exit 1 }
$fn = $src.Substring($start, $end - $start)
$dir = "$env:TEMP\reshape_test"; New-Item -ItemType Directory -Force $dir | Out-Null
$shim = @'
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <random>
namespace juce {
template <typename T> T jmax(T a, T b) { return a > b ? a : b; }
template <typename T> T jmin(T a, T b) { return a < b ? a : b; }
template <typename T> T jlimit(T lo, T hi, T v) { return v < lo ? lo : (v > hi ? hi : v); }
template <typename T> struct MathConstants { static constexpr T pi = (T)3.14159265358979323846; };
struct Random { std::mt19937 g; Random(int64_t s) : g((unsigned)s) {} int nextInt(int n) { return n <= 1 ? 0 : (int)(g() % (unsigned)n); } };
}
'@
$main = @'
static void runCase(const char* name, double ratio, bool steep)
{
    const double sr = 48000.0;
    const int frames = (int)(3.0 * sr);
    const int natural = (int)(frames / ratio), target = frames;
    std::mt19937 g(1); std::normal_distribution<double> n(0.0, 1.0);
    auto envDb = [&](double t) { return steep ? (t < 0.1 ? -200.0 * t : -20.0 - 45.0 * (t - 0.1)) : (t < 0.4 ? -30.0 * t : -12.0 - 18.0 * (t - 0.4)); };
    std::vector<std::vector<float>> src(2, std::vector<float>(frames));
    for (int i = 0; i < frames; ++i)
    {
        const double t = i / sr;
        double v = 0.3 * std::sin(2 * 3.14159265 * 220 * t) + 0.2 * std::sin(2 * 3.14159265 * 440 * t + 1.0) + 0.1 * n(g);
        double fade = i > frames - 2400 ? (double)(frames - i) / 2400.0 : 1.0;
        const float s = (float)(v * std::pow(10.0, envDb(t) / 20.0) * fade);
        src[0][i] = s; src[1][i] = s * 0.9f;
    }
    std::vector<std::vector<float>> out(2, std::vector<float>(natural));
    for (int ch = 0; ch < 2; ++ch) for (int i = 0; i < natural; ++i) out[ch][i] = src[ch][std::min(frames - 1, (int)(i * ratio))];
    auto profile = [&](const std::vector<std::vector<float>>& s, const char* what)
    {
        auto rmsDb = [&](double from, double to) { int a = (int)(from * s[0].size()), b = std::max(a + 1, (int)(to * s[0].size())); double p = 0; for (int ch = 0; ch < 2; ++ch) for (int i = a; i < b && i < (int)s[ch].size(); ++i) p += s[ch][i] * s[ch][i]; return 10.0 * std::log10(std::max(1e-12, p / ((b - a) * 2.0))); };
        const double head = rmsDb(0.0, 0.1 * sr / s[0].size());
        std::printf("  %s (%7d frames): ", what, (int)s[0].size());
        for (double at : { 0.1, 0.25, 0.4, 0.5, 0.6, 0.75, 0.9, 0.95 }) std::printf("%6.1f ", rmsDb(at - 0.02, at + 0.02) - head);
        std::printf("\n");
    };
    std::printf("%s\n", name);
    profile(src, "source  ");
    profile(out, "material");
    reshapeRelease(out, natural, target, sr);
    profile(out, "copy    ");
}
int main()
{
    runCase("+12 steep room (-45 dB/s, the gain cap is reached)", 2.0, true);
    runCase("+12 gentle room (-18 dB/s)", 2.0, false);
    runCase("-24 (the material four times as long, cut)", 0.25, false);
    return 0;
}
'@
[IO.File]::WriteAllText("$dir\reshape_test.cpp", $shim + "`n" + $fn + "`n" + $main)
$dev = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
cmd /c "`"$dev`" -arch=amd64 -no_logo >nul 2>&1 && cd /d `"$dir`" && cl /nologo /EHsc /O2 /std:c++17 reshape_test.cpp > cl.log 2>&1"
$errors = Get-Content "$dir\cl.log" | Select-String -Pattern 'error'
if ($errors) { $errors | Select-Object -First 10; exit 1 }
& "$dir\reshape_test.exe"
