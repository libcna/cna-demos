// SPDX-License-Identifier: MS-PL
// The native window behind GameWindow: XNA's Handle, and CNA's typed NativeWindowHandle, for code
// that hands the window to a native library (a video overlay, a GPU API, a platform dialog).
#include <cstdint>
#include <format>
#include <string>

#include "CNA/Platform/NativeWindowHandle.hpp"
#include "CNA/Platform/NativeWindowSystem.hpp"
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/GameWindow.hpp"
#include "Microsoft/Xna/Framework/Input/Mouse.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"

using namespace Microsoft::Xna::Framework;
using Microsoft::Xna::Framework::Input::Mouse;
using Microsoft::Xna::Framework::Input::Touch::TouchPanel;
namespace Platform = CNA::Platform;

namespace
{
    std::string Pointer(const void* p) { return std::format("{:#x}", reinterpret_cast<std::uintptr_t>(p)); }

    class NativeHandleDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            const GameWindow& window = getWindowProperty();

            // XNA's GameWindow.Handle: one pointer-sized value (an IntPtr).
            const auto legacy = window.getHandleProperty();
            ctx.Print("Handle = {:#x}", static_cast<std::uint64_t>(legacy));
            ctx.Check(legacy != 0, "GameWindow::Handle is set once the window exists");
            ctx.Check(Mouse::getWindowHandleProperty() == legacy,
                      "Mouse.WindowHandle targets the GameWindow created for this game");
            Mouse::setWindowHandleProperty(0);
            const bool cleared = Mouse::getWindowHandleProperty() == 0;
            Mouse::setWindowHandleProperty(legacy);
            ctx.Check(cleared && Mouse::getWindowHandleProperty() == legacy,
                      "Mouse.WindowHandle accepts a cleared target and restores the real window token");

            const auto previousTouchTarget = TouchPanel::getWindowHandleProperty();
            TouchPanel::setWindowHandleProperty(legacy);
            const bool touchTargetsWindow = TouchPanel::getWindowHandleProperty() == legacy;
            TouchPanel::setWindowHandleProperty(0);
            const bool touchCleared = TouchPanel::getWindowHandleProperty() == 0;
            TouchPanel::setWindowHandleProperty(previousTouchTarget);
            ctx.Check(touchTargetsWindow && touchCleared &&
                          TouchPanel::getWindowHandleProperty() == previousTouchTarget,
                      "TouchPanel.WindowHandle accepts the real window token, clears and restores prior state");

            // CNA's handle says which window system the values belong to. Never assume: on Linux
            // the same game runs on X11 or Wayland.
            const Platform::NativeWindowHandle native = window.GetNativeWindowHandleEXT();
            ctx.Print("window system: {}", Platform::ToString(native.system));
            ctx.Check(native.system != Platform::NativeWindowSystem::Unknown, "the window system is known");

            // The typed accessors check the system and the fields it needs before handing them out.
            bool typed = false;
            switch (native.system)
            {
            case Platform::NativeWindowSystem::X11:
            {
                Platform::X11NativeWindow x11;
                typed = Platform::TryGetX11(native, x11);
                ctx.Print("X11: Display* {}, Window XID {:#x}", Pointer(x11.display), x11.window);
                break;
            }
            case Platform::NativeWindowSystem::Wayland:
            {
                Platform::WaylandNativeWindow wayland;
                typed = Platform::TryGetWayland(native, wayland);
                ctx.Print("Wayland: wl_display* {}, wl_surface* {}", Pointer(wayland.display),
                          Pointer(wayland.surface));
                break;
            }
            case Platform::NativeWindowSystem::Win32:
            {
                Platform::Win32NativeWindow win32;
                typed = Platform::TryGetWin32(native, win32);
                ctx.Print("Win32: HWND {}", Pointer(win32.hwnd));
                break;
            }
            case Platform::NativeWindowSystem::Cocoa:
            {
                Platform::CocoaNativeWindow cocoa;
                typed = Platform::TryGetCocoa(native, cocoa);
                ctx.Print("Cocoa: NSWindow* {}", Pointer(cocoa.window));
                break;
            }
            case Platform::NativeWindowSystem::UIKit:
            {
                Platform::UIKitNativeWindow uikit;
                typed = Platform::TryGetUIKit(native, uikit);
                ctx.Print("UIKit: UIWindow* {}", Pointer(uikit.window));
                break;
            }
            case Platform::NativeWindowSystem::Android:
            {
                Platform::AndroidNativeWindow android;
                typed = Platform::TryGetAndroid(native, android);
                ctx.Print("Android: ANativeWindow* {}", Pointer(android.window));
                break;
            }
            default:
                ctx.Print("no typed accessor shown for this system");
                typed = true;
                break;
            }
            ctx.Check(typed, "the typed accessor for that system accepts the handle");

            // An accessor for another system refuses it instead of returning a wrong pointer.
            Platform::X11NativeWindow x11;
            Platform::Win32NativeWindow win32;
            const bool wrong = native.system == Platform::NativeWindowSystem::X11 ? Platform::TryGetWin32(native, win32)
                                                                                  : Platform::TryGetX11(native, x11);
            ctx.Check(!wrong, "an accessor for another window system refuses it");

            const bool expectsNativeWindow = native.system != Platform::NativeWindowSystem::Unknown &&
                                             native.system != Platform::NativeWindowSystem::Web &&
                                             native.system != Platform::NativeWindowSystem::Headless &&
                                             native.system != Platform::NativeWindowSystem::Terminal;
            const std::string description = Platform::Describe(native);
            ctx.Print("native description: {}", description);
            ctx.Check(Platform::HasNativeWindow(native) == expectsNativeWindow &&
                          description.starts_with(Platform::ToString(native.system)) &&
                          description.find("0x") == std::string::npos,
                      "native presence and diagnostic description match the real window without leaking addresses");

            // Synthetic handles expose mobile-only decoders on this desktop without pretending
            // that a UIKit or Android window was created. The stand-in address is never dereferenced.
            int fakeWindow = 0;
            Platform::NativeWindowHandle mobile;
            mobile.window = &fakeWindow;
            Platform::UIKitNativeWindow uikit;
            mobile.system = Platform::NativeWindowSystem::UIKit;
            const bool uikitValid = Platform::TryGetUIKit(mobile, uikit) && uikit.window == &fakeWindow;
            Platform::AndroidNativeWindow android;
            mobile.system = Platform::NativeWindowSystem::Android;
            const bool androidValid = Platform::TryGetAndroid(mobile, android) && android.window == &fakeWindow;
            ctx.Check(uikitValid && androidValid && Platform::HasNativeWindow(mobile),
                      "the UIKit and Android decoders retain their typed native window pointer");

            int fakeDisplay = 0;
            Platform::NativeWindowHandle incompleteX11;
            incompleteX11.system = Platform::NativeWindowSystem::X11;
            incompleteX11.display = &fakeDisplay;
            Platform::X11NativeWindow unchanged{&fakeWindow, 77};
            ctx.Check(!Platform::TryGetX11(incompleteX11, unchanged) && unchanged.display == &fakeWindow &&
                          unchanged.window == 77 && !Platform::HasNativeWindow(incompleteX11) &&
                          Platform::Describe(incompleteX11) == "X11(display=set)",
                      "an incomplete X11 handle is refused atomically and described by populated fields");

            bool windowlessRejected = true;
            for (const Platform::NativeWindowSystem system :
                 {Platform::NativeWindowSystem::Unknown, Platform::NativeWindowSystem::Web,
                  Platform::NativeWindowSystem::Headless, Platform::NativeWindowSystem::Terminal})
            {
                Platform::NativeWindowHandle windowless;
                windowless.system = system;
                windowless.display = &fakeDisplay;
                windowless.window = &fakeWindow;
                windowless.surface = &fakeWindow;
                windowless.windowId = 1;
                windowlessRejected &= !Platform::HasNativeWindow(windowless);
            }
            ctx.Check(windowlessRejected,
                      "windowless systems never expose a native window even when irrelevant fields are populated");
        }
    };
}

CNA_DEMOS_GAME(NativeHandleDemo, {
    .id = "platform.window.native_handle",
    .title = "The native window handle",
    .description = "Matches Mouse.WindowHandle to XNA's GameWindow::Handle, round-trips that real token through "
                   "TouchPanel.WindowHandle, then reads CNA's "
                   "GetNativeWindowHandleEXT, whose NativeWindowSystem says "
                   "how to read it (X11 display and XID, Wayland display and surface, HWND, NSWindow), through the "
                   "typed TryGet accessors, native-presence and diagnostic helpers, including mobile, malformed and "
                   "windowless handle boundaries.",
    .apis = {"GameWindow::Handle", "Mouse::WindowHandle", "TouchPanel::WindowHandle",
             "GameWindow::GetNativeWindowHandleEXT",
             "CNA::Platform::NativeWindowHandle",
             "CNA::Platform::NativeWindowSystem", "CNA::Platform::TryGetX11", "CNA::Platform::TryGetWayland",
             "CNA::Platform::TryGetWin32", "CNA::Platform::TryGetCocoa", "CNA::Platform::TryGetUIKit",
             "CNA::Platform::TryGetAndroid", "CNA::Platform::HasNativeWindow", "CNA::Platform::Describe",
             "CNA::Platform::ToString"},
    .tags = {"window", "native", "interop", "handle", "x11", "wayland", "cnaext"},
    .needs = CnaDemos::Need::Window,
    .frames = 3,
})
