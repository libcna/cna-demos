// SPDX-License-Identifier: MS-PL
// The live platform capability snapshot used by games to choose portable code paths.
#include "CNA/Platform/CurrentPlatform.hpp"
#include "CNA/Platform/PlatformCapabilities.hpp"
#include "CNA/Platform/PlatformCapability.hpp"
#include "CnaDemos/Demo.hpp"

#include <array>
#include <set>
#include <string>
#include <string_view>
#include <utility>

namespace Platform = CNA::Platform;

namespace
{
    using NamedCapability = std::pair<Platform::PlatformCapability, std::string_view>;

    constexpr std::array NamedCapabilities{
        NamedCapability{Platform::PlatformCapability::MultipleWindows, "MultipleWindows"},
        NamedCapability{Platform::PlatformCapability::HighDpi, "HighDpi"},
        NamedCapability{Platform::PlatformCapability::MultipleDisplays, "MultipleDisplays"},
        NamedCapability{Platform::PlatformCapability::BorderlessFullscreen, "BorderlessFullscreen"},
        NamedCapability{Platform::PlatformCapability::NativeWindowHandle, "NativeWindowHandle"},
        NamedCapability{Platform::PlatformCapability::SurfacePresentation, "SurfacePresentation"},
        NamedCapability{Platform::PlatformCapability::OpenGlContext, "OpenGlContext"},
        NamedCapability{Platform::PlatformCapability::Clipboard, "Clipboard"},
        NamedCapability{Platform::PlatformCapability::TextInput, "TextInput"},
        NamedCapability{Platform::PlatformCapability::Ime, "Ime"},
        NamedCapability{Platform::PlatformCapability::ExactKeyboardState, "ExactKeyboardState"},
        NamedCapability{Platform::PlatformCapability::PixelAccurateMouse, "PixelAccurateMouse"},
        NamedCapability{Platform::PlatformCapability::RelativeMouse, "RelativeMouse"},
        NamedCapability{Platform::PlatformCapability::CursorShapes, "CursorShapes"},
        NamedCapability{Platform::PlatformCapability::GlobalPointer, "GlobalPointer"},
        NamedCapability{Platform::PlatformCapability::InputDeviceEnumeration, "InputDeviceEnumeration"},
        NamedCapability{Platform::PlatformCapability::Gamepad, "Gamepad"},
        NamedCapability{Platform::PlatformCapability::Joystick, "Joystick"},
        NamedCapability{Platform::PlatformCapability::GamepadRumble, "GamepadRumble"},
        NamedCapability{Platform::PlatformCapability::GamepadSensors, "GamepadSensors"},
        NamedCapability{Platform::PlatformCapability::Haptics, "Haptics"},
        NamedCapability{Platform::PlatformCapability::Sensors, "Sensors"},
        NamedCapability{Platform::PlatformCapability::PowerInfo, "PowerInfo"},
        NamedCapability{Platform::PlatformCapability::MessageBox, "MessageBox"},
        NamedCapability{Platform::PlatformCapability::NativeFileDialog, "NativeFileDialog"},
        NamedCapability{Platform::PlatformCapability::Tray, "Tray"},
        NamedCapability{Platform::PlatformCapability::Camera, "Camera"},
        NamedCapability{Platform::PlatformCapability::ManagedEntrypoint, "ManagedEntrypoint"},
        NamedCapability{Platform::PlatformCapability::DragAndDrop, "DragAndDrop"},
        NamedCapability{Platform::PlatformCapability::PrimarySelection, "PrimarySelection"},
        NamedCapability{Platform::PlatformCapability::ClipboardData, "ClipboardData"},
    };

    class CapabilityMatrixDemo final : public CnaDemos::DemoGame
    {
    protected:
        void LoadContent() override
        {
            auto& ctx = Context();
            Platform::IPlatform& platform = GetPlatformEXT();
            const Platform::PlatformCapabilities& capabilities = GetPlatformCapabilitiesEXT();
            const Platform::PlatformCapabilities& sameCapabilities = GetPlatformCapabilitiesEXT();

            ctx.Check(Platform::HasCurrentPlatform() && &Platform::GetCurrentPlatform() == &platform &&
                          !platform.GetName().empty(),
                      "the game-owned platform is the installed ambient platform and has a stable name");
            ctx.Check(&capabilities == &sameCapabilities,
                      "Game caches one capability snapshot and returns the same object on later reads");

            const auto& all = Platform::AllCapabilities();
            std::set<Platform::PlatformCapability> unique;
            bool orderedAndNamed = all.size() == NamedCapabilities.size();
            for (std::size_t i = 0; i < all.size() && i < NamedCapabilities.size(); ++i)
            {
                orderedAndNamed &= all[i] == NamedCapabilities[i].first &&
                                   Platform::ToString(all[i]) == NamedCapabilities[i].second;
                unique.insert(all[i]);
            }
            ctx.Check(orderedAndNamed && unique.size() == NamedCapabilities.size(),
                      "AllCapabilities enumerates every unique capability in declaration order with a stable name");

            bool liveQueriesMatch = true;
            std::size_t supported = 0;
            for (const Platform::PlatformCapability capability : all)
            {
                const bool cached = Platform::Supports(capabilities, capability);
                liveQueriesMatch &= platform.Supports(capability) == cached;
                supported += cached ? 1u : 0u;
            }
            ctx.Check(liveQueriesMatch,
                      "the platform's focused capability queries agree with the cached complete snapshot");

            const Platform::PlatformCapabilities safeDefaults;
            bool everyDefaultIsUnsupported = true;
            for (const Platform::PlatformCapability capability : all)
                everyDefaultIsUnsupported &= !Platform::Supports(safeDefaults, capability);
            ctx.Check(everyDefaultIsUnsupported,
                      "a default capability aggregate safely reports every optional feature unsupported");

            ctx.Print("Platform {} supports {} of {} declared capabilities", platform.GetName(), supported, all.size());
        }
    };
}

CNA_DEMOS_GAME(CapabilityMatrixDemo, {
    .id = "core.platform.capability_matrix",
    .title = "Platform capability matrix",
    .description = "Reads the Game-owned platform's cached capability snapshot, enumerates all named capabilities, "
                   "and compares every cached answer with the platform's focused Supports query.",
    .apis = {"Game::GetPlatformEXT", "Game::GetPlatformCapabilitiesEXT", "CNA::Platform::IPlatform::GetName",
             "CNA::Platform::IPlatform::Supports", "CNA::Platform::PlatformCapability",
             "CNA::Platform::PlatformCapabilities", "CNA::Platform::AllCapabilities",
             "CNA::Platform::Supports", "CNA::Platform::ToString", "CNA::Platform::HasCurrentPlatform",
             "CNA::Platform::GetCurrentPlatform"},
    .tags = {"platform", "capabilities", "portability", "cnaext"},
    .frames = 3,
})
