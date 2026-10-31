// X11 only - no JUCE header in this file (see LinuxWindowPlacement.h).
#include "LinuxWindowPlacement.h"

#if defined(__linux__) && !defined(__ANDROID__)
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

namespace
{
    std::string lowered(std::string s)
    {
        for (auto& c : s) c = (char)std::tolower((unsigned char)c);
        return s;
    }

    bool containsIgnoreCase(const std::string& haystack, const std::string& needle)
    {
        return !needle.empty() && lowered(haystack).find(lowered(needle)) != std::string::npos;
    }

    std::string windowName(Display* display, Window window)
    {
        const Atom utf8 = XInternAtom(display, "UTF8_STRING", False), netName = XInternAtom(display, "_NET_WM_NAME", False);
        Atom type = 0; int format = 0; unsigned long count = 0, after = 0; unsigned char* data = nullptr;
        if (XGetWindowProperty(display, window, netName, 0, 1024, False, utf8, &type, &format, &count, &after, &data) == Success && data != nullptr)
        {
            const std::string name((const char*)data, (size_t)count);
            XFree(data);
            if (!name.empty()) return name;
        }
        XTextProperty text{};
        if (XGetWMName(display, window, &text) && text.value != nullptr)
        {
            const std::string name((const char*)text.value);
            XFree(text.value);
            return name;
        }
        return {};
    }

    std::string windowClass(Display* display, Window window)
    {
        XClassHint hint{};
        if (!XGetClassHint(display, window, &hint)) return {};
        std::string result;
        if (hint.res_class != nullptr) { result += hint.res_class; XFree(hint.res_class); }
        if (hint.res_name != nullptr) { result += " "; result += hint.res_name; XFree(hint.res_name); }
        return lowered(result);
    }

    void sendToRoot(Display* display, XEvent& event)
    {
        XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
    }

    // Whether the window manager announces an EWMH message in _NET_SUPPORTED.
    bool wmSupports(Display* display, const char* atomName)
    {
        const Atom supported = XInternAtom(display, "_NET_SUPPORTED", False), wanted = XInternAtom(display, atomName, False);
        Atom type = 0; int format = 0; unsigned long count = 0, after = 0; unsigned char* data = nullptr;
        bool found = false;
        if (XGetWindowProperty(display, DefaultRootWindow(display), supported, 0, 4096, False, XA_ATOM, &type, &format, &count, &after, &data) == Success && data != nullptr)
        {
            for (unsigned long i = 0; i < count && !found; ++i) found = ((Atom*)data)[i] == wanted;
            XFree(data);
        }
        return found;
    }

    void placeWindow(Display* display, Window window, LinuxWindowPlacement::Box r, bool ewmh)
    {
        // Out of the maximised state first (a maximised window ignores a move).
        XEvent state{};
        state.xclient.type = ClientMessage;
        state.xclient.window = window;
        state.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", False);
        state.xclient.format = 32;
        state.xclient.data.l[0] = 0;   // _NET_WM_STATE_REMOVE
        state.xclient.data.l[1] = (long)XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
        state.xclient.data.l[2] = (long)XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
        state.xclient.data.l[3] = 2;   // the source: a pager/user action
        sendToRoot(display, state);
        XEvent move{};
        move.xclient.type = ClientMessage;
        move.xclient.window = window;
        move.xclient.message_type = XInternAtom(display, "_NET_MOVERESIZE_WINDOW", False);
        move.xclient.format = 32;
        move.xclient.data.l[0] = (1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (2 << 12);   // x, y, width, height given; source: user
        move.xclient.data.l[1] = r.x;
        move.xclient.data.l[2] = r.y;
        move.xclient.data.l[3] = r.width;
        move.xclient.data.l[4] = r.height;
        // A manager with _NET_MOVERESIZE_WINDOW gets the message alone (0.8.149):
        // moving the client window as well under KWin left the frame and the
        // client at odds, the windows overlapping (#58). Without EWMH the plain move.
        if (ewmh) sendToRoot(display, move);
        else XMoveResizeWindow(display, window, r.x, r.y, (unsigned)std::max(1, r.width), (unsigned)std::max(1, r.height));
        XRaiseWindow(display, window);
    }
}

namespace
{
    std::string shellQuote(const std::string& s)
    {
        std::string out = "'";
        for (const char c : s) { if (c == '\'') out += "'\\''"; else out += c; }
        return out + "'";
    }

    std::string runCapture(const std::string& command)
    {
        std::string out;
        if (FILE* p = popen((command + " 2>&1").c_str(), "r"))
        {
            char buffer[512];
            while (fgets(buffer, sizeof(buffer), p) != nullptr) out += buffer;
            pclose(p);
        }
        return out;
    }

    // KDE Plasma on Wayland (0.8.150): the X11 client list shows XWayland windows
    // only - Dolphin and a Wayland browser are not in it. KWin's scripting
    // (D-Bus org.kde.KWin /Scripting) sees every window: a script finds the file
    // manager by the folder in its caption (or by its class) and the browser by
    // its class, un-maximises them and sets their frames. Plasma 6 and 5 API.
    bool placeThroughKWin(const std::string& folderName, LinuxWindowPlacement::Box left, LinuxWindowPlacement::Box right, LinuxWindowPlacement::Box box, std::string& note)
    {
        if (runCapture("gdbus introspect --session --dest org.kde.KWin --object-path /Scripting").find("loadScript") == std::string::npos) return false;
        std::string js;
        auto num = [](int v) { return std::to_string(v); };
        std::string folder;
        for (const char c : folderName) { if (c == '"' || c == '\\') folder += '\\'; folder += c; }
        js += "var folder = \"" + folder + "\".toLowerCase();\n";
        js += "var browsers = ['firefox','chrom','brave','vivaldi','opera','epiphany','falkon','konqueror','midori','librewolf','edge'];\n";
        js += "var managers = ['dolphin','nautilus','thunar','pcmanfm','nemo','caja','spacefm','krusader','doublecmd','org.gnome.files'];\n";
        js += "var list = (typeof workspace.windowList === 'function') ? workspace.windowList() : workspace.clientList();\n";
        js += "var fm = null, br = null;\n";
        js += "function cls(w) { return ((w.resourceClass || '') + ' ' + (w.resourceName || '')).toString().toLowerCase(); }\n";
        js += "for (var i = 0; i < list.length; ++i) { var w = list[i]; if (!w || w.specialWindow || cls(w).indexOf('libreorgan') >= 0) continue;\n";
        js += "  if (!fm && (w.caption || '').toString().toLowerCase().indexOf(folder) >= 0) { fm = w; continue; }\n";
        js += "  if (!br) for (var b = 0; b < browsers.length; ++b) if (cls(w).indexOf(browsers[b]) >= 0) { br = w; break; } }\n";
        js += "if (!fm) for (var i = 0; i < list.length; ++i) { var w = list[i]; if (!w || w === br || w.specialWindow) continue;\n";
        js += "  for (var m = 0; m < managers.length; ++m) if (cls(w).indexOf(managers[m]) >= 0) { fm = w; break; } if (fm) break; }\n";
        js += "function place(w, x, y, wd, ht) { try { if (typeof w.setMaximize === 'function') w.setMaximize(false, false); } catch (e) {}\n";
        js += "  if ('frameGeometry' in w) w.frameGeometry = { x: x, y: y, width: wd, height: ht }; else w.geometry = { x: x, y: y, width: wd, height: ht };\n";
        js += "  if ('activeWindow' in workspace) workspace.activeWindow = w; else workspace.activeClient = w; }\n";
        js += "if (fm) place(fm, " + num(right.x) + ", " + num(right.y) + ", " + num(right.width) + ", " + num(right.height) + ");\n";
        js += "if (br) place(br, " + num(left.x) + ", " + num(left.y) + ", " + num(left.width) + ", " + num(left.height) + ");\n";
        if (box.width > 0)
        {
            // The program's own box: KWin put it where its placement policy likes (the top
            // left corner), and an XWayland client cannot move itself - the script can.
            js += "for (var i = 0; i < list.length; ++i) { var w = list[i]; if (w && cls(w).indexOf('libreorgan') >= 0 && (w.caption || '').toString() == 'Report a problem')\n";
            js += "  { w.frameGeometry = { x: " + num(box.x) + ", y: " + num(box.y) + ", width: " + num(box.width) + ", height: " + num(box.height) + " }; if ('activeWindow' in workspace) workspace.activeWindow = w; } }\n";
        }
        js += "print('LibreOrgan Sampler report windows: file manager ' + (fm ? fm.caption : 'none') + ', browser ' + (br ? br.caption : 'none'));\n";
        const std::string path = std::string(getenv("HOME") ? getenv("HOME") : "/tmp") + "/.config/LibreOrgan/kwin-report-windows.js";
        { std::ofstream out(path); if (!out) return false; out << js; }
        // A script of this name still loaded (a run that never reached the unload) makes
        // loadScript answer -1: unload first.
        runCapture("gdbus call --session --dest org.kde.KWin --object-path /Scripting --method org.kde.kwin.Scripting.unloadScript libreorgan-report-windows");
        const std::string loaded = runCapture("gdbus call --session --dest org.kde.KWin --object-path /Scripting --method org.kde.kwin.Scripting.loadScript "
            + shellQuote(path) + " libreorgan-report-windows");
        // gdbus prints the int32 reply as "(7,)" (0.8.151: 0.8.150 looked for "int32 7" and
        // took every answer for a refusal); -1 = refused
        int id = -1;
        {
            auto p = loaded.find('(');
            if (p != std::string::npos)
            {
                ++p;
                while (p < loaded.size() && (loaded[p] == ' ' || isalpha((unsigned char)loaded[p]))) ++p;
                if (p < loaded.size() && (loaded[p] == '-' || isdigit((unsigned char)loaded[p]))) id = atoi(loaded.c_str() + p);
            }
        }
        if (id < 0) { note = "KWin scripting refused the script: " + loaded.substr(0, 120); return false; }
        runCapture("gdbus call --session --dest org.kde.KWin --object-path /Scripting/Script" + std::to_string(id) + " --method org.kde.kwin.Script.run");
        runCapture("gdbus call --session --dest org.kde.KWin --object-path /Scripting --method org.kde.kwin.Scripting.unloadScript libreorgan-report-windows");
        note = "Report windows (KWin scripting, Wayland): script " + std::to_string(id) + " run; left " + num(left.x) + "," + num(left.y) + " " + num(left.width) + "x" + num(left.height)
            + ", right " + num(right.x) + "," + num(right.y) + " " + num(right.width) + "x" + num(right.height) + " (what it found is in KWin's own log: journalctl --user -u plasma-kwin_wayland)";
        return true;
    }
}

bool LinuxWindowPlacement::placeSideBySide(const std::string& folderName, Box left, Box right, Box box, std::string& note)
{
    // A Wayland session (KDE): the X11 list below holds the XWayland windows only -
    // KWin's scripting places the native ones. The X11 way stays for X11 sessions.
    std::string kwinNote;
    if (const char* session = getenv("XDG_SESSION_TYPE"); session != nullptr && std::string(session) == "wayland")
        if (placeThroughKWin(folderName, left, right, box, kwinNote)) { note = kwinNote; return true; }
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) { note = "Report windows (X11): no display"; return false; }
    const Window root = DefaultRootWindow(display);
    std::vector<Window> windows;
    {
        const Atom clientList = XInternAtom(display, "_NET_CLIENT_LIST", False);
        Atom type = 0; int format = 0; unsigned long count = 0, after = 0; unsigned char* data = nullptr;
        if (XGetWindowProperty(display, root, clientList, 0, 4096, False, XA_WINDOW, &type, &format, &count, &after, &data) == Success && data != nullptr)
        {
            for (unsigned long i = 0; i < count; ++i) windows.push_back(((Window*)data)[i]);
            XFree(data);
        }
    }
    Window active = 0;
    {
        const Atom activeAtom = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
        Atom type = 0; int format = 0; unsigned long count = 0, after = 0; unsigned char* data = nullptr;
        if (XGetWindowProperty(display, root, activeAtom, 0, 1, False, XA_WINDOW, &type, &format, &count, &after, &data) == Success && data != nullptr)
        {
            if (count > 0) active = ((Window*)data)[0];
            XFree(data);
        }
    }
    static const char* browsers[]{ "firefox", "chrom", "brave", "vivaldi", "opera", "epiphany", "falkon", "konqueror", "midori", "librewolf", "edge" };
    static const char* fileManagers[]{ "dolphin", "nautilus", "thunar", "pcmanfm", "nemo", "caja", "spacefm", "krusader", "doublecmd", "org.gnome.files" };
    Window fileManager = 0, browser = 0;
    std::string seen;
    for (const Window w : windows)
    {
        const std::string name = windowName(display, w), cls = windowClass(display, w);
        if (cls.find("libreorgan") != std::string::npos) continue;
        if (seen.size() < 600) seen += " [" + cls.substr(0, 24) + ": " + name.substr(0, 40) + "]";
        if (fileManager == 0 && containsIgnoreCase(name, folderName)) { fileManager = w; continue; }
        if (browser == 0)
            for (const char* b : browsers) if (cls.find(b) != std::string::npos) { browser = w; break; }
    }
    // The folder's window not found by its title (a file manager that shows the
    // folder name differently): the first file manager by its class (0.8.149).
    if (fileManager == 0)
        for (const Window w : windows)
        {
            const std::string cls = windowClass(display, w);
            if (cls.find("libreorgan") != std::string::npos || w == browser) continue;
            bool fm = false;
            for (const char* f : fileManagers) if (cls.find(f) != std::string::npos) fm = true;
            if (fm) { fileManager = w; break; }
        }
    if (browser == 0 && active != 0 && active != fileManager && windowClass(display, active).find("libreorgan") == std::string::npos) browser = active;
    const bool ewmh = wmSupports(display, "_NET_MOVERESIZE_WINDOW");
    if (fileManager != 0) placeWindow(display, fileManager, right, ewmh);
    if (browser != 0) placeWindow(display, browser, left, ewmh);
    XFlush(display);
    XCloseDisplay(display);
    note = std::string("Report windows (X11, ") + (ewmh ? "EWMH" : "no EWMH") + "): file manager " + (fileManager != 0 ? "placed" : "not found") + ", browser " + (browser != 0 ? "placed" : "not found")
        + "; left " + std::to_string(left.x) + "," + std::to_string(left.y) + " " + std::to_string(left.width) + "x" + std::to_string(left.height)
        + ", right " + std::to_string(right.x) + "," + std::to_string(right.y) + " " + std::to_string(right.width) + "x" + std::to_string(right.height)
        + "; windows seen:" + seen;
    if (!kwinNote.empty()) note += "; " + kwinNote;
    return fileManager != 0 || browser != 0;
}

bool LinuxWindowPlacement::windowSize(unsigned long window, int& width, int& height)
{
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr || window == 0) return false;
    Window root = 0; int x = 0, y = 0; unsigned int w = 0, h = 0, border = 0, depth = 0;
    const bool ok = XGetGeometry(display, (Drawable)window, &root, &x, &y, &w, &h, &border, &depth) != 0;
    XCloseDisplay(display);
    if (!ok) return false;
    width = (int)w;
    height = (int)h;
    return true;
}

void LinuxWindowPlacement::hintMaximised(unsigned long window)
{
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr || window == 0) return;
    const Atom state = XInternAtom(display, "_NET_WM_STATE", False);
    Atom atoms[2] = { XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_VERT", False), XInternAtom(display, "_NET_WM_STATE_MAXIMIZED_HORZ", False) };
    XChangeProperty(display, (Window)window, state, XA_ATOM, 32, PropModeReplace, (unsigned char*)atoms, 2);
    XFlush(display);
    XCloseDisplay(display);
}

void LinuxWindowPlacement::setTransientFor(unsigned long window, unsigned long parent)
{
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr || window == 0 || parent == 0) return;
    XSetTransientForHint(display, (Window)window, (Window)parent);
    XFlush(display);
    XCloseDisplay(display);
}

bool LinuxWindowPlacement::waylandSession()
{
    const char* session = getenv("XDG_SESSION_TYPE");
    return session != nullptr && std::string(session) == "wayland";
}
#else
bool LinuxWindowPlacement::placeSideBySide(const std::string&, Box, Box, Box, std::string&) { return false; }
bool LinuxWindowPlacement::windowSize(unsigned long, int&, int&) { return false; }
void LinuxWindowPlacement::hintMaximised(unsigned long) {}
void LinuxWindowPlacement::setTransientFor(unsigned long, unsigned long) {}
bool LinuxWindowPlacement::waylandSession() { return false; }
#endif
