// SPDX-License-Identifier: MS-PL
// The gesture queue: ReadGesture returns gestures in order; CNAEXT EnqueueGesture injects one.
#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/GestureSample.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/GestureType.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Input::Touch;

namespace
{
    void GestureQueue(CnaDemos::DemoContext& ctx)
    {
        // EnabledGestures is a flag set.
        TouchPanel::setEnabledGesturesProperty(GestureType::Tap | GestureType::FreeDrag);
        ctx.Check((TouchPanel::getEnabledGesturesProperty() & GestureType::FreeDrag) == GestureType::FreeDrag &&
                      (TouchPanel::getEnabledGesturesProperty() & GestureType::Pinch) == GestureType::None,
                  "EnabledGestures holds exactly the enabled flags");

        // The usual loop empties the queue: start from a known state.
        while (TouchPanel::getIsGestureAvailableProperty())
            (void)TouchPanel::ReadGesture();

        // CNAEXT: gestures can be injected -- for tests, replays or a tutorial that demonstrates
        // a swipe. Game code reading the queue cannot tell them from recognized ones.
        TouchPanel::EnqueueGesture(GestureSample(GestureType::Tap, System::TimeSpan::FromSeconds(1.0),
                                                 Vector2(200.0f, 100.0f), Vector2::Zero, Vector2::Zero, Vector2::Zero));
        TouchPanel::EnqueueGesture(GestureSample(GestureType::FreeDrag, System::TimeSpan::FromSeconds(1.5),
                                                 Vector2(220.0f, 100.0f), Vector2::Zero, Vector2(20.0f, 0.0f),
                                                 Vector2::Zero));
        ctx.Check(TouchPanel::getIsGestureAvailableProperty(), "IsGestureAvailable is true while gestures wait");

        int read = 0;
        while (TouchPanel::getIsGestureAvailableProperty())
        {
            const GestureSample gesture = TouchPanel::ReadGesture();
            ctx.Print("{} at ({}, {}), delta ({}, {}), timestamp {} s", CnaDemos::Name(gesture.getGestureTypeProperty()),
                      gesture.getPositionProperty().X, gesture.getPositionProperty().Y, gesture.getDeltaProperty().X,
                      gesture.getDeltaProperty().Y, gesture.getTimestampProperty().getTotalSecondsProperty());
            ctx.Check(gesture.getGestureTypeProperty() == (read == 0 ? GestureType::Tap : GestureType::FreeDrag),
                      read == 0 ? "the first gesture read is the Tap" : "the second is the FreeDrag");
            ++read;
        }
        ctx.Check(read == 2, "two gestures were read, then the queue was empty");

        // Reading from an empty queue is an error: check IsGestureAvailable first.
        bool threw = false;
        try
        {
            (void)TouchPanel::ReadGesture();
        }
        catch (const System::InvalidOperationException&)
        {
            threw = true;
        }
        ctx.Check(threw, "ReadGesture with no gesture available throws InvalidOperationException");
        TouchPanel::setEnabledGesturesProperty(GestureType::None);
    }
}

CNA_DEMOS_CONSOLE(GestureQueue, {
    .id = "input.touch.gesture_queue",
    .title = "Gesture queue",
    .description = "Sets TouchPanel::EnabledGestures, injects a tap and a drag with the CNAEXT EnqueueGesture, reads "
                   "them back in order with IsGestureAvailable and ReadGesture, and shows that reading an empty queue "
                   "throws.",
    .apis = {"TouchPanel::EnabledGestures", "TouchPanel::IsGestureAvailable", "TouchPanel::ReadGesture",
             "TouchPanel::EnqueueGesture", "GestureSample::GestureSample", "GestureSample::GestureType",
             "GestureSample::Timestamp", "GestureSample::Position", "GestureSample::Delta", "GestureType"},
    .tags = {"touch", "gesture", "queue", "testing", "replay", "cnaext"},
})
