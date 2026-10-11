// SPDX-License-Identifier: MS-PL
// CNA::Devices SystemInfo, Locale and PowerInfo: what a game may ask about the machine.
#include <string>
#include <thread>
#include <vector>

#include "CNA/Devices/Locale.hpp"
#include "CNA/Devices/LocaleInfo.hpp"
#include "CNA/Devices/PowerInfo.hpp"
#include "CNA/Devices/PowerState.hpp"
#include "CNA/Devices/SystemInfo.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    const char* PowerName(CNA::Devices::PowerState state)
    {
        switch (state)
        {
            case CNA::Devices::PowerState::Error: return "Error";
            case CNA::Devices::PowerState::Unknown: return "Unknown";
            case CNA::Devices::PowerState::OnBattery: return "OnBattery";
            case CNA::Devices::PowerState::NoBattery: return "NoBattery";
            case CNA::Devices::PowerState::Charging: return "Charging";
            case CNA::Devices::PowerState::Charged: return "Charged";
        }
        return "?";
    }

    void MachineInfo(CnaDemos::DemoContext& ctx)
    {
        // Sizing worker threads or picking default quality settings.
        const int cores = CNA::Devices::SystemInfo::getLogicalCpuCoreCountProperty();
        const int ram = CNA::Devices::SystemInfo::getSystemRamMegabytesProperty();
        ctx.Print("logical CPU cores: {}, RAM: {} MB", cores, ram);
        ctx.Check(cores >= 1 && (std::thread::hardware_concurrency() == 0 ||
                                 cores == static_cast<int>(std::thread::hardware_concurrency())),
                  "the core count matches what the C++ library reports");
        ctx.Check(ram > 0, "the amount of RAM is known");

        // Choosing the language of a game's text: the user's languages, most preferred first.
        const std::vector<CNA::Devices::LocaleInfo> locales = CNA::Devices::Locale::getPreferredLocalesProperty();
        for (const auto& locale : locales)
        {
            ctx.Print("preferred locale: language \"{}\", country \"{}\"", locale.Language, locale.Country);
        }
        ctx.Print("{} preferred locale(s)", locales.size());

        // Lowering the frame rate on battery, or warning before a laptop runs out.
        const CNA::Devices::PowerState power = CNA::Devices::PowerInfo::getStateProperty();
        ctx.Print("power: {}, battery {}%, {} s remaining", PowerName(power),
                  CNA::Devices::PowerInfo::getBatteryPercentProperty(),
                  CNA::Devices::PowerInfo::getSecondsRemainingProperty());
        ctx.Check(power != CNA::Devices::PowerState::Error, "the power state could be read");
    }
}

CNA_DEMOS_CONSOLE(MachineInfo, {
    .id = "devices.system.machine_info",
    .title = "Machine information",
    .description = "Reads CNA::Devices SystemInfo (logical CPU cores, RAM), the user's preferred locales with Locale, "
                   "and the power state, battery percentage and time remaining with PowerInfo -- the inputs for "
                   "worker counts, the text language and battery-saving decisions.",
    .apis = {"CNA::Devices::SystemInfo::LogicalCpuCoreCount", "CNA::Devices::SystemInfo::SystemRamMegabytes",
             "CNA::Devices::Locale::PreferredLocales", "CNA::Devices::LocaleInfo", "CNA::Devices::PowerInfo::State",
             "CNA::Devices::PowerInfo::BatteryPercent", "CNA::Devices::PowerInfo::SecondsRemaining",
             "CNA::Devices::PowerState"},
    .tags = {"devices", "system", "hardware", "locale", "battery", "cnaext"},
})
