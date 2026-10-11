// SPDX-License-Identifier: MS-PL
// Microsoft.Devices.VibrateController: the phone's vibration motor (on a desktop: the gamepad's).
#include <chrono>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Devices/VibrateController.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/TimeSpan.hpp"

using Microsoft::Devices::VibrateController;

namespace
{
    void StartStop(CnaDemos::DemoContext& ctx)
    {
        // One controller per device, reached through Default.
        VibrateController* vibrate = VibrateController::getDefaultProperty();
        ctx.Check(vibrate != nullptr && vibrate == VibrateController::getDefaultProperty(),
                  "Default is the one shared controller");

        // A short buzz as feedback for a hit. Without a motor (a desktop without a gamepad)
        // Start does nothing, so the same game code runs everywhere.
        vibrate->Start(System::TimeSpan::FromMilliseconds(150.0));
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // CNAEXT: a weaker buzz; plain Start always vibrates at full strength.
        vibrate->Start(System::TimeSpan::FromMilliseconds(300.0), 0.3f);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        vibrate->Stop(); // ends it early

        // Windows Phone limits a single vibration to five seconds.
        bool refused = false;
        try
        {
            vibrate->Start(System::TimeSpan::FromSeconds(6.0));
        }
        catch (const System::ArgumentOutOfRangeException&)
        {
            refused = true;
        }
        ctx.Check(refused, "a duration over five seconds throws ArgumentOutOfRangeException");
        vibrate->Stop();
    }
}

CNA_DEMOS_CONSOLE(StartStop, {
    .id = "devices.vibrate_controller.start_stop",
    .title = "Vibration",
    .description = "Buzzes VibrateController::Default for 150 ms, then more weakly with the CNAEXT intensity "
                   "overload and stops it early; durations over five seconds are refused. Without a motor (a "
                   "desktop with no gamepad) Start does nothing.",
    .apis = {"VibrateController::Default", "VibrateController::Start", "VibrateController::Stop"},
    .tags = {"devices", "vibration", "haptics", "phone", "feedback"},
})
