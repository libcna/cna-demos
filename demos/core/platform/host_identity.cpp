// SPDX-License-Identifier: MS-PL
// Compile-target and desktop-OS identity through CNA's native C++ core API.
#include "CNA/CNAException.hpp"
#include "CNA/DesktopOS.hpp"
#include "CNA/TargetPlatform.hpp"
#include "CnaDemos/Demo.hpp"

#include <string_view>

namespace
{
    constexpr std::string_view DesktopOsName(const CNA::DesktopOS os)
    {
        switch (os)
        {
            case CNA::DesktopOS::Windows: return "Windows";
            case CNA::DesktopOS::Linux: return "Linux";
            case CNA::DesktopOS::MacOSX: return "macOS";
            case CNA::DesktopOS::Other: return "Desktop";
        }
        return "Unknown";
    }

    void HostIdentity(CnaDemos::DemoContext& ctx)
    {
        constexpr CNA::TargetPlatform target = CNA::getCurrentPlatform();
        constexpr std::string_view platformName = CNA::getCurrentPlatformName();
        constexpr bool mobile = CNA::isMobilePlatform();
        constexpr bool apple = CNA::isApplePlatform();

        ctx.Check(mobile == (target == CNA::TargetPlatform::Android || target == CNA::TargetPlatform::iOS),
                  "the mobile flag agrees with the compile-time target identity");
        ctx.Check(apple == (target == CNA::TargetPlatform::iOS || platformName == "macOS"),
                  "the Apple flag agrees with iOS and macOS target names");

        bool targetNameMatches = false;
        switch (target)
        {
            case CNA::TargetPlatform::Desktop:
                targetNameMatches = platformName == "Windows" || platformName == "Linux" ||
                                    platformName == "macOS" || platformName == "Desktop";
                break;
            case CNA::TargetPlatform::Android: targetNameMatches = platformName == "Android"; break;
            case CNA::TargetPlatform::iOS: targetNameMatches = platformName == "iOS"; break;
            case CNA::TargetPlatform::Web: targetNameMatches = platformName == "Web"; break;
        }
        ctx.Check(targetNameMatches, "the target has its documented stable diagnostic name");

        if (target == CNA::TargetPlatform::Desktop)
        {
            const CNA::DesktopOS desktop = CNA::getCurrentDesktopOS();
            ctx.Check(DesktopOsName(desktop) == platformName,
                      "the desktop OS identity agrees with the more specific platform name");
            ctx.Print("Target platform: {}; desktop OS: {}", platformName, DesktopOsName(desktop));
        }
        else
        {
            bool refused = false;
            try
            {
                (void) CNA::getCurrentDesktopOS();
            }
            catch (const CNA::CNAException&)
            {
                refused = true;
            }
            ctx.Check(refused, "a non-desktop target refuses the desktop-only OS query");
            ctx.Print("Target platform: {}; mobile: {}; Apple: {}", platformName,
                      mobile ? "yes" : "no", apple ? "yes" : "no");
        }
    }
}

CNA_DEMOS_CONSOLE(HostIdentity, {
    .id = "core.platform.host_identity",
    .title = "Host platform identity",
    .description = "Reports CNA's compile-time target platform, mobile and Apple classifications, and the "
                   "desktop operating-system identity when that query applies.",
    .apis = {"CNA::TargetPlatform", "CNA::getCurrentPlatform", "CNA::getCurrentPlatformName",
             "CNA::isMobilePlatform", "CNA::isApplePlatform", "CNA::DesktopOS",
             "CNA::getCurrentDesktopOS", "CNA::CNAException"},
    .tags = {"platform", "host", "metadata"},
})
