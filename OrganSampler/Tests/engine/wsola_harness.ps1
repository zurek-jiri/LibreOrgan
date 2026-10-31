# Standalone check of SamplerEngine.cpp's wsolaResize (the timing of attacks and
# releases in the "pitch shift, timing kept" method): the function is cut out of
# the source, compiled with cl.exe against shims of the JUCE helpers, and run on
# a synthetic QUASI-steady pipe tone - pitch drifting through the whole sound
# (-15 cents over 3 s plus 4 Hz vibrato of 6 cents), level breathing (5 Hz, 1 dB),
# wind noise, a chiff burst at the start - compressed to 1/2 and 1/4 and stretched
# to 2x. Prints the length, the RMS envelope at quarters, the "click factor" (max
# sample jump per 10 ms window over the median, the self-test's measure) and the
# spectral flux (mean frame-to-frame change of the magnitude spectrum, re the
# input's own flux) of the result. No app involved - safe to run any time.
$src = Get-Content "F:\JUCE_PROJEKTY\LibreOrgan\LibreOrgan\Source\SamplerEngine.cpp" -Raw
$start = $src.IndexOf('    std::vector<std::vector<float>> wsolaResize('); $end = $src.IndexOf('    std::unique_ptr<Sample> makeResampledPitchCopy(')
if ($start -lt 0 -or $end -lt 0) { 'wsolaResize not found'; exit 1 }
$fn = $src.Substring($start, $end - $start)
$dir = "$env:TEMP\wsola_test"; New-Item -ItemType Directory -Force $dir | Out-Null
$shim = @'
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <random>
#include <complex>
namespace juce {
template <typename T> T jmax(T a, T b) { return a > b ? a : b; }
template <typename T> T jmin(T a, T b) { return a < b ? a : b; }
template <typename T> T jlimit(T lo, T hi, T v) { return v < lo ? lo : (v > hi ? hi : v); }
template <typename T> struct MathConstants { static constexpr T pi = (T)3.14159265358979323846; };
}
'@
$main = @'
static double clickFactor(const std::vector<float>& x, double sr)
{
    const int win = (int)(0.01 * sr);
    std::vector<double> jumps;
    for (int start = 0; start + win <= (int)x.size(); start += win)
    {
        double m = 0.0;
        for (int i = start + 1; i < start + win; ++i) m = std::max(m, (double)std::fabs(x[i] - x[i - 1]));
        jumps.push_back(m);
    }
    if (jumps.empty()) return 0.0;
    std::vector<double> sorted(jumps); std::sort(sorted.begin(), sorted.end());
    const double median = sorted[sorted.size() / 2];
    return median > 1e-9 ? *std::max_element(jumps.begin(), jumps.end()) / median : 0.0;
}
static double spectralFlux(const std::vector<float>& x, double sr)
{
    const int n = 2048, hop = 512;
    std::vector<double> previous(n / 2, 0.0);
    double flux = 0.0; int frames = 0;
    for (int start = 0; start + n <= (int)x.size(); start += hop)
    {
        std::vector<double> mag(n / 2, 0.0);
        for (int k = 1; k < n / 2; k += 4)   // a sparse DFT is enough for a trend
        {
            std::complex<double> acc(0.0, 0.0);
            for (int i = 0; i < n; ++i)
            {
                const double w = 0.5 - 0.5 * std::cos(2 * 3.14159265358979 * i / n);
                acc += std::polar(w * (double)x[start + i], -2 * 3.14159265358979 * k * i / n);
            }
            mag[k] = std::abs(acc);
        }
        if (frames > 0) { double d = 0.0, e = 0.0; for (int k = 1; k < n / 2; k += 4) { d += std::fabs(mag[k] - previous[k]); e += previous[k]; } flux += e > 1e-9 ? d / e : 0.0; }
        previous = mag; ++frames;
    }
    return frames > 1 ? flux / (frames - 1) : 0.0;
}
static void profile(const char* name, const std::vector<std::vector<float>>& s, double sr, double inFlux)
{
    auto rmsDb = [&](double from, double to) { int a = (int)(from * s[0].size()), b = std::max(a + 1, (int)(to * s[0].size())); double p = 0; for (int i = a; i < b && i < (int)s[0].size(); ++i) p += s[0][i] * s[0][i]; return 10.0 * std::log10(std::max(1e-12, p / (b - a))); };
    std::printf("  %-10s %7d frames: level at 10/25/50/75/95 %%: ", name, (int)s[0].size());
    for (double at : { 0.1, 0.25, 0.5, 0.75, 0.95 }) std::printf("%6.1f ", rmsDb(at - 0.02, at + 0.02));
    const double flux = spectralFlux(s[0], sr);
    std::printf(" click factor %.1f  spectral flux %.3f (x%.2f of the input)\n", clickFactor(s[0], sr), flux, inFlux > 1e-9 ? flux / inFlux : 0.0);
}
int main()
{
    const double sr = 48000.0;
    const int frames = (int)(3.0 * sr);
    const double f0 = 32.7;   // a 16' C (period 1468 frames)
    std::mt19937 g(7); std::normal_distribution<double> n(0.0, 1.0);
    std::vector<std::vector<float>> in(2, std::vector<float>(frames));
    double phase = 0.0;
    for (int i = 0; i < frames; ++i)
    {
        const double t = i / sr;
        const double cents = -15.0 * t / 3.0 + 6.0 * std::sin(2 * 3.14159265 * 4.0 * t);   // drift and vibrato
        const double f = f0 * std::pow(2.0, cents / 1200.0);
        phase += 2 * 3.14159265 * f / sr;
        double v = 0.0;
        for (int h = 1; h <= 12; ++h) v += std::sin(h * phase + 0.7 * h) / (double)h;   // a reedy spectrum
        const double breathe = std::pow(10.0, 0.5 * std::sin(2 * 3.14159265 * 5.0 * t) / 20.0);   // +-0.5 dB at 5 Hz
        const double chiff = t < 0.08 ? 0.6 * n(g) * (1.0 - t / 0.08) : 0.0;
        const double onset = t < 0.15 ? t / 0.15 : 1.0;
        const double wind = 0.02 * n(g);
        const float s = (float)(0.3 * v * breathe * onset + chiff + wind);
        in[0][i] = s; in[1][i] = s * 0.95f + (float)(0.01 * n(g));
    }
    const int period = (int)std::llround(sr / f0);
    const double inFlux = spectralFlux(in[0], sr);
    std::printf("input: 16' C, pitch drifting -15 cents over 3 s with 4 Hz vibrato, level breathing, chiff, wind\n");
    profile("input", in, sr, inFlux);
    for (double factor : { 0.5, 0.25, 2.0 })
    {
        const int outFrames = (int)(frames * factor);
        std::vector<std::pair<int, int>> positions;
        auto out = wsolaResize(in, frames, outFrames, period, &positions);
        char name[32]; std::snprintf(name, sizeof(name), "x%.2f", factor);
        profile(name, out, sr, inFlux);
    }
    return 0;
}
'@
[IO.File]::WriteAllText("$dir\wsola_test.cpp", $shim + "`n" + $fn + "`n" + $main)
$dev = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
cmd /c "`"$dev`" -arch=amd64 -no_logo >nul 2>&1 && cd /d `"$dir`" && cl /nologo /EHsc /O2 /std:c++17 wsola_test.cpp > cl.log 2>&1"
$errors = Get-Content "$dir\cl.log" | Select-String -Pattern 'error'
if ($errors) { $errors | Select-Object -First 10; exit 1 }
& "$dir\wsola_test.exe"
