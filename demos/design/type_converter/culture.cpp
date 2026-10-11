// SPDX-License-Identifier: MS-PL
// TypeConverter text in a given culture: its decimal and list separators.
#include <any>
#include <string>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Design.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "System/ArgumentException.hpp"
#include "System/Globalization/CultureInfo.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::Design;
using System::Globalization::CultureInfo;

namespace
{
    void Culture(CnaDemos::DemoContext& ctx)
    {
        const Vector3Converter converter;
        const Vector3 value(1.5f, 2.25f, -3.75f);
        // A property grid shows values in the user's culture; files should use the invariant one.
        const CultureInfo invariant("");
        const CultureInfo english("en-US");
        const CultureInfo czech("cs-CZ"); // decimal comma, so the list separator is ';'

        const std::string inInvariant = converter.ConvertToString(nullptr, &invariant, std::any(value));
        const std::string inEnglish = converter.ConvertToString(nullptr, &english, std::any(value));
        const std::string inCzech = converter.ConvertToString(nullptr, &czech, std::any(value));
        ctx.Print("invariant: \"{}\"", inInvariant);
        ctx.Print("en-US:     \"{}\"", inEnglish);
        ctx.Print("cs-CZ:     \"{}\"", inCzech);
        ctx.Check(inInvariant == "1.5, 2.25, -3.75" && inEnglish == inInvariant, "en-US writes like the invariant culture");
        ctx.Check(inCzech == "1,5; 2,25; -3,75", "cs-CZ writes decimal commas and separates with \"; \"");

        ctx.Check(std::any_cast<Vector3>(converter.ConvertFromString(nullptr, &czech, " 1,5; 2,25; -3,75 ")) == value,
                  "cs-CZ text reads back in cs-CZ (surrounding spaces are ignored)");
        bool rejected = false;
        try
        {
            (void)converter.ConvertFromString(nullptr, &czech, "1.5, 2.25, -3.75");
        }
        catch (const System::ArgumentException&)
        {
            rejected = true;
        }
        ctx.Check(rejected, "invariant text read as cs-CZ is rejected: write and read in the same culture");
    }
}

CNA_DEMOS_CONSOLE(Culture, {
    .id = "design.type_converter.culture",
    .title = "Converter text and cultures",
    .description = "Writes the same Vector3 with Vector3Converter in the invariant, en-US and cs-CZ cultures (decimal "
                   "comma, ';' between components), reads cs-CZ text back, and shows that text must be read in the "
                   "culture it was written in.",
    .apis = {"Vector3Converter::ConvertTo", "Vector3Converter::ConvertFrom"},
    .tags = {"design", "type-converter", "culture", "localization", "text"},
})
