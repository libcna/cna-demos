// SPDX-License-Identifier: MS-PL
// The gesture queue: ReadGesture returns gestures in order; CNAEXT EnqueueGesture injects one.
#include <array>
#include <string>

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
        constexpr std::array gestureTypes = {
            GestureType::Tap, GestureType::DoubleTap, GestureType::Hold,
            GestureType::HorizontalDrag, GestureType::VerticalDrag, GestureType::FreeDrag,
            GestureType::Pinch, GestureType::Flick, GestureType::DragComplete,
            GestureType::PinchComplete};
        constexpr GestureType allGestures =
            GestureType::Tap | GestureType::DoubleTap | GestureType::Hold |
            GestureType::HorizontalDrag | GestureType::VerticalDrag | GestureType::FreeDrag |
            GestureType::Pinch | GestureType::Flick | GestureType::DragComplete |
            GestureType::PinchComplete;

        // EnabledGestures is a flag set.
        TouchPanel::setEnabledGesturesProperty(allGestures);
        ctx.Check(TouchPanel::getEnabledGesturesProperty() == allGestures &&
                      (TouchPanel::getEnabledGesturesProperty() & GestureType::None) == GestureType::None,
                  "EnabledGestures combines all ten recognizer flags while None stays the empty set");

        // The usual loop empties the queue: start from a known state.
        while (TouchPanel::getIsGestureAvailableProperty())
            (void)TouchPanel::ReadGesture();

        // CNAEXT: gestures can be injected -- for tests, replays or tutorials. Game code reading
        // the queue cannot tell them from recognized ones. Distinct payloads make ordering and all
        // six public GestureSample fields observable for every gesture identity.
        for (std::size_t i = 0; i < gestureTypes.size(); ++i)
        {
            const float n = static_cast<float>(i + 1);
            TouchPanel::EnqueueGesture(GestureSample(
                gestureTypes[i], System::TimeSpan::FromSeconds(n),
                Vector2(10.0f * n + 1.0f, 10.0f * n + 2.0f),
                Vector2(10.0f * n + 3.0f, 10.0f * n + 4.0f),
                Vector2(n, -n), Vector2(-n, n)));
        }
        ctx.Check(TouchPanel::getIsGestureAvailableProperty(), "IsGestureAvailable is true while gestures wait");

        std::size_t read = 0;
        while (TouchPanel::getIsGestureAvailableProperty())
        {
            const GestureSample gesture = TouchPanel::ReadGesture();
            ctx.Print("{} at ({}, {}), delta ({}, {}), timestamp {} s", CnaDemos::Name(gesture.getGestureTypeProperty()),
                      gesture.getPositionProperty().X, gesture.getPositionProperty().Y, gesture.getDeltaProperty().X,
                      gesture.getDeltaProperty().Y, gesture.getTimestampProperty().getTotalSecondsProperty());
            const float n = static_cast<float>(read + 1);
            const bool expectedIndex = read < gestureTypes.size();
            ctx.Check(expectedIndex && gesture.getGestureTypeProperty() == gestureTypes[read] &&
                          gesture.getTimestampProperty() == System::TimeSpan::FromSeconds(n) &&
                          gesture.getPositionProperty() == Vector2(10.0f * n + 1.0f, 10.0f * n + 2.0f) &&
                          gesture.getPosition2Property() == Vector2(10.0f * n + 3.0f, 10.0f * n + 4.0f) &&
                          gesture.getDeltaProperty() == Vector2(n, -n) &&
                          gesture.getDelta2Property() == Vector2(-n, n),
                      std::string(expectedIndex ? CnaDemos::Name(gestureTypes[read]) : "unexpected gesture") +
                          " retains its queue position and payload");
            ++read;
        }
        ctx.Check(read == gestureTypes.size(), "all ten gestures were read, then the queue was empty");

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
    .description = "Combines every TouchPanel::EnabledGestures flag, injects one sample of every gesture type with "
                   "CNAEXT EnqueueGesture, reads their complete payloads back in order, and shows that reading an empty "
                   "queue throws.",
    .apis = {"TouchPanel::EnabledGestures", "TouchPanel::IsGestureAvailable", "TouchPanel::ReadGesture",
             "TouchPanel::EnqueueGesture", "GestureSample::GestureSample", "GestureSample::GestureType",
             "GestureSample::Timestamp", "GestureSample::Position", "GestureSample::Position2",
             "GestureSample::Delta", "GestureSample::Delta2", "GestureType", "GestureType::None",
             "GestureType::Tap", "GestureType::DoubleTap", "GestureType::Hold",
             "GestureType::HorizontalDrag", "GestureType::VerticalDrag", "GestureType::FreeDrag",
             "GestureType::Pinch", "GestureType::Flick", "GestureType::DragComplete",
             "GestureType::PinchComplete"},
    .tags = {"touch", "gesture", "queue", "testing", "replay", "cnaext"},
})
