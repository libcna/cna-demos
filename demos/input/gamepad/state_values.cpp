// SPDX-License-Identifier: MS-PL
// GamePadState as a value: build one, and the virtual stick and trigger buttons it derives.
#include <any>
#include <array>

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
    template<std::size_t N>
    bool AllDown(const GamePadState& state, const std::array<Buttons, N>& buttons)
    {
        for (const Buttons button : buttons)
            if (!state.IsButtonDown(button)) return false;
        return true;
    }

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

        const std::array physicalFlags = {
            Buttons::A, Buttons::B, Buttons::X, Buttons::Y, Buttons::Back, Buttons::Start,
            Buttons::BigButton, Buttons::LeftShoulder, Buttons::RightShoulder,
            Buttons::LeftStick, Buttons::RightStick,
            Buttons::DPadUp, Buttons::DPadDown, Buttons::DPadLeft, Buttons::DPadRight};
        const GamePadState physical(
            Vector2::Zero, Vector2::Zero, 0.0f, 0.0f,
            {Buttons::A, Buttons::B, Buttons::X, Buttons::Y, Buttons::Back, Buttons::Start,
             Buttons::BigButton, Buttons::LeftShoulder, Buttons::RightShoulder,
             Buttons::LeftStick, Buttons::RightStick,
             Buttons::DPadUp, Buttons::DPadDown, Buttons::DPadLeft, Buttons::DPadRight});
        const GamePadButtons& physicalButtons = physical.getButtonsProperty();
        const GamePadDPad& physicalDPad = physical.getDPadProperty();
        ctx.Check(AllDown(physical, physicalFlags) &&
                      physicalButtons.getAProperty() == ButtonState::Pressed &&
                      physicalButtons.getBProperty() == ButtonState::Pressed &&
                      physicalButtons.getXProperty() == ButtonState::Pressed &&
                      physicalButtons.getYProperty() == ButtonState::Pressed &&
                      physicalButtons.getBackProperty() == ButtonState::Pressed &&
                      physicalButtons.getStartProperty() == ButtonState::Pressed &&
                      physicalButtons.getBigButtonProperty() == ButtonState::Pressed &&
                      physicalButtons.getLeftShoulderProperty() == ButtonState::Pressed &&
                      physicalButtons.getRightShoulderProperty() == ButtonState::Pressed &&
                      physicalButtons.getLeftStickProperty() == ButtonState::Pressed &&
                      physicalButtons.getRightStickProperty() == ButtonState::Pressed &&
                      physicalDPad.getUpProperty() == ButtonState::Pressed &&
                      physicalDPad.getDownProperty() == ButtonState::Pressed &&
                      physicalDPad.getLeftProperty() == ButtonState::Pressed &&
                      physicalDPad.getRightProperty() == ButtonState::Pressed,
                  "all fifteen physical button flags reach IsButtonDown and their component properties");

        const GamePadButtons fromArray = GamePadButtons::FromButtonArray(
            {Buttons::A, Buttons::LeftShoulder, Buttons::Start});
        ctx.Check(fromArray == GamePadButtons(Buttons::A | Buttons::LeftShoulder | Buttons::Start) &&
                      fromArray.getAProperty() == ButtonState::Pressed &&
                      fromArray.getLeftShoulderProperty() == ButtonState::Pressed &&
                      fromArray.getStartProperty() == ButtonState::Pressed,
                  "FromButtonArray combines a list into the same button flags as the bitwise constructor");

        const GamePadState positiveAnalog(Vector2(1.0f, 1.0f), Vector2(1.0f, 1.0f), 1.0f, 1.0f, {});
        const std::array positiveFlags = {
            Buttons::LeftThumbstickRight, Buttons::LeftThumbstickUp,
            Buttons::RightThumbstickRight, Buttons::RightThumbstickUp,
            Buttons::LeftTrigger, Buttons::RightTrigger};
        ctx.Check(AllDown(positiveAnalog, positiveFlags) &&
                      positiveAnalog.IsButtonUp(Buttons::LeftThumbstickLeft) &&
                      positiveAnalog.IsButtonUp(Buttons::LeftThumbstickDown) &&
                      positiveAnalog.IsButtonUp(Buttons::RightThumbstickLeft) &&
                      positiveAnalog.IsButtonUp(Buttons::RightThumbstickDown),
                  "positive stick axes and both triggers derive their six virtual button flags");

        const GamePadState negativeAnalog(Vector2(-1.0f, -1.0f), Vector2(-1.0f, -1.0f), 0.0f, 0.0f, {});
        const std::array negativeFlags = {
            Buttons::LeftThumbstickLeft, Buttons::LeftThumbstickDown,
            Buttons::RightThumbstickLeft, Buttons::RightThumbstickDown};
        ctx.Check(AllDown(negativeAnalog, negativeFlags) &&
                      negativeAnalog.IsButtonUp(Buttons::LeftThumbstickRight) &&
                      negativeAnalog.IsButtonUp(Buttons::LeftThumbstickUp) &&
                      negativeAnalog.IsButtonUp(Buttons::RightThumbstickRight) &&
                      negativeAnalog.IsButtonUp(Buttons::RightThumbstickUp),
                  "negative stick axes derive the four opposite virtual button flags");

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

        ctx.Check(state.Equals(same) && state.Equals(std::any(same)) &&
                      !state.Equals(std::any(state.getButtonsProperty())) &&
                      state.GetHashCode() == same.GetHashCode(),
                  "typed and boxed state equality agree, reject another value type and share a hash");
        ctx.Check(state.ToString() == "{IsConnected:True}" &&
                      GamePadState().ToString() == "{IsConnected:False}",
                  "GamePadState's diagnostic string is XNA's {IsConnected:...}");

        const GamePadButtons buttonsCopy = state.getButtonsProperty();
        const GamePadDPad dPadCopy = state.getDPadProperty();
        const GamePadThumbSticks sticksCopy = state.getThumbSticksProperty();
        const GamePadTriggers triggersCopy = state.getTriggersProperty();
        ctx.Check(buttonsCopy == same.getButtonsProperty() &&
                      buttonsCopy.Equals(same.getButtonsProperty()) &&
                      buttonsCopy.Equals(std::any(same.getButtonsProperty())) &&
                      buttonsCopy.GetHashCode() == same.getButtonsProperty().GetHashCode() &&
                      buttonsCopy != GamePadButtons(),
                  "GamePadButtons equality and hashing observe the complete flag value");
        ctx.Check(dPadCopy == same.getDPadProperty() && dPadCopy.Equals(same.getDPadProperty()) &&
                      dPadCopy.Equals(std::any(same.getDPadProperty())) &&
                      dPadCopy.GetHashCode() == same.getDPadProperty().GetHashCode() &&
                      dPadCopy != GamePadDPad(),
                  "GamePadDPad equality and hashing observe all four directions");
        ctx.Check(sticksCopy == same.getThumbSticksProperty() &&
                      sticksCopy.Equals(same.getThumbSticksProperty()) &&
                      sticksCopy.Equals(std::any(same.getThumbSticksProperty())) &&
                      sticksCopy.GetHashCode() == same.getThumbSticksProperty().GetHashCode() &&
                      sticksCopy != GamePadThumbSticks(),
                  "GamePadThumbSticks equality and hashing observe both vectors");
        ctx.Check(triggersCopy == same.getTriggersProperty() &&
                      triggersCopy.Equals(same.getTriggersProperty()) &&
                      triggersCopy.Equals(std::any(same.getTriggersProperty())) &&
                      triggersCopy.GetHashCode() == same.getTriggersProperty().GetHashCode() &&
                      triggersCopy != GamePadTriggers(),
                  "GamePadTriggers equality and hashing observe both clamped values");
        ctx.Check(buttonsCopy.ToString() == "{Buttons:A}" && dPadCopy.ToString() == "{DPad:Up}" &&
                      sticksCopy.ToString() == "{Left:{X:1 Y:0} Right:{X:0 Y:-1}}" &&
                      triggersCopy.ToString() == "{Left:0.5 Right:0}",
                  "the four component values use exact field-labelled diagnostic text");
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
             "GamePadState::Equals", "GamePadState::GetHashCode", "GamePadState::ToString",
             "GamePadState::op_Equality", "GamePadState::op_Inequality",
             "GamePadButtons::Equals", "GamePadButtons::GetHashCode", "GamePadButtons::op_Equality",
             "GamePadButtons::op_Inequality", "GamePadDPad::Equals", "GamePadDPad::GetHashCode",
             "GamePadDPad::ToString", "GamePadDPad::op_Equality", "GamePadDPad::op_Inequality",
             "GamePadThumbSticks::Equals", "GamePadThumbSticks::GetHashCode",
             "GamePadThumbSticks::op_Equality", "GamePadThumbSticks::op_Inequality",
             "GamePadTriggers::Equals", "GamePadTriggers::GetHashCode", "GamePadTriggers::op_Equality",
             "GamePadTriggers::op_Inequality", "GamePadThumbSticks::GamePadThumbSticks",
             "GamePadTriggers::GamePadTriggers", "GamePadButtons::GamePadButtons",
             "GamePadButtons::FromButtonArray", "GamePadDPad::GamePadDPad",
             "Buttons", "GamePadButtons::A", "GamePadDPad::Up", "GamePadThumbSticks::Left",
             "GamePadThumbSticks::Right", "GamePadTriggers::Left", "GamePadTriggers::Right",
             "Buttons::A", "Buttons::B", "Buttons::X", "Buttons::Y", "Buttons::Back", "Buttons::Start",
             "Buttons::BigButton", "Buttons::LeftShoulder", "Buttons::RightShoulder",
             "Buttons::LeftStick", "Buttons::RightStick", "Buttons::DPadUp", "Buttons::DPadDown",
             "Buttons::DPadLeft", "Buttons::DPadRight", "Buttons::LeftTrigger", "Buttons::RightTrigger",
             "Buttons::LeftThumbstickLeft", "Buttons::LeftThumbstickRight",
             "Buttons::LeftThumbstickUp", "Buttons::LeftThumbstickDown",
             "Buttons::RightThumbstickLeft", "Buttons::RightThumbstickRight",
             "Buttons::RightThumbstickUp", "Buttons::RightThumbstickDown"},
    .tags = {"gamepad", "controller", "virtual-buttons", "testing", "value"},
})
