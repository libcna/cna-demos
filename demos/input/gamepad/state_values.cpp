// SPDX-License-Identifier: MS-PL
// GamePadState as a value: build one, and the virtual stick and trigger buttons it derives.
#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/Buttons.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadButtons.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadDPad.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadState.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadThumbSticks.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadTriggers.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Input;

namespace
{
    void StateValues(CnaDemos::DemoContext& ctx)
    {
        // Left stick pushed right, right stick pulled down, left trigger half in, A and D-pad up
        // held. Built states serve tests, input replay and on-screen virtual controllers.
        const GamePadState state(Vector2(1.0f, 0.0f), Vector2(0.0f, -1.0f), 0.5f, 0.0f,
                                 {Buttons::A, Buttons::DPadUp});
        ctx.Print("ThumbSticks {}, Triggers {}, Buttons {}", state.getThumbSticksProperty().ToString(),
                  state.getTriggersProperty().ToString(), state.getButtonsProperty().ToString());
        ctx.Print("IsConnected of a constructed state: {}", state.getIsConnectedProperty());

        ctx.Check(state.IsButtonDown(Buttons::A) && state.getButtonsProperty().getAProperty() == ButtonState::Pressed,
                  "A is down, through IsButtonDown and through Buttons.A");
        ctx.Check(state.getDPadProperty().getUpProperty() == ButtonState::Pressed && state.IsButtonUp(Buttons::DPadDown),
                  "Buttons::DPadUp in the list shows up as DPad.Up");

        // Sticks and triggers also count as buttons, which makes menu navigation and "any
        // button" checks uniform.
        ctx.Check(state.IsButtonDown(Buttons::LeftThumbstickRight) && state.IsButtonUp(Buttons::LeftThumbstickLeft),
                  "the left stick pushed right is Buttons::LeftThumbstickRight");
        ctx.Check(state.IsButtonDown(Buttons::RightThumbstickDown), "the right stick pulled down is RightThumbstickDown");
        ctx.Check(state.IsButtonDown(Buttons::LeftTrigger) && state.IsButtonUp(Buttons::RightTrigger),
                  "a trigger past its threshold is a button too");
        ctx.Check(state.IsButtonDown(Buttons::A | Buttons::LeftTrigger),
                  "IsButtonDown with several flags is true when all of them are down");
        ctx.Check(!state.IsButtonDown(Buttons::A | Buttons::B), "...and false when one of them is up");

        // Stick positions are clamped to -1..1 per axis, trigger values to 0..1.
        const GamePadThumbSticks sticks(Vector2(2.0f, 0.5f), Vector2(-3.0f, -3.0f));
        ctx.Check(sticks.getLeftProperty() == Vector2(1.0f, 0.5f) && sticks.getRightProperty() == Vector2(-1.0f, -1.0f),
                  "GamePadThumbSticks clamps each axis to -1..1");
        const GamePadTriggers triggers(1.5f, -0.5f);
        ctx.Check(triggers.getLeftProperty() == 1.0f && triggers.getRightProperty() == 0.0f,
                  "GamePadTriggers clamps to 0..1");

        // The four-part constructor takes each part as a value of its own (the button list of
        // the constructor above went into both the buttons and the D-pad).
        const GamePadState same(GamePadThumbSticks(Vector2(1.0f, 0.0f), Vector2(0.0f, -1.0f)),
                                GamePadTriggers(0.5f, 0.0f), GamePadButtons(Buttons::A | Buttons::DPadUp),
                                GamePadDPad(ButtonState::Pressed, ButtonState::Released, ButtonState::Released,
                                            ButtonState::Released));
        ctx.Check(same == state, "the same controls built part by part give an equal state");
        ctx.Check(same != GamePadState(), "a state at rest differs");
    }
}

CNA_DEMOS_CONSOLE(StateValues, {
    .id = "input.gamepad.state_values",
    .title = "GamePadState values",
    .description = "Builds GamePadState values from sticks, triggers and buttons and shows the virtual buttons XNA "
                   "derives from them (LeftThumbstickRight, LeftTrigger, ...), multi-flag IsButtonDown, the clamping "
                   "of sticks and triggers, and equality.",
    .apis = {"GamePadState::GamePadState", "GamePadState::IsButtonDown", "GamePadState::IsButtonUp",
             "GamePadState::Buttons", "GamePadState::DPad", "GamePadState::IsConnected", "GamePadState::ThumbSticks",
             "GamePadState::Triggers", "GamePadThumbSticks::ToString", "GamePadTriggers::ToString",
             "GamePadButtons::ToString",
             "GamePadState::op_Equality", "GamePadState::op_Inequality", "GamePadThumbSticks::GamePadThumbSticks",
             "GamePadTriggers::GamePadTriggers", "GamePadButtons::GamePadButtons", "GamePadDPad::GamePadDPad",
             "Buttons", "GamePadButtons::A", "GamePadDPad::Up", "GamePadThumbSticks::Left",
             "GamePadThumbSticks::Right", "GamePadTriggers::Left", "GamePadTriggers::Right"},
    .tags = {"gamepad", "controller", "virtual-buttons", "testing", "value"},
})
