// SPDX-License-Identifier: MS-PL
// ISSUE-108: direct programmatic regions have the same documented validation boundary.
#include "CNA/Content/Pipeline/SpriteFontContentPipeline.hpp"
#include <iostream>
#include <stdexcept>
#include <typeinfo>

namespace Pipeline = CNA::Content::Pipeline;
namespace
{
    int checks = 0, failed = 0;
    void Check(bool condition, const char* label)
    {
        ++checks;
        if (!condition) ++failed;
        std::cout << (condition ? "PASS " : "FAIL ") << label << '\n';
    }
    void Refuses(const Pipeline::FontDescription& description, const char* label)
    {
        bool exact = false;
        try { (void)Pipeline::ExpandCharacterRegions(description); }
        catch (const std::runtime_error& error)
        {
            exact = typeid(error) == typeid(std::runtime_error);
        }
        catch (...) { }
        Check(exact, label);
    }
}
int main()
{
    try
    {
        Pipeline::FontDescription description;
        Check(Pipeline::ExpandCharacterRegions(description).empty(), "empty direct description stays empty");
        description.characterRegions = {{u'a', u'c'}, {u'b', u'd'}, {u'A', u'A'}};
        const std::vector<SharpRuntime::charcs> expected{u'A', u'a', u'b', u'c', u'd'};
        Check(Pipeline::ExpandCharacterRegions(description) == expected, "overlapping regions sort and deduplicate");
        const auto original = description;
        (void)Pipeline::ExpandCharacterRegions(description);
        Check(description == original, "expansion does not mutate authored input");
        description.defaultCharacter = u'0';
        const std::vector<SharpRuntime::charcs> fallback{u'0', u'A', u'a', u'b', u'c', u'd'};
        Check(Pipeline::ExpandCharacterRegions(description) == fallback, "outside fallback joins sorted characters");
        description.defaultCharacter = u'b';
        Check(Pipeline::ExpandCharacterRegions(description) == expected, "inside fallback does not duplicate a character");
        description.defaultCharacter.reset();
        description.characterRegions = {{u'\uffff', u'\uffff'}};
        Check(Pipeline::ExpandCharacterRegions(description) == std::vector<SharpRuntime::charcs>{u'\uffff'}, "last UTF16 code unit terminates inclusively");
        description.characterRegions = {{0, 8191}};
        Check(Pipeline::ExpandCharacterRegions(description).size() == 8192, "exact glyph ceiling succeeds");
        description.characterRegions = {{0, 8192}};
        Refuses(description, "one more than glyph ceiling is refused");
        description.characterRegions = {{u'z', u'a'}};
        Refuses(description, "sole inverted region is refused");
        description.characterRegions = {{u'a', u'b'}, {u'z', u'c'}};
        Refuses(description, "trailing inverted region is refused");
        description.characterRegions = {{u'z', u'c'}, {u'a', u'b'}};
        Refuses(description, "leading inverted region is refused");
        description.characterRegions = {{u'\uffff', u'\0'}};
        description.defaultCharacter = u'?';
        Refuses(description, "fallback cannot hide an inverted endpoint range");
        std::cout << "checks=" << checks << " failed=" << failed << '\n';
        return failed ? 1 : 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Unexpected fixture error: " << error.what() << '\n';
        return 2;
    }
}
