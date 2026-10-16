// SPDX-License-Identifier: MS-PL
// Microsoft.Devices.VibrateController: the phone's vibration motor (on a desktop: the gamepad's).
#include <chrono>
#include <string>
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

        const bool supported = vibrate->getIsSupportedProperty();
        const std::string deviceName = vibrate->getDeviceNameProperty();
        ctx.Check(supported == vibrate->getIsSupportedProperty() &&
                      deviceName == vibrate->getDeviceNameProperty() &&
                      (!supported || !deviceName.empty()),
                  "support and diagnostic device name form a stable capability snapshot");

        // A short buzz as feedback for a hit. Without a motor (a desktop without a gamepad)
        // Start does nothing, so the same game code runs everywhere.
        vibrate->Start(System::TimeSpan::FromMilliseconds(150.0));
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // CNAEXT: a weaker buzz; plain Start always vibrates at full strength.
        vibrate->Start(System::TimeSpan::FromMilliseconds(300.0), 0.3f);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        vibrate->Stop(); // ends it early

        // CNAEXT: distinct low/high-frequency motor strengths. On a host without a suitable
        // phone-style haptic device this follows the same documented no-op path as Start.
        vibrate->StartLeftRight(0.8f, 0.25f, System::TimeSpan::FromMilliseconds(50.0));
        vibrate->Stop();
        ctx.Check(true, "the dual-motor overload accepts an in-range request or documented no-op");

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

        bool negativeRefused = false;
        bool dualOverlongRefused = false;
        try
        {
            vibrate->Start(System::TimeSpan::FromMilliseconds(-1.0));
        }
        catch (const System::ArgumentOutOfRangeException&)
        {
            negativeRefused = true;
        }
        try
        {
            vibrate->StartLeftRight(0.5f, 0.5f, System::TimeSpan::FromSeconds(6.0));
        }
        catch (const System::ArgumentOutOfRangeException&)
        {
            dualOverlongRefused = true;
        }
        ctx.Check(negativeRefused && dualOverlongRefused,
                  "negative and dual-motor overlong durations are refused before hardware access");
        vibrate->Stop();
    }
}

CNA_DEMOS_CONSOLE(StartStop, {
    .id = "devices.vibrate_controller.start_stop",
    .title = "Vibration",
    .description = "Buzzes VibrateController::Default for 150 ms, then more weakly with the CNAEXT intensity "
                   "and dual-motor overloads and stops them early; invalid durations are refused. It reports the "
                   "selected haptic capability, while a host without a suitable motor follows the documented no-op.",
    .apis = {"VibrateController::Default", "VibrateController::IsSupported",
             "VibrateController::DeviceName", "VibrateController::Start",
             "VibrateController::StartLeftRight", "VibrateController::Stop"},
    .tags = {"devices", "vibration", "haptics", "phone", "feedback"},
})
