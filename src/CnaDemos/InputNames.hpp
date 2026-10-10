// SPDX-License-Identifier: MS-PL
#pragma once

#include <string_view>

#include "CNA/Input/GamePadButtonLabel.hpp"
#include "CNA/Input/GamePadConnectionState.hpp"
#include "CNA/Input/JoystickHatPosition.hpp"
#include "CNA/Input/JoystickType.hpp"
#include "CNA/Input/PowerState.hpp"
#include "CNA/Input/Sensors.hpp"
#include "Microsoft/Xna/Framework/Input/ButtonState.hpp"
#include "Microsoft/Xna/Framework/Input/Buttons.hpp"
#include "Microsoft/Xna/Framework/Input/GamePadType.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/GestureType.hpp"
#include "Microsoft/Xna/Framework/Input/Touch/TouchLocationState.hpp"

/**
 * Names of input enumerations for demo output -- what C#'s Enum.ToString() gives an XNA game.
 * Buttons and GestureType are flag enums; Name() names a single flag. Output formatting only --
 * no demo is about these functions.
 */
namespace CnaDemos
{
    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Input::Keys value)
    {
        using enum Microsoft::Xna::Framework::Input::Keys;
        switch (value)
        {
        case None: return "None";
        case Back: return "Back";
        case Tab: return "Tab";
        case Enter: return "Enter";
        case Pause: return "Pause";
        case CapsLock: return "CapsLock";
        case Kana: return "Kana";
        case Kanji: return "Kanji";
        case Escape: return "Escape";
        case ImeConvert: return "ImeConvert";
        case ImeNoConvert: return "ImeNoConvert";
        case Space: return "Space";
        case PageUp: return "PageUp";
        case PageDown: return "PageDown";
        case End: return "End";
        case Home: return "Home";
        case Left: return "Left";
        case Up: return "Up";
        case Right: return "Right";
        case Down: return "Down";
        case Select: return "Select";
        case Print: return "Print";
        case Execute: return "Execute";
        case PrintScreen: return "PrintScreen";
        case Insert: return "Insert";
        case Delete: return "Delete";
        case Help: return "Help";
        case D0: return "D0";
        case D1: return "D1";
        case D2: return "D2";
        case D3: return "D3";
        case D4: return "D4";
        case D5: return "D5";
        case D6: return "D6";
        case D7: return "D7";
        case D8: return "D8";
        case D9: return "D9";
        case A: return "A";
        case B: return "B";
        case C: return "C";
        case D: return "D";
        case E: return "E";
        case F: return "F";
        case G: return "G";
        case H: return "H";
        case I: return "I";
        case J: return "J";
        case K: return "K";
        case L: return "L";
        case M: return "M";
        case N: return "N";
        case O: return "O";
        case P: return "P";
        case Q: return "Q";
        case R: return "R";
        case S: return "S";
        case T: return "T";
        case U: return "U";
        case V: return "V";
        case W: return "W";
        case X: return "X";
        case Y: return "Y";
        case Z: return "Z";
        case LeftWindows: return "LeftWindows";
        case RightWindows: return "RightWindows";
        case Apps: return "Apps";
        case Sleep: return "Sleep";
        case NumPad0: return "NumPad0";
        case NumPad1: return "NumPad1";
        case NumPad2: return "NumPad2";
        case NumPad3: return "NumPad3";
        case NumPad4: return "NumPad4";
        case NumPad5: return "NumPad5";
        case NumPad6: return "NumPad6";
        case NumPad7: return "NumPad7";
        case NumPad8: return "NumPad8";
        case NumPad9: return "NumPad9";
        case Multiply: return "Multiply";
        case Add: return "Add";
        case Separator: return "Separator";
        case Subtract: return "Subtract";
        case Decimal: return "Decimal";
        case Divide: return "Divide";
        case F1: return "F1";
        case F2: return "F2";
        case F3: return "F3";
        case F4: return "F4";
        case F5: return "F5";
        case F6: return "F6";
        case F7: return "F7";
        case F8: return "F8";
        case F9: return "F9";
        case F10: return "F10";
        case F11: return "F11";
        case F12: return "F12";
        case F13: return "F13";
        case F14: return "F14";
        case F15: return "F15";
        case F16: return "F16";
        case F17: return "F17";
        case F18: return "F18";
        case F19: return "F19";
        case F20: return "F20";
        case F21: return "F21";
        case F22: return "F22";
        case F23: return "F23";
        case F24: return "F24";
        case NumLock: return "NumLock";
        case Scroll: return "Scroll";
        case LeftShift: return "LeftShift";
        case RightShift: return "RightShift";
        case LeftControl: return "LeftControl";
        case RightControl: return "RightControl";
        case LeftAlt: return "LeftAlt";
        case RightAlt: return "RightAlt";
        case BrowserBack: return "BrowserBack";
        case BrowserForward: return "BrowserForward";
        case BrowserRefresh: return "BrowserRefresh";
        case BrowserStop: return "BrowserStop";
        case BrowserSearch: return "BrowserSearch";
        case BrowserFavorites: return "BrowserFavorites";
        case BrowserHome: return "BrowserHome";
        case VolumeMute: return "VolumeMute";
        case VolumeDown: return "VolumeDown";
        case VolumeUp: return "VolumeUp";
        case MediaNextTrack: return "MediaNextTrack";
        case MediaPreviousTrack: return "MediaPreviousTrack";
        case MediaStop: return "MediaStop";
        case MediaPlayPause: return "MediaPlayPause";
        case LaunchMail: return "LaunchMail";
        case SelectMedia: return "SelectMedia";
        case LaunchApplication1: return "LaunchApplication1";
        case LaunchApplication2: return "LaunchApplication2";
        case OemSemicolon: return "OemSemicolon";
        case OemPlus: return "OemPlus";
        case OemComma: return "OemComma";
        case OemMinus: return "OemMinus";
        case OemPeriod: return "OemPeriod";
        case OemQuestion: return "OemQuestion";
        case OemTilde: return "OemTilde";
        case ChatPadGreen: return "ChatPadGreen";
        case ChatPadOrange: return "ChatPadOrange";
        case OemOpenBrackets: return "OemOpenBrackets";
        case OemPipe: return "OemPipe";
        case OemCloseBrackets: return "OemCloseBrackets";
        case OemQuotes: return "OemQuotes";
        case Oem8: return "Oem8";
        case OemBackslash: return "OemBackslash";
        case ProcessKey: return "ProcessKey";
        case OemCopy: return "OemCopy";
        case OemAuto: return "OemAuto";
        case OemEnlW: return "OemEnlW";
        case Attn: return "Attn";
        case Crsel: return "Crsel";
        case Exsel: return "Exsel";
        case EraseEof: return "EraseEof";
        case Play: return "Play";
        case Zoom: return "Zoom";
        case Pa1: return "Pa1";
        case OemClear: return "OemClear";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Input::Buttons value)
    {
        using enum Microsoft::Xna::Framework::Input::Buttons;
        switch (value)
        {
        case DPadUp: return "DPadUp";
        case DPadDown: return "DPadDown";
        case DPadLeft: return "DPadLeft";
        case DPadRight: return "DPadRight";
        case Start: return "Start";
        case Back: return "Back";
        case LeftStick: return "LeftStick";
        case RightStick: return "RightStick";
        case LeftShoulder: return "LeftShoulder";
        case RightShoulder: return "RightShoulder";
        case BigButton: return "BigButton";
        case A: return "A";
        case B: return "B";
        case X: return "X";
        case Y: return "Y";
        case LeftThumbstickLeft: return "LeftThumbstickLeft";
        case RightTrigger: return "RightTrigger";
        case LeftTrigger: return "LeftTrigger";
        case RightThumbstickUp: return "RightThumbstickUp";
        case RightThumbstickDown: return "RightThumbstickDown";
        case RightThumbstickRight: return "RightThumbstickRight";
        case RightThumbstickLeft: return "RightThumbstickLeft";
        case LeftThumbstickUp: return "LeftThumbstickUp";
        case LeftThumbstickDown: return "LeftThumbstickDown";
        case LeftThumbstickRight: return "LeftThumbstickRight";
        case Misc1EXT: return "Misc1EXT";
        case Paddle1EXT: return "Paddle1EXT";
        case Paddle2EXT: return "Paddle2EXT";
        case Paddle3EXT: return "Paddle3EXT";
        case Paddle4EXT: return "Paddle4EXT";
        case TouchPadEXT: return "TouchPadEXT";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Input::GamePadType value)
    {
        using enum Microsoft::Xna::Framework::Input::GamePadType;
        switch (value)
        {
        case Unknown: return "Unknown";
        case GamePad: return "GamePad";
        case Wheel: return "Wheel";
        case ArcadeStick: return "ArcadeStick";
        case FlightStick: return "FlightStick";
        case DancePad: return "DancePad";
        case Guitar: return "Guitar";
        case AlternateGuitar: return "AlternateGuitar";
        case DrumKit: return "DrumKit";
        case BigButtonPad: return "BigButtonPad";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Input::Touch::GestureType value)
    {
        using enum Microsoft::Xna::Framework::Input::Touch::GestureType;
        switch (value)
        {
        case None: return "None";
        case Tap: return "Tap";
        case DoubleTap: return "DoubleTap";
        case Hold: return "Hold";
        case HorizontalDrag: return "HorizontalDrag";
        case VerticalDrag: return "VerticalDrag";
        case FreeDrag: return "FreeDrag";
        case Pinch: return "Pinch";
        case Flick: return "Flick";
        case DragComplete: return "DragComplete";
        case PinchComplete: return "PinchComplete";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Input::ButtonState value)
    {
        return value == Microsoft::Xna::Framework::Input::ButtonState::Pressed ? "Pressed" : "Released";
    }

    [[nodiscard]] constexpr std::string_view Name(Microsoft::Xna::Framework::Input::Touch::TouchLocationState value)
    {
        using enum Microsoft::Xna::Framework::Input::Touch::TouchLocationState;
        switch (value)
        {
        case Invalid: return "Invalid";
        case Released: return "Released";
        case Pressed: return "Pressed";
        case Moved: return "Moved";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(CNA::Input::GamePadConnectionStateEXT value)
    {
        using enum CNA::Input::GamePadConnectionStateEXT;
        switch (value)
        {
        case Unknown: return "Unknown";
        case Wired: return "Wired";
        case Wireless: return "Wireless";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(CNA::Input::PowerStateEXT value)
    {
        using enum CNA::Input::PowerStateEXT;
        switch (value)
        {
        case Error: return "Error";
        case Unknown: return "Unknown";
        case OnBattery: return "OnBattery";
        case NoBattery: return "NoBattery";
        case Charging: return "Charging";
        case Charged: return "Charged";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(CNA::Input::GamePadButtonLabelEXT value)
    {
        using enum CNA::Input::GamePadButtonLabelEXT;
        switch (value)
        {
        case Unknown: return "Unknown";
        case A: return "A";
        case B: return "B";
        case X: return "X";
        case Y: return "Y";
        case Cross: return "Cross";
        case Circle: return "Circle";
        case Square: return "Square";
        case Triangle: return "Triangle";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(CNA::Input::JoystickTypeEXT value)
    {
        using enum CNA::Input::JoystickTypeEXT;
        switch (value)
        {
        case Unknown: return "Unknown";
        case Gamepad: return "Gamepad";
        case Wheel: return "Wheel";
        case ArcadeStick: return "ArcadeStick";
        case FlightStick: return "FlightStick";
        case DancePad: return "DancePad";
        case Guitar: return "Guitar";
        case DrumKit: return "DrumKit";
        case ArcadePad: return "ArcadePad";
        case Throttle: return "Throttle";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(CNA::Input::JoystickHatPositionEXT value)
    {
        using enum CNA::Input::JoystickHatPositionEXT;
        switch (value)
        {
        case Centered: return "Centered";
        case Up: return "Up";
        case Right: return "Right";
        case Down: return "Down";
        case Left: return "Left";
        case RightUp: return "RightUp";
        case RightDown: return "RightDown";
        case LeftUp: return "LeftUp";
        case LeftDown: return "LeftDown";
        }
        return "?";
    }

    [[nodiscard]] constexpr std::string_view Name(CNA::Input::SensorTypeEXT value)
    {
        using enum CNA::Input::SensorTypeEXT;
        switch (value)
        {
        case Unknown: return "Unknown";
        case Accelerometer: return "Accelerometer";
        case Gyroscope: return "Gyroscope";
        case AccelerometerLeft: return "AccelerometerLeft";
        case GyroscopeLeft: return "GyroscopeLeft";
        case AccelerometerRight: return "AccelerometerRight";
        case GyroscopeRight: return "GyroscopeRight";
        }
        return "?";
    }
}
