// SPDX-License-Identifier: MS-PL
// CNAEXT key names: the physical (scancode) name of a key, its name on the active layout, and back.
#include <initializer_list>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/InputNames.hpp"
#include "Microsoft/Xna/Framework/Input/Keyboard.hpp"
#include "Microsoft/Xna/Framework/Input/Keys.hpp"

using namespace Microsoft::Xna::Framework::Input;

namespace
{
    void KeyNames(CnaDemos::DemoContext& ctx)
    {
        // XNA's Keys values name positions on a US keyboard. Showing "Press W" to a French
        // player, whose key in that position is Z, needs the layout's name for the key.
        for (const Keys key : {Keys::W, Keys::A, Keys::D1, Keys::Space, Keys::Enter, Keys::LeftShift, Keys::F1,
                               Keys::Left, Keys::NumPad5, Keys::OemTilde})
        {
            const std::string physical = Keyboard::GetScancodeNameEXT(key);
            const std::string onLayout = Keyboard::GetKeyNameEXT(key);
            ctx.Print("{:<10} physical \"{}\", on this layout \"{}\", the layout puts {} there", CnaDemos::Name(key),
                      physical, onLayout, CnaDemos::Name(Keyboard::GetKeyFromScancodeEXT(key)));
            ctx.Check(!physical.empty() && Keyboard::GetScancodeFromNameEXT(physical) == key,
                      std::string(CnaDemos::Name(key)) + ": the physical name leads back to the key");
            ctx.Check(!onLayout.empty() && Keyboard::GetKeyFromNameEXT(onLayout) == key,
                      std::string(CnaDemos::Name(key)) + ": the layout name leads back to the key");
        }

        // Names typed by a person, for example in a key-binding configuration file.
        ctx.Check(Keyboard::GetKeyFromNameEXT("Space") == Keys::Space, "\"Space\" names Keys::Space");
        ctx.Check(Keyboard::GetKeyFromNameEXT("No Such Key") == Keys::None, "an unknown name gives Keys::None");
    }
}

CNA_DEMOS_CONSOLE(KeyNames, {
    .id = "input.keyboard.key_names",
    .title = "Key names (CNAEXT)",
    .description = "Prints the physical and layout names of keys with Keyboard::GetScancodeNameEXT and "
                   "GetKeyNameEXT, which key the active layout puts in a position (GetKeyFromScancodeEXT), and maps "
                   "names back to keys.",
    .apis = {"Keyboard::GetScancodeNameEXT", "Keyboard::GetKeyNameEXT", "Keyboard::GetScancodeFromNameEXT",
             "Keyboard::GetKeyFromNameEXT", "Keyboard::GetKeyFromScancodeEXT"},
    .tags = {"keys", "layout", "localization", "scancode", "cnaext", "key-binding"},
})
