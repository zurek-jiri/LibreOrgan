Add-Type -TypeDefinition @'
using System; using System.Runtime.InteropServices; using System.Collections.Generic; using System.Text;
public class W {
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint f);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint flags, uint dx, uint dy, uint data, UIntPtr extra);
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, UIntPtr extra);
  // Windows lets a process take the foreground after a key press: tap ALT first.
  public static bool Activate(IntPtr h) { keybd_event(0x12, 0, 0, UIntPtr.Zero); System.Threading.Thread.Sleep(30); bool ok = SetForegroundWindow(h); System.Threading.Thread.Sleep(30); keybd_event(0x12, 0, 2, UIntPtr.Zero); return ok; }
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public static List<IntPtr> Find(uint pid) { var list = new List<IntPtr>(); EnumWindows((h, l) => { uint p; GetWindowThreadProcessId(h, out p); if (p == pid && IsWindowVisible(h)) list.Add(h); return true; }, IntPtr.Zero); return list; }
  public static string Title(IntPtr h) { var sb = new StringBuilder(512); GetWindowText(h, sb, 512); return sb.ToString(); }
  public static void Click(int x, int y) { SetCursorPos(x, y); System.Threading.Thread.Sleep(80); mouse_event(0x0002, 0, 0, 0, UIntPtr.Zero); System.Threading.Thread.Sleep(60); mouse_event(0x0004, 0, 0, 0, UIntPtr.Zero); }
}
'@
Add-Type -AssemblyName System.Drawing

function Get-OrganWindows($procId) {
  foreach ($h in [W]::Find([uint32]$procId)) {
    $r = New-Object W+RECT; [void][W]::GetWindowRect($h, [ref]$r)
    [pscustomobject]@{ Handle = $h; Title = [W]::Title($h); X = $r.L; Y = $r.T; W = ($r.R - $r.L); H = ($r.B - $r.T) }
  }
}
function Save-WindowShot($h, $file) {
  $r = New-Object W+RECT; [void][W]::GetWindowRect($h, [ref]$r)
  $w = $r.R - $r.L; $ht = $r.B - $r.T
  if ($w -lt 20 -or $ht -lt 10) { return $null }
  $bmp = New-Object System.Drawing.Bitmap $w, $ht
  $g = [System.Drawing.Graphics]::FromImage($bmp); $dc = $g.GetHdc()
  [void][W]::PrintWindow($h, $dc, 2); $g.ReleaseHdc($dc); $g.Dispose()
  $bmp.Save($file); $bmp.Dispose(); return $file
}
# Clicks at window-relative coordinates only if that window is in the foreground.
function Click-InWindow($h, $rx, $ry) {
  [void][W]::Activate($h); Start-Sleep -Milliseconds 400
  if ([W]::GetForegroundWindow() -ne $h) { return "NOT IN FOREGROUND - click skipped" }
  $r = New-Object W+RECT; [void][W]::GetWindowRect($h, [ref]$r)
  [W]::Click($r.L + $rx, $r.T + $ry); return "clicked $($r.L + $rx),$($r.T + $ry)"
}

