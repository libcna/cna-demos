// SPDX-License-Identifier: MS-PL
// MouseState as a constructed value: exact fields, equality, hashing and diagnostic text.
#include <any>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/MouseState.hpp"

using namespace Microsoft::Xna::Framework::Input;

namespace
{
    void StateValues(CnaDemos::DemoContext& ctx)
    {
        const MouseState resting;
        ctx.Check(resting.getXProperty() == 0 && resting.getYProperty() == 0 &&
                      resting.getScrollWheelValueProperty() == 0 &&
                      resting.getHorizontalScrollWheelValueEXTProperty() == 0 &&
                      resting.getLeftButtonProperty() == ButtonState::Released &&
                      resting.getMiddleButtonProperty() == ButtonState::Released &&
                      resting.getRightButtonProperty() == ButtonState::Released &&
                      resting.getXButton1Property() == ButtonState::Released &&
                      resting.getXButton2Property() == ButtonState::Released,
                  "a default MouseState has zero coordinates and wheels with every button released");

        const MouseState authored(12, -34, 240,
                                  ButtonState::Pressed, ButtonState::Released,
                                  ButtonState::Pressed, ButtonState::Pressed,
                                  ButtonState::Released);
        ctx.Check(authored.getXProperty() == 12 && authored.getYProperty() == -34 &&
                      authored.getScrollWheelValueProperty() == 240 &&
                      authored.getHorizontalScrollWheelValueEXTProperty() == 0 &&
                      authored.getLeftButtonProperty() == ButtonState::Pressed &&
                      authored.getMiddleButtonProperty() == ButtonState::Released &&
                      authored.getRightButtonProperty() == ButtonState::Pressed &&
                      authored.getXButton1Property() == ButtonState::Pressed &&
                      authored.getXButton2Property() == ButtonState::Released,
                  "the XNA constructor preserves coordinates, vertical wheel and all five button positions");

        const MouseState horizontal(12, -34, 240,
                                    ButtonState::Pressed, ButtonState::Released,
                                    ButtonState::Pressed, ButtonState::Pressed,
                                    ButtonState::Released, -120);
        ctx.Check(horizontal.getHorizontalScrollWheelValueEXTProperty() == -120,
                  "the CNA extension constructor preserves the horizontal wheel independently");
        ctx.Check(authored.Equals(horizontal) && authored == horizontal && !(authored != horizontal),
                  "XNA equality deliberately ignores CNA's additive horizontal-wheel field");
        ctx.Check(authored.Equals(std::any(horizontal)) && !authored.Equals(std::any(resting)) &&
                      !authored.Equals(std::any(17)) && !authored.Equals(std::any{}),
                  "boxed equality accepts the same MouseState value and rejects different, wrong and empty boxes");
        ctx.Check(authored.GetHashCode() == horizontal.GetHashCode(),
                  "states equal under the XNA contract have equal hashes");

        const MouseState moved(13, -34, 240,
                               ButtonState::Pressed, ButtonState::Released,
                               ButtonState::Pressed, ButtonState::Pressed,
                               ButtonState::Released);
        ctx.Check(moved != authored && !moved.Equals(authored),
                  "changing an XNA coordinate makes two MouseState values unequal");
        ctx.Check(authored.ToString() ==
                      "{X:12 Y:-34 Buttons:Left Right XButton1 Wheel:240}" &&
                      resting.ToString() == "{X:0 Y:0 Buttons:None Wheel:0}",
                  "ToString reports coordinates, pressed buttons in stable order and the vertical wheel");
    }
}

CNA_DEMOS_CONSOLE(StateValues, {
    .id = "input.mouse.state_values",
    .title = "MouseState values",
    .description = "Constructs mouse snapshots with exact coordinates, wheel totals and button states, then checks "
                   "typed and boxed equality, operators, hashing and diagnostic text. It also shows why CNA's "
                   "additive horizontal wheel is excluded from the XNA value contract.",
    .apis = {"MouseState::MouseState", "MouseState::X", "MouseState::Y", "MouseState::ScrollWheelValue",
             "MouseState::LeftButton", "MouseState::MiddleButton", "MouseState::RightButton",
             "MouseState::XButton1", "MouseState::XButton2", "MouseState::HorizontalScrollWheelValueEXT",
             "MouseState::Equals", "MouseState::GetHashCode", "MouseState::op_Equality",
             "MouseState::op_Inequality", "MouseState::ToString", "ButtonState",
             "ButtonState::Pressed", "ButtonState::Released"},
    .tags = {"mouse", "state", "value", "equality", "testing"},
})
