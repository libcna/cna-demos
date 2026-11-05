// SPDX-License-Identifier: MS-PL
// TouchLocation and TouchCollection as values: previous locations and finding a finger by Id.
#include <any>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchCollection.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocation.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocationState.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchPanel.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "System/ArgumentOutOfRangeException.hpp"
#include "System/NotSupportedException.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Input::Touch;

namespace
{
    void TouchValues(CnaDemos::DemoContext& ctx)
    {
        // A finger that just touched down has no previous location...
        const TouchLocation down(7, TouchLocationState::Pressed, Vector2(100.0f, 50.0f));
        TouchLocation previous;
        ctx.Check(!down.TryGetPreviousLocation(previous) && previous.getIdProperty() == TouchPanel::NO_FINGER &&
                      previous.getStateProperty() == TouchLocationState::Invalid &&
                      previous.getPositionProperty() == Vector2::Zero,
                  "a new touch reports no previous location but still writes its invalid sentinel");

        // ...one that is held carries where it was in the previous frame.
        const TouchLocation held(7, TouchLocationState::Moved, Vector2(130.0f, 90.0f), TouchLocationState::Pressed,
                                 Vector2(100.0f, 50.0f));
        ctx.Check(held.TryGetPreviousLocation(previous) && previous.getStateProperty() == TouchLocationState::Pressed &&
                      previous.getPositionProperty() == Vector2(100.0f, 50.0f),
                  "a held touch knows its previous state and position");
        const Vector2 motion = held.getPositionProperty() - previous.getPositionProperty();
        ctx.Print("finger {} moved by ({}, {}) = {} pixels", held.getIdProperty(), motion.X, motion.Y, motion.Length());
        ctx.CheckNear(motion.Length(), 50.0, 1e-4, "it moved 50 pixels since the previous frame");

        const TouchLocation pressured(7, TouchLocationState::Moved, Vector2(130.0f, 90.0f),
                                      TouchLocationState::Pressed, Vector2(100.0f, 50.0f), 0.75f);
        ctx.Check(pressured.getPressureEXT() == 0.75f && held.Equals(pressured) && held == pressured &&
                      !(held != pressured),
                  "CNA pressure is preserved but excluded from the XNA TouchLocation equality contract");
        ctx.Check(held.Equals(std::any(pressured)) && !held.Equals(std::any(down)) &&
                      !held.Equals(std::any(17)) && !held.Equals(std::any{}),
                  "boxed equality accepts the same XNA location and rejects different, wrong and empty boxes");
        ctx.Check(held.GetHashCode() == pressured.GetHashCode(),
                  "TouchLocation values equal under the XNA contract have equal hashes");
        const TouchLocation differentPrevious(7, TouchLocationState::Moved, Vector2(130.0f, 90.0f),
                                              TouchLocationState::Pressed, Vector2(99.0f, 50.0f));
        ctx.Check(differentPrevious != held && !differentPrevious.Equals(held) &&
                      held.ToString() == "{Position:{X:130 Y:90}}",
                  "equality includes the previous location while text reports the current position only");

        // A frame's touches: the Id, not the index, identifies a finger from frame to frame.
        const TouchCollection frame(std::vector<TouchLocation>{
            held, TouchLocation(9, TouchLocationState::Pressed, Vector2(600.0f, 300.0f))});
        ctx.Check(frame.getCountProperty() == 2 && frame.getIsConnectedProperty(),
                  "the array-constructed collection holds two touches and reports connected");
        ctx.Check(frame.getIsReadOnlyProperty(), "a TouchCollection reports itself read-only");
        TouchLocation found;
        ctx.Check(frame.FindById(9, found) && found.getPositionProperty() == Vector2(600.0f, 300.0f),
                  "FindById finds the second finger");
        ctx.Check(!frame.FindById(3, found) && found.getIdProperty() == 0 &&
                      found.getStateProperty() == TouchLocationState::Invalid &&
                      found.getPositionProperty() == Vector2::Zero,
                  "FindById reports a missing finger and writes the default invalid location");
        ctx.Check(frame.Contains(held) && frame.IndexOf(held) == 0 && frame[1].getIdProperty() == 9,
                  "Contains, IndexOf and the indexer work like any list");

        auto cursor = frame.GetEnumerator();
        bool currentBeforeStartThrows = false;
        try
        {
            static_cast<void>(cursor.Current());
        }
        catch (const System::ArgumentOutOfRangeException&)
        {
            currentBeforeStartThrows = true;
        }
        ctx.Check(currentBeforeStartThrows && cursor.MoveNext() && cursor.Current() == held,
                  "the explicit enumerator starts before the first touch and advances to it");

        auto copiedCursor = cursor;
        ctx.Check(cursor.MoveNext() && cursor.Current().getIdProperty() == 9 && !cursor.MoveNext() &&
                      copiedCursor.Current() == held && copiedCursor.MoveNext() &&
                      copiedCursor.Current().getIdProperty() == 9,
                  "copied enumerators keep independent positions over the same value snapshot");

        cursor.Reset();
        System::Collections::IEnumerator& erasedCursor = cursor;
        const bool resetToBeforeStart = cursor.MoveNext() &&
            std::any_cast<TouchLocation>(erasedCursor.getCurrentProperty()) == held;
        cursor.Dispose();
        ctx.Check(resetToBeforeStart && cursor.Current() == held,
                  "Reset restarts the cursor, its non-generic Current boxes the value and Dispose is harmless");

        TouchCollection snapshotSource = frame;
        auto snapshotCursor = snapshotSource.GetEnumerator();
        snapshotSource = TouchCollection{};
        ctx.Check(snapshotCursor.MoveNext() && snapshotCursor.Current() == held && snapshotCursor.MoveNext() &&
                      snapshotCursor.Current().getIdProperty() == 9 && !snapshotCursor.MoveNext(),
                  "an enumerator owns a snapshot that survives replacement of the source value");

        std::vector<TouchLocation> copied{down};
        frame.CopyTo(copied, 1);
        ctx.Check(copied.size() == 3 && copied[0] == down && copied[1] == held &&
                      copied[2].getIdProperty() == 9,
                  "CopyTo inserts the complete touch snapshot at the requested vector index");

        TouchCollection edited = frame;
        const TouchLocation inserted(5, TouchLocationState::Pressed, Vector2(20.0f, 30.0f));
        const TouchLocation appended(11, TouchLocationState::Released, Vector2(700.0f, 400.0f));
        const auto refusesEdit = [&ctx, &edited, &frame](auto edit, const char* description) {
            bool refused = false;
            try { edit(); }
            catch (const System::NotSupportedException&) { refused = true; }
            ctx.Check(refused && edited.getIsReadOnlyProperty() && edited.getCountProperty() == 2 &&
                          edited[0] == frame[0] && edited[1] == frame[1], description);
        };
        refusesEdit([&] { edited.Add(appended); }, "Add throws without changing the read-only snapshot");
        refusesEdit([&] { edited.Insert(1, inserted); }, "Insert throws without changing the snapshot");
        refusesEdit([&] { (void)edited.Remove(held); }, "Remove throws even for a present touch");
        refusesEdit([&] { (void)edited.Remove(down); }, "Remove throws even for an absent touch");
        refusesEdit([&] { edited.RemoveAt(1); }, "RemoveAt throws without changing the snapshot");
        refusesEdit([&] { edited.setItemProperty(0, inserted); }, "the item setter throws without changing the snapshot");
        refusesEdit([&] { edited.Clear(); }, "Clear throws without changing the snapshot");
    }
}

CNA_DEMOS_CONSOLE(TouchValues, {
    .id = "input.touch.touch_values",
    .title = "TouchLocation and TouchCollection values",
    .description = "Builds TouchLocation values with previous locations and pressure, checks their XNA equality, hash "
                   "and text contracts, then enumerates, searches and copies a read-only TouchCollection and checks "
                   "that its mutators throw without changing the snapshot.",
    .apis = {"TouchLocation::TouchLocation", "TouchLocation::TryGetPreviousLocation", "TouchLocation::Id",
             "TouchLocation::State", "TouchLocation::Position", "TouchLocation::PressureEXT", "TouchLocation::Equals",
             "TouchLocation::GetHashCode", "TouchLocation::op_Equality", "TouchLocation::op_Inequality",
             "TouchLocation::ToString", "TouchCollection::TouchCollection",
             "TouchCollection::Count", "TouchCollection::IsConnected", "TouchCollection::IsReadOnly", "TouchCollection::FindById",
             "TouchCollection::Contains", "TouchCollection::CopyTo", "TouchCollection::IndexOf",
             "TouchCollection::Item", "TouchCollection::Add", "TouchCollection::Insert",
             "TouchCollection::Remove", "TouchCollection::RemoveAt", "TouchCollection::Clear",
             "TouchCollection.Enumerator::Current", "TouchCollection.Enumerator::MoveNext",
             "TouchCollection.Enumerator::System#Collections#IEnumerator#Current",
             "TouchCollection.Enumerator::System#Collections#IEnumerator#Reset",
             "TouchCollection.Enumerator::Dispose",
             "TouchPanel::NO_FINGER",
             "TouchLocationState::Invalid", "TouchLocationState::Released",
             "TouchLocationState::Pressed", "TouchLocationState::Moved"},
    .tags = {"touch", "multitouch", "finger", "testing", "value", "collection"},
})
