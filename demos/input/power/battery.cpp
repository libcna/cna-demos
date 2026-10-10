// SPDX-License-Identifier: MS-PL
// CNAEXT Power::GetInfoEXT: on battery or mains, charge left -- e.g. to warn before saving fails.
#include "CNA/Input/Power.hpp"
#include "CNA/Input/PowerState.hpp"
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"

using CNA::Input::Power;
using CNA::Input::PowerStateEXT;

namespace
{
    void Battery(CnaDemos::DemoContext& ctx)
    {
        // One call answers all three questions at the same instant. -1 means "unknown".
        int secondsLeft = 0;
        int percent = 0;
        const PowerStateEXT state = Power::GetInfoEXT(secondsLeft, percent);
        ctx.Print("state {}, {} % charge, {} seconds left", CnaDemos::Name(state), percent, secondsLeft);
        if (state == PowerStateEXT::OnBattery && percent >= 0 && percent < 10)
            ctx.Print("battery low: a game would suggest saving now");

        ctx.Check(percent >= -1 && percent <= 100, "the charge is a percentage, or -1 when unknown");
        ctx.Check(secondsLeft >= -1, "the time left is in seconds, or -1 when unknown");
        if (state == PowerStateEXT::NoBattery)
            ctx.Check(percent == -1 && secondsLeft == -1, "without a battery there is no charge or time to report");
    }
}

CNA_DEMOS_CONSOLE(Battery, {
    .id = "input.power.battery",
    .title = "Battery state (CNAEXT)",
    .description = "Asks Power::GetInfoEXT whether the machine runs on battery or mains, how much charge is left "
                   "and for how long, and checks the ranges of the answers.",
    .apis = {"CNA::Input::Power::GetInfoEXT", "CNA::Input::PowerStateEXT"},
    .tags = {"power", "battery", "laptop", "phone", "cnaext"},
})
