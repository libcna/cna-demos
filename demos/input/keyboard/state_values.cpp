// SPDX-License-Identifier: MS-PL
// KeyboardState as a value: build one from keys, query it, compare it -- and replay a recording.
#include <algorithm>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Input/KeyState.hpp"
#include "Microsoft/Xna/Framework/Input/KeyboardState.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"

using namespace Microsoft::Xna::Framework::Input;

#define CNA_DEMOS_KEYS_CONTROL(APPLY) \
    APPLY(None) APPLY(Back) APPLY(Tab) APPLY(Enter) APPLY(Pause) APPLY(CapsLock) APPLY(Kana) APPLY(Kanji) APPLY(Escape) \
    APPLY(ImeConvert) APPLY(ImeNoConvert) APPLY(Space) APPLY(PageUp) APPLY(PageDown) APPLY(End) APPLY(Home) APPLY(Left) APPLY(Up) \
    APPLY(Right) APPLY(Down) APPLY(Select) APPLY(Print) APPLY(Execute) APPLY(PrintScreen) APPLY(Insert) APPLY(Delete) APPLY(Help)
#define CNA_DEMOS_KEYS_ALPHANUMERIC(APPLY) \
    APPLY(D0) APPLY(D1) APPLY(D2) APPLY(D3) APPLY(D4) APPLY(D5) APPLY(D6) APPLY(D7) APPLY(D8) APPLY(D9) \
    APPLY(A) APPLY(B) APPLY(C) APPLY(D) APPLY(E) APPLY(F) APPLY(G) APPLY(H) APPLY(I) APPLY(J) APPLY(K) APPLY(L) APPLY(M) \
    APPLY(N) APPLY(O) APPLY(P) APPLY(Q) APPLY(R) APPLY(S) APPLY(T) APPLY(U) APPLY(V) APPLY(W) APPLY(X) APPLY(Y) APPLY(Z) \
    APPLY(LeftWindows) APPLY(RightWindows) APPLY(Apps) APPLY(Sleep)
#define CNA_DEMOS_KEYS_NUMPAD(APPLY) \
    APPLY(NumPad0) APPLY(NumPad1) APPLY(NumPad2) APPLY(NumPad3) APPLY(NumPad4) APPLY(NumPad5) APPLY(NumPad6) APPLY(NumPad7) \
    APPLY(NumPad8) APPLY(NumPad9) APPLY(Multiply) APPLY(Add) APPLY(Separator) APPLY(Subtract) APPLY(Decimal) APPLY(Divide)
#define CNA_DEMOS_KEYS_FUNCTION_MODIFIER(APPLY) \
    APPLY(F1) APPLY(F2) APPLY(F3) APPLY(F4) APPLY(F5) APPLY(F6) APPLY(F7) APPLY(F8) APPLY(F9) APPLY(F10) APPLY(F11) APPLY(F12) \
    APPLY(F13) APPLY(F14) APPLY(F15) APPLY(F16) APPLY(F17) APPLY(F18) APPLY(F19) APPLY(F20) APPLY(F21) APPLY(F22) APPLY(F23) APPLY(F24) \
    APPLY(NumLock) APPLY(Scroll) APPLY(LeftShift) APPLY(RightShift) APPLY(LeftControl) APPLY(RightControl) APPLY(LeftAlt) APPLY(RightAlt)
#define CNA_DEMOS_KEYS_BROWSER_MEDIA(APPLY) \
    APPLY(BrowserBack) APPLY(BrowserForward) APPLY(BrowserRefresh) APPLY(BrowserStop) APPLY(BrowserSearch) \
    APPLY(BrowserFavorites) APPLY(BrowserHome) APPLY(VolumeMute) APPLY(VolumeDown) APPLY(VolumeUp) \
    APPLY(MediaNextTrack) APPLY(MediaPreviousTrack) APPLY(MediaStop) APPLY(MediaPlayPause) \
    APPLY(LaunchMail) APPLY(SelectMedia) APPLY(LaunchApplication1) APPLY(LaunchApplication2)
#define CNA_DEMOS_KEYS_OEM(APPLY) \
    APPLY(OemSemicolon) APPLY(OemPlus) APPLY(OemComma) APPLY(OemMinus) APPLY(OemPeriod) APPLY(OemQuestion) APPLY(OemTilde) \
    APPLY(ChatPadGreen) APPLY(ChatPadOrange) APPLY(OemOpenBrackets) APPLY(OemPipe) APPLY(OemCloseBrackets) \
    APPLY(OemQuotes) APPLY(Oem8) APPLY(OemBackslash)
#define CNA_DEMOS_KEYS_LEGACY(APPLY) \
    APPLY(ProcessKey) APPLY(OemCopy) APPLY(OemAuto) APPLY(OemEnlW) APPLY(Attn) APPLY(Crsel) APPLY(Exsel) APPLY(EraseEof) \
    APPLY(Play) APPLY(Zoom) APPLY(Pa1) APPLY(OemClear)

namespace
{
    // Game logic that reads a pair of snapshots instead of the keyboard can run on recorded input.
    bool PressedThisFrame(const KeyboardState& previous, const KeyboardState& current, Keys key)
    {
        return current.IsKeyDown(key) && previous.IsKeyUp(key);
    }

    void CheckKeyFamily(CnaDemos::DemoContext& ctx, std::initializer_list<Keys> keys, std::string_view family)
    {
        const KeyboardState state(keys);
        std::vector<Keys> expected(keys);
        std::sort(expected.begin(), expected.end(), [](const Keys left, const Keys right) {
            return static_cast<int>(left) < static_cast<int>(right);
        });
        bool everyKeyIsDown = true;
        for (const Keys key : keys)
            everyKeyIsDown = everyKeyIsDown && state.IsKeyDown(key) && state[key] == KeyState::Down;
        ctx.Check(state.GetPressedKeys() == expected && everyKeyIsDown,
                  std::string(family) + " keys survive construction, queries and numeric ordering");
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

#define CNA_DEMOS_KEY_VALUE(name) Keys::name,
        CheckKeyFamily(ctx, {CNA_DEMOS_KEYS_CONTROL(CNA_DEMOS_KEY_VALUE)}, "control/navigation/IME");
        CheckKeyFamily(ctx, {CNA_DEMOS_KEYS_ALPHANUMERIC(CNA_DEMOS_KEY_VALUE)}, "alphanumeric/system");
        CheckKeyFamily(ctx, {CNA_DEMOS_KEYS_NUMPAD(CNA_DEMOS_KEY_VALUE)}, "numeric keypad");
        CheckKeyFamily(ctx, {CNA_DEMOS_KEYS_FUNCTION_MODIFIER(CNA_DEMOS_KEY_VALUE)}, "function/lock/modifier");
        CheckKeyFamily(ctx, {CNA_DEMOS_KEYS_BROWSER_MEDIA(CNA_DEMOS_KEY_VALUE)}, "browser/media/launch");
        CheckKeyFamily(ctx, {CNA_DEMOS_KEYS_OEM(CNA_DEMOS_KEY_VALUE)}, "OEM/chatpad");
        CheckKeyFamily(ctx, {CNA_DEMOS_KEYS_LEGACY(CNA_DEMOS_KEY_VALUE)}, "legacy process/editing");
#undef CNA_DEMOS_KEY_VALUE

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

#define CNA_DEMOS_KEY_API(name) "Keys::" #name,

CNA_DEMOS_CONSOLE(StateValues, {
    .id = "input.keyboard.state_values",
    .title = "KeyboardState values",
    .description = "Builds KeyboardState values from keys, queries them with IsKeyDown, IsKeyUp, the indexer and "
                   "GetPressedKeys, compares them, and counts key presses in a recorded sequence of states.",
    .apis = {"KeyboardState::KeyboardState", "KeyboardState::IsKeyDown", "KeyboardState::IsKeyUp",
             "KeyboardState::Item", "KeyboardState::GetPressedKeys", "KeyboardState::Equals",
             "KeyboardState::GetHashCode", "KeyboardState::op_Equality", "KeyboardState::op_Inequality",
             "KeyState", "KeyState::Down", "KeyState::Up", "Keys",
             CNA_DEMOS_KEYS_CONTROL(CNA_DEMOS_KEY_API)
             CNA_DEMOS_KEYS_ALPHANUMERIC(CNA_DEMOS_KEY_API)
             CNA_DEMOS_KEYS_NUMPAD(CNA_DEMOS_KEY_API)
             CNA_DEMOS_KEYS_FUNCTION_MODIFIER(CNA_DEMOS_KEY_API)
             CNA_DEMOS_KEYS_BROWSER_MEDIA(CNA_DEMOS_KEY_API)
             CNA_DEMOS_KEYS_OEM(CNA_DEMOS_KEY_API)
             CNA_DEMOS_KEYS_LEGACY(CNA_DEMOS_KEY_API)
             },
    .tags = {"keys", "replay", "testing", "value"},
})

#undef CNA_DEMOS_KEY_API
#undef CNA_DEMOS_KEYS_CONTROL
#undef CNA_DEMOS_KEYS_ALPHANUMERIC
#undef CNA_DEMOS_KEYS_NUMPAD
#undef CNA_DEMOS_KEYS_FUNCTION_MODIFIER
#undef CNA_DEMOS_KEYS_BROWSER_MEDIA
#undef CNA_DEMOS_KEYS_OEM
#undef CNA_DEMOS_KEYS_LEGACY
