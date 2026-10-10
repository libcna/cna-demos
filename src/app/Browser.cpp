// SPDX-License-Identifier: MS-PL
#include "App.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Catalog.hpp"
#include "CnaDemos/Registry.hpp"
#include "CnaDemos/Runner.hpp"

namespace CnaDemos::App
{
    namespace
    {
        // Segment spellings that title-casing would get wrong. Anything not listed is shown as
        // "Title Case" with underscores turned into spaces.
        const std::map<std::string_view, std::string_view> DisplayNames = {
            {"c_api", "C API"},
            {"cnb", "CNB"},
            {"cnj", "CNJ"},
            {"gltf", "glTF"},
            {"xnb", "XNB"},
            {"xact", "XACT"},
            {"sprite_batch", "SpriteBatch"},
            {"sprite_font", "SpriteFont"},
            {"render_target", "RenderTarget"},
            {"texture2d", "Texture2D"},
            {"texture3d", "Texture3D"},
            {"texture_cube", "TextureCube"},
            {"vector2", "Vector2"},
            {"vector3", "Vector3"},
            {"vector4", "Vector4"},
            {"math_helper", "MathHelper"},
            {"gamepad", "GamePad"},
            {"basic_effect", "BasicEffect"},
            {"gamer_services", "GamerServices"},
            {"ui", "UI"},
            {"cnaext", "CNA extensions"},
        };

        std::string DisplayName(std::string_view segment)
        {
            if (const auto known = DisplayNames.find(segment); known != DisplayNames.end())
            {
                return std::string(known->second);
            }
            std::string name;
            bool startOfWord = true;
            for (const char c : segment)
            {
                if (c == '_')
                {
                    name += ' ';
                    startOfWord = true;
                }
                else
                {
                    name += startOfWord ? static_cast<char>(std::toupper(static_cast<unsigned char>(c))) : c;
                    startOfWord = false;
                }
            }
            return name;
        }

        struct Child
        {
            std::string segment;
            const RegisteredDemo* demo = nullptr;   // set for a demo leaf
            int demoCount = 0;                      // for a group
        };

        // Groups first, then demos, each alphabetically -- the registry is already sorted by ID.
        std::vector<Child> ChildrenOf(const std::string& prefix)
        {
            std::vector<Child> groups;
            std::vector<Child> leaves;
            const std::string start = prefix.empty() ? std::string() : prefix + ".";
            for (const RegisteredDemo& demo : Registry::Instance().All())
            {
                const std::string_view id = demo.entry.info.id;
                if (!id.starts_with(start))
                {
                    continue;
                }
                const std::string_view rest = id.substr(start.size());
                const auto dot = rest.find('.');
                if (dot == std::string_view::npos)
                {
                    leaves.push_back({std::string(rest), &demo, 0});
                    continue;
                }
                const std::string segment(rest.substr(0, dot));
                if (groups.empty() || groups.back().segment != segment)
                {
                    groups.push_back({segment, nullptr, 0});
                }
                ++groups.back().demoCount;
            }
            groups.insert(groups.end(), leaves.begin(), leaves.end());
            return groups;
        }

        std::string Breadcrumb(const std::string& prefix)
        {
            std::string crumb = "CNA Demos";
            std::size_t start = 0;
            while (start < prefix.size())
            {
                const auto dot = prefix.find('.', start);
                crumb += " / " + DisplayName(prefix.substr(start, dot == std::string::npos ? std::string::npos
                                                                                         : dot - start));
                if (dot == std::string::npos)
                {
                    break;
                }
                start = dot + 1;
            }
            return crumb;
        }

        bool ReadLine(std::string& line)
        {
            std::cout << "> " << std::flush;
            if (!std::getline(std::cin, line))
            {
                std::cout << '\n';
                return false;
            }
            const auto first = line.find_first_not_of(" \t\r");
            const auto last = line.find_last_not_of(" \t\r");
            line = first == std::string::npos ? std::string() : line.substr(first, last - first + 1);
            return true;
        }

        std::optional<std::size_t> ParseChoice(const std::string& line, std::size_t count)
        {
            std::size_t value = 0;
            const auto [end, error] = std::from_chars(line.data(), line.data() + line.size(), value);
            if (error != std::errc() || end != line.data() + line.size() || value < 1 || value > count)
            {
                return std::nullopt;
            }
            return value - 1;
        }

        enum class Navigation
        {
            Stay,
            Back,
            Quit
        };

        void PrintHelp()
        {
            std::cout << "  <number>  open a group or demo        b  back      q  quit\n"
                         "  /<text>   search all demos            ?  this help\n"
                         "  <id>      open a demo by its ID (e.g. math.vector2.length)\n";
        }

        Navigation ShowDemo(const RegisteredDemo& demo, const RunOptions& options)
        {
            while (true)
            {
                std::cout << '\n';
                WriteInfo(demo, std::cout);
                std::cout << "\n  r  run      b  back      q  quit\n";
                std::string line;
                if (!ReadLine(line) || line == "q")
                {
                    return Navigation::Quit;
                }
                if (line == "b" || line.empty())
                {
                    return Navigation::Back;
                }
                if (line == "r")
                {
                    if (demo.entry.kind == DemoKind::Game)
                    {
                        std::cout << "Running " << demo.entry.info.id
                                  << " (close the window or press Escape to come back)...\n";
                    }
                    const RunResult result = RunDemo(demo, options, std::cout);
                    std::cout << "== " << ToString(result.status);
                    if (!result.message.empty())
                    {
                        std::cout << ": " << result.message;
                    }
                    std::cout << '\n';
                }
            }
        }

        Navigation ShowList(const std::string& title, const std::vector<const RegisteredDemo*>& demos,
                            const RunOptions& options)
        {
            while (true)
            {
                std::cout << '\n' << title << "\n\n";
                for (std::size_t i = 0; i < demos.size(); ++i)
                {
                    std::cout << "  " << (i + 1) << ". " << demos[i]->entry.info.title << "  ["
                              << demos[i]->entry.info.id << "]\n";
                }
                if (demos.empty())
                {
                    std::cout << "  (no demos match)\n";
                }
                std::string line;
                if (!ReadLine(line) || line == "q")
                {
                    return Navigation::Quit;
                }
                if (line == "b" || line.empty())
                {
                    return Navigation::Back;
                }
                if (const auto choice = ParseChoice(line, demos.size()))
                {
                    if (ShowDemo(*demos[*choice], options) == Navigation::Quit)
                    {
                        return Navigation::Quit;
                    }
                }
            }
        }
    }

    int RunBrowser(const RunOptions& options)
    {
        Registry& registry = Registry::Instance();
        std::cout << "CNA Demo Browser " << CnaDemosVersion() << " -- " << registry.All().size()
                  << " demos (CNA " << CnaVersion() << "). Type ? for help.\n";

        std::string prefix;
        while (true)
        {
            const auto children = ChildrenOf(prefix);
            std::cout << '\n' << Breadcrumb(prefix) << "\n\n";
            for (std::size_t i = 0; i < children.size(); ++i)
            {
                const Child& child = children[i];
                if (child.demo != nullptr)
                {
                    std::cout << "  " << (i + 1) << ". " << child.demo->entry.info.title << "  ["
                              << child.demo->entry.info.id << "]\n";
                }
                else
                {
                    std::cout << "  " << (i + 1) << ". " << DisplayName(child.segment) << "/  ("
                              << child.demoCount << (child.demoCount == 1 ? " demo" : " demos") << ")\n";
                }
            }

            std::string line;
            if (!ReadLine(line) || line == "q")
            {
                return 0;
            }
            if (line.empty())
            {
                continue;
            }
            if (line == "?")
            {
                PrintHelp();
                continue;
            }
            if (line == "b" || line == "..")
            {
                const auto dot = prefix.rfind('.');
                prefix = dot == std::string::npos ? std::string() : prefix.substr(0, dot);
                continue;
            }
            if (line.front() == '/')
            {
                const std::string text = line.substr(1);
                if (ShowList("Search: " + text, registry.Search(text), options) == Navigation::Quit)
                {
                    return 0;
                }
                continue;
            }
            if (const RegisteredDemo* demo = registry.Find(line))
            {
                if (ShowDemo(*demo, options) == Navigation::Quit)
                {
                    return 0;
                }
                continue;
            }
            const auto choice = ParseChoice(line, children.size());
            if (!choice)
            {
                std::cout << "Unknown choice '" << line << "'. Type ? for help.\n";
                continue;
            }
            const Child& child = children[*choice];
            if (child.demo != nullptr)
            {
                if (ShowDemo(*child.demo, options) == Navigation::Quit)
                {
                    return 0;
                }
            }
            else
            {
                prefix = prefix.empty() ? child.segment : prefix + "." + child.segment;
            }
        }
    }
}
