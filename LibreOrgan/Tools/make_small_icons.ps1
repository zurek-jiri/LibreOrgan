# Small icon sizes (16 and 24 px) made for legibility from the photographic
# 1024 px icon (LibreOrgan_icon_1024.png): the three pipes are cut out, made
# much wider and set further apart, their mouth band (the black cut) is made
# taller, the contrast and the twist bands are strengthened, then the composite
# is downscaled. The other sizes (256/128/64/48/32) come unchanged from
# LibreOrgan_icon.ico (the 7-size icon; Projucer regenerates icon.ico with
# 4 plain downscales at every resave, so this script runs after each resave).
# Output: Builds\VisualStudio2026\icon.ico and LibreOrgan_icon.ico (both 7 sizes).
param([double]$WidthFactor = 2.1, [double]$BandFactor = 2.0, [double]$Contrast = 1.3, [double]$Twist = 2.2)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$root = Split-Path $PSScriptRoot -Parent
$source = Join-Path $root 'LibreOrgan_icon_1024.png'
$full = Join-Path $root 'LibreOrgan_icon.ico'
$target = Join-Path $root 'Builds\VisualStudio2026\icon.ico'
if (-not (Test-Path $source)) { throw "missing $source" }
if (-not (Test-Path $full)) { throw "missing $full (the 7-size icon)" }

$code = @'
using System; using System.Drawing; using System.Drawing.Imaging; using System.Runtime.InteropServices; using System.IO; using System.Collections.Generic;
public static class IconWork
{
    static int Clamp(double v) { return v < 0 ? 0 : v > 255 ? 255 : (int)Math.Round(v); }
    // The pipes of the 1024 photo: columns [x0,x1), nominal left-to-right order.
    public static Bitmap Compose(Bitmap src, int[][] pipeColumns, double widthFactor, double bandFactor, double contrast, double twist)
    {
        int w = src.Width, h = src.Height;
        var rect = new Rectangle(0, 0, w, h);
        var data = src.LockBits(rect, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
        var px = new byte[data.Stride * h]; Marshal.Copy(data.Scan0, px, 0, px.Length); src.UnlockBits(data);
        int stride = data.Stride;
        // Background per row from a column left of the pipes (the gradient is horizontal-uniform away from the corners).
        var bg = new byte[h][];
        for (int y = 0; y < h; ++y) { int o = y * stride + 120 * 4; bg[y] = new byte[] { px[o], px[o + 1], px[o + 2] }; }
        // A clean background: the original with the pipe columns refilled by the row background.
        var canvas = new Bitmap(w, h, PixelFormat.Format32bppArgb);
        var cd = canvas.LockBits(rect, ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
        var cpx = new byte[cd.Stride * h]; Array.Copy(px, cpx, px.Length);
        foreach (var cols in pipeColumns)
            for (int y = 40; y < h - 40; ++y)
                for (int x = cols[0] - 12; x < cols[1] + 12; ++x)
                { int o = y * cd.Stride + x * 4; cpx[o] = bg[y][0]; cpx[o + 1] = bg[y][1]; cpx[o + 2] = bg[y][2]; cpx[o + 3] = 255; }
        Marshal.Copy(cpx, 0, cd.Scan0, cpx.Length); canvas.UnlockBits(cd);
        // Every pipe: cut out (alpha from the difference to the background), contrast and local
        // contrast (the twist bands), the mouth band taller; placed wider and further apart.
        double[] centres = { 0.22, 0.5, 0.78 };
        using (var g = Graphics.FromImage(canvas))
        {
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
            g.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.HighQuality;
            for (int p = 0; p < pipeColumns.Length; ++p)
            {
                int x0 = pipeColumns[p][0], x1 = pipeColumns[p][1], pw = x1 - x0;
                var pipe = new Bitmap(pw, h, PixelFormat.Format32bppArgb);
                var pd = pipe.LockBits(new Rectangle(0, 0, pw, h), ImageLockMode.WriteOnly, PixelFormat.Format32bppArgb);
                var ppx = new byte[pd.Stride * h];
                int top = h, bottom = 0;
                var lum = new double[h, pw];
                for (int y = 0; y < h; ++y)
                    for (int x = 0; x < pw; ++x)
                    {
                        int o = y * stride + (x0 + x) * 4;
                        double d = Math.Abs(px[o] - bg[y][0]) + Math.Abs(px[o + 1] - bg[y][1]) + Math.Abs(px[o + 2] - bg[y][2]);
                        lum[y, x] = (px[o] + px[o + 1] + px[o + 2]) / 3.0;
                        if (d > 60) { if (y < top) top = y; if (y > bottom) bottom = y; }
                    }
                // Mouth band: the lowest run of pure black rows (the cut itself, not the
                // shaded cavity above it) in the middle column; made bandFactor times taller.
                int bandTop = -1, bandBottom = -1; int mid = pw / 2;
                for (int y = bottom; y >= top; --y)
                {
                    bool black = lum[y, mid] < 14 && lum[y, mid - 6] < 14 && lum[y, mid + 6] < 14;
                    if (black) { if (bandBottom < 0) bandBottom = y; bandTop = y; }
                    else if (bandBottom >= 0) break;
                }
                // At least 12 % of the icon's height: two clear rows even at 16 px.
                int newBandTop = bandTop >= 0 ? (int)(bandBottom - Math.Max((bandBottom - bandTop) * bandFactor, h * 0.12)) : -1;
                for (int y = 0; y < h; ++y)
                    for (int x = 0; x < pw; ++x)
                    {
                        int o = y * stride + (x0 + x) * 4, q = y * pd.Stride + x * 4;
                        double d = Math.Abs(px[o] - bg[y][0]) + Math.Abs(px[o + 1] - bg[y][1]) + Math.Abs(px[o + 2] - bg[y][2]);
                        double alpha = Math.Min(1.0, Math.Max(0.0, (d - 24) / 50.0));
                        // Local contrast: the pixel against the mean of its row neighbourhood (the twist bands).
                        double mean = 0; int n = 0;
                        for (int k = -14; k <= 14; k += 2) { int xx = x + k; if (xx >= 0 && xx < pw) { mean += lum[y, xx]; ++n; } }
                        mean /= Math.Max(1, n);
                        double[] rgb = { px[o], px[o + 1], px[o + 2] };
                        bool band = newBandTop >= 0 && y >= newBandTop && y <= bandBottom && alpha > 0.5;
                        // Twist bands and contrast on the luminance only (the metal keeps its hue).
                        double l = lum[y, x];
                        double nl = (l - mean) * twist + mean;
                        nl = (nl - 150) * contrast + 150;
                        double ratio = l > 1 ? Math.Max(0.0, nl) / l : 1.0;
                        for (int c = 0; c < 3; ++c)
                        {
                            double v = rgb[c] * ratio;
                            if (band) v = 8;                        // the mouth cut, taller
                            rgb[c] = Clamp(v);
                        }
                        ppx[q] = (byte)rgb[0]; ppx[q + 1] = (byte)rgb[1]; ppx[q + 2] = (byte)rgb[2]; ppx[q + 3] = (byte)Clamp(alpha * 255);
                    }
                Marshal.Copy(ppx, 0, pd.Scan0, ppx.Length); pipe.UnlockBits(pd);
                int newWidth = (int)Math.Round(pw * widthFactor);
                int cx = (int)Math.Round(centres[p] * w);
                g.DrawImage(pipe, new Rectangle(cx - newWidth / 2, 0, newWidth, h), new Rectangle(0, 0, pw, h), GraphicsUnit.Pixel);
                pipe.Dispose();
            }
        }
        return canvas;
    }
    public static Bitmap Downscale(Bitmap src, int size)
    {
        var bmp = new Bitmap(size, size, PixelFormat.Format32bppArgb);
        using (var g = Graphics.FromImage(bmp))
        {
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
            g.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.HighQuality;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighQuality;
            g.DrawImage(src, new Rectangle(0, 0, size, size), new Rectangle(0, 0, src.Width, src.Height), GraphicsUnit.Pixel);
        }
        return bmp;
    }
    // A 32-bit DIB icon entry (BITMAPINFOHEADER, BGRA bottom-up, 1-bit AND mask).
    public static byte[] DibEntry(Bitmap bmp)
    {
        int w = bmp.Width, h = bmp.Height;
        var ms = new MemoryStream(); var bw = new BinaryWriter(ms);
        bw.Write(40); bw.Write(w); bw.Write(h * 2); bw.Write((short)1); bw.Write((short)32); bw.Write(0); bw.Write(w * h * 4 + ((w + 31) / 32) * 4 * h); bw.Write(0); bw.Write(0); bw.Write(0); bw.Write(0);
        for (int y = h - 1; y >= 0; --y) for (int x = 0; x < w; ++x) { var c = bmp.GetPixel(x, y); bw.Write(c.B); bw.Write(c.G); bw.Write(c.R); bw.Write(c.A); }
        int maskStride = ((w + 31) / 32) * 4;
        for (int y = h - 1; y >= 0; --y) { var row = new byte[maskStride]; for (int x = 0; x < w; ++x) if (bmp.GetPixel(x, y).A < 128) row[x / 8] |= (byte)(0x80 >> (x % 8)); bw.Write(row); }
        bw.Flush(); return ms.ToArray();
    }
    public struct Entry { public int Size; public byte[] Data; public bool Png; }
    public static List<Entry> ReadIco(string path)
    {
        var b = File.ReadAllBytes(path); var list = new List<Entry>();
        int count = BitConverter.ToUInt16(b, 4);
        for (int i = 0; i < count; ++i)
        {
            int o = 6 + 16 * i; int size = b[o] == 0 ? 256 : b[o];
            int len = BitConverter.ToInt32(b, o + 8), off = BitConverter.ToInt32(b, o + 12);
            var data = new byte[len]; Array.Copy(b, off, data, 0, len);
            list.Add(new Entry { Size = size, Data = data, Png = data.Length > 8 && data[1] == (byte)'P' && data[2] == (byte)'N' && data[3] == (byte)'G' });
        }
        return list;
    }
    public static void WriteIco(string path, List<Entry> entries)
    {
        var ms = new MemoryStream(); var bw = new BinaryWriter(ms);
        bw.Write((short)0); bw.Write((short)1); bw.Write((short)entries.Count);
        int offset = 6 + 16 * entries.Count;
        foreach (var e in entries)
        {
            bw.Write((byte)(e.Size >= 256 ? 0 : e.Size)); bw.Write((byte)(e.Size >= 256 ? 0 : e.Size)); bw.Write((byte)0); bw.Write((byte)0);
            bw.Write((short)1); bw.Write((short)32); bw.Write(e.Data.Length); bw.Write(offset); offset += e.Data.Length;
        }
        foreach (var e in entries) bw.Write(e.Data);
        bw.Flush(); File.WriteAllBytes(path, ms.ToArray());
    }
}
'@
Add-Type -TypeDefinition $code -ReferencedAssemblies System.Drawing

$src = [System.Drawing.Bitmap]::FromFile($source)
# The pipe columns of the 1024 photo (measured): left, centre, right.
$columns = [int[][]]@(@(300, 392), @(452, 562), @(626, 722))
$composite = [IconWork]::Compose($src, $columns, $WidthFactor, $BandFactor, $Contrast, $Twist)
$composite.Save((Join-Path $env:TEMP 'icon_small_composite.png'))
$entries = [IconWork]::ReadIco($full)
$result = New-Object 'System.Collections.Generic.List[IconWork+Entry]'
foreach ($e in $entries)
{
    if ($e.Size -eq 16 -or $e.Size -eq 24)
    {
        $small = [IconWork]::Downscale($composite, $e.Size)
        $small.Save((Join-Path $env:TEMP ("icon_small_{0}.png" -f $e.Size)))
        $n = New-Object 'IconWork+Entry'; $n.Size = $e.Size; $n.Data = [IconWork]::DibEntry($small); $n.Png = $false
        $result.Add($n); $small.Dispose()
    }
    else { $result.Add($e) }
}
[IconWork]::WriteIco($target, $result)
[IconWork]::WriteIco($full, $result)
$composite.Dispose(); $src.Dispose()
"written: $target and $full (" + ($result | ForEach-Object { $_.Size }) -join ',' + ")"
