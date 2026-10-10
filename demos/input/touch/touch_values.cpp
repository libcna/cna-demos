// SPDX-License-Identifier: MS-PL
// TouchLocation and TouchCollection as values: previous locations and finding a finger by Id.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchCollection.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocationState.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Input::Touch;

namespace
{
    void TouchValues(CnaDemos::DemoContext& ctx)
    {
        // A finger that just touched down has no previous location...
        const TouchLocation down(7, TouchLocationState::Pressed, Vector2(100.0f, 50.0f));
        TouchLocation previous;
        ctx.Check(!down.TryGetPreviousLocation(previous), "a new touch has no previous location");

        // ...one that is held carries where it was in the previous frame.
        const TouchLocation held(7, TouchLocationState::Moved, Vector2(130.0f, 90.0f), TouchLocationState::Pressed,
                                 Vector2(100.0f, 50.0f));
        ctx.Check(held.TryGetPreviousLocation(previous) && previous.getStateProperty() == TouchLocationState::Pressed &&
                      previous.getPositionProperty() == Vector2(100.0f, 50.0f),
                  "a held touch knows its previous state and position");
        const Vector2 motion = held.getPositionProperty() - previous.getPositionProperty();
        ctx.Print("finger {} moved by ({}, {}) = {} pixels", held.getIdProperty(), motion.X, motion.Y, motion.Length());
        ctx.CheckNear(motion.Length(), 50.0, 1e-4, "it moved 50 pixels since the previous frame");

        // A frame's touches: the Id, not the index, identifies a finger from frame to frame.
        const TouchCollection frame(std::vector<TouchLocation>{
            held, TouchLocation(9, TouchLocationState::Pressed, Vector2(600.0f, 300.0f))});
        ctx.Check(frame.getCountProperty() == 2, "the collection holds two touches");
        ctx.Check(frame.getIsReadOnlyProperty(), "a TouchCollection reports itself read-only");
        TouchLocation found;
        ctx.Check(frame.FindById(9, found) && found.getPositionProperty() == Vector2(600.0f, 300.0f),
                  "FindById finds the second finger");
        ctx.Check(!frame.FindById(3, found), "FindById reports a finger that is not there");
        ctx.Check(frame.Contains(held) && frame.IndexOf(held) == 0 && frame[1].getIdProperty() == 9,
                  "Contains, IndexOf and the indexer work like any list");
    }
}

CNA_DEMOS_CONSOLE(TouchValues, {
    .id = "input.touch.touch_values",
    .title = "TouchLocation and TouchCollection values",
    .description = "Builds TouchLocation values with and without a previous location (TryGetPreviousLocation) and a "
                   "TouchCollection of two fingers, then finds them with FindById, Contains, IndexOf and the indexer.",
    .apis = {"TouchLocation::TouchLocation", "TouchLocation::TryGetPreviousLocation", "TouchLocation::Id",
             "TouchLocation::State", "TouchLocation::Position", "TouchCollection::TouchCollection",
             "TouchCollection::Count", "TouchCollection::IsReadOnly", "TouchCollection::FindById",
             "TouchCollection::Contains", "TouchCollection::IndexOf", "TouchCollection::Item"},
    .tags = {"touch", "multitouch", "finger", "testing", "value"},
})
