// SPDX-License-Identifier: MS-PL
// KeyboardState as a value: build one from keys, query it, compare it -- and replay a recording.
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Input/KeyState.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"

using namespace Microsoft::Xna::Framework::Input;

namespace
{
    // Game logic that reads a pair of snapshots instead of the keyboard can run on recorded input.
    bool PressedThisFrame(const KeyboardState& previous, const KeyboardState& current, Keys key)
    {
        return current.IsKeyDown(key) && previous.IsKeyUp(key);
    }

    void StateValues(CnaDemos::DemoContext& ctx)
    {
        // A KeyboardState is just the set of keys that were down. Keyboard::GetState returns one,
        // and the constructor builds one -- for tests, input replay or a virtual keyboard.
        const KeyboardState noKeys;
        const KeyboardState state{Keys::LeftShift, Keys::A};

        ctx.Check(state.IsKeyDown(Keys::A) && state.IsKeyDown(Keys::LeftShift), "IsKeyDown is true for the given keys");
        ctx.Check(state.IsKeyUp(Keys::B), "IsKeyUp is true for every other key");
        ctx.Check(state[Keys::A] == KeyState::Down && state[Keys::B] == KeyState::Up,
                  "the indexer (C#'s state[key]) answers with KeyState::Down or KeyState::Up");

        const std::vector<Keys> pressed = state.GetPressedKeys();
        for (const Keys key : pressed)
            ctx.Print("GetPressedKeys: {} ({})", CnaDemos::Name(key), static_cast<int>(key));
        ctx.Check(pressed.size() == 2, "GetPressedKeys lists the two keys");
        ctx.Check(noKeys.GetPressedKeys().empty(), "a default KeyboardState has no keys down");

        const KeyboardState reordered{Keys::A, Keys::LeftShift};
        ctx.Check(reordered == state, "states with the same keys are equal, in any order");
        ctx.Check(reordered.GetHashCode() == state.GetHashCode() && noKeys.GetHashCode() == 0,
                  "equal key sets have equal hashes independent of insertion order, and the empty set hashes to zero");
        ctx.Check(state != noKeys && !state.Equals(noKeys), "different keys make different states");

        // Replay: Space held during frames 1-3, released, then held again at frame 5.
        const std::vector<KeyboardState> recording = {noKeys, {Keys::Space}, {Keys::Space}, {Keys::Space, Keys::Up},
                                                      noKeys, {Keys::Space}};
        int jumps = 0;
        for (std::size_t frame = 1; frame < recording.size(); ++frame)
        {
            if (PressedThisFrame(recording[frame - 1], recording[frame], Keys::Space))
            {
                ++jumps;
                ctx.Print("frame {}: Space pressed", frame);
            }
        }
        ctx.Check(jumps == 2, "the recording holds two Space presses although Space is down in four frames");
    }
}

CNA_DEMOS_CONSOLE(StateValues, {
    .id = "input.keyboard.state_values",
    .title = "KeyboardState values",
    .description = "Builds KeyboardState values from keys, queries them with IsKeyDown, IsKeyUp, the indexer and "
                   "GetPressedKeys, compares them, and counts key presses in a recorded sequence of states.",
    .apis = {"KeyboardState::KeyboardState", "KeyboardState::IsKeyDown", "KeyboardState::IsKeyUp",
             "KeyboardState::Item", "KeyboardState::GetPressedKeys", "KeyboardState::Equals",
             "KeyboardState::GetHashCode", "KeyboardState::op_Equality", "KeyboardState::op_Inequality",
             "KeyState", "Keys"},
    .tags = {"keys", "replay", "testing", "value"},
})
