// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Registry.hpp"

#include <algorithm>
#include <cctype>
#include <set>
#include <string>

namespace CnaDemos
{
    namespace
    {
        bool StartsWith(std::string_view text, std::string_view prefix)
        {
            return text.size() >= prefix.size() && text.substr(0, prefix.size()) == prefix;
        }

        std::string Lower(std::string_view text)
        {
            std::string result(text);
            std::ranges::transform(result, result.begin(),
                                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return result;
        }

        bool ContainsIgnoringCase(std::string_view haystack, const std::string& lowerNeedle)
        {
            return Lower(haystack).find(lowerNeedle) != std::string::npos;
        }

        bool MatchesWord(const RegisteredDemo& demo, const std::string& word)
        {
            const DemoInfo& info = demo.entry.info;
            if (ContainsIgnoringCase(info.id, word) || ContainsIgnoringCase(info.title, word)
                || ContainsIgnoringCase(info.description, word))
            {
                return true;
            }
            const auto anyMatch = [&word](const std::vector<std::string_view>& values) {
                return std::ranges::any_of(values, [&word](std::string_view value) {
                    return ContainsIgnoringCase(value, word);
                });
            };
            return anyMatch(info.tags) || anyMatch(info.apis);
        }
    }

    Registry& Registry::Instance()
    {
        static Registry registry;
        return registry;
    }

    void Registry::Add(RegisteredDemo demo)
    {
        demos_.push_back(std::move(demo));
        sorted_ = false;
    }

    void Registry::SortIfNeeded()
    {
        if (!sorted_)
        {
            // stable_sort keeps duplicate IDs adjacent and in a deterministic order, so the
            // validator can report them instead of one silently shadowing the other.
            std::ranges::stable_sort(demos_, {}, [](const RegisteredDemo& d) { return d.entry.info.id; });
            sorted_ = true;
        }
    }

    std::span<const RegisteredDemo> Registry::All()
    {
        SortIfNeeded();
        return demos_;
    }

    const RegisteredDemo* Registry::Find(std::string_view id)
    {
        SortIfNeeded();
        const auto it = std::ranges::lower_bound(demos_, id, {},
                                                 [](const RegisteredDemo& d) { return d.entry.info.id; });
        return it != demos_.end() && it->entry.info.id == id ? &*it : nullptr;
    }

    std::vector<const RegisteredDemo*> Registry::WithPrefix(std::string_view prefix)
    {
        SortIfNeeded();
        std::vector<const RegisteredDemo*> result;
        for (const auto& demo : demos_)
        {
            const std::string_view id = demo.entry.info.id;
            if (id == prefix || (StartsWith(id, prefix) && id.size() > prefix.size() && id[prefix.size()] == '.'))
            {
                result.push_back(&demo);
            }
        }
        return result;
    }

    std::vector<const RegisteredDemo*> Registry::WithTag(std::string_view tag)
    {
        SortIfNeeded();
        std::vector<const RegisteredDemo*> result;
        for (const auto& demo : demos_)
        {
            if (std::ranges::find(demo.entry.info.tags, tag) != demo.entry.info.tags.end())
            {
                result.push_back(&demo);
            }
        }
        return result;
    }

    std::vector<const RegisteredDemo*> Registry::Search(std::string_view text)
    {
        SortIfNeeded();
        std::vector<std::string> words;
        std::string current;
        for (const char c : text)
        {
            if (std::isspace(static_cast<unsigned char>(c)))
            {
                if (!current.empty())
                {
                    words.push_back(Lower(current));
                    current.clear();
                }
            }
            else
            {
                current.push_back(c);
            }
        }
        if (!current.empty())
        {
            words.push_back(Lower(current));
        }

        std::vector<const RegisteredDemo*> result;
        for (const auto& demo : demos_)
        {
            if (std::ranges::all_of(words, [&demo](const std::string& w) { return MatchesWord(demo, w); }))
            {
                result.push_back(&demo);
            }
        }
        return result;
    }

    std::vector<std::string_view> Registry::Categories()
    {
        std::set<std::string_view> categories;
        for (const auto& demo : All())
        {
            categories.insert(demo.entry.Category());
        }
        return {categories.begin(), categories.end()};
    }

    namespace Detail
    {
        std::string RepositoryRelativePath(std::string_view file)
        {
            std::string path(file);
            std::ranges::replace(path, '\\', '/');
#ifdef CNA_DEMOS_SOURCE_DIR
            std::string root = CNA_DEMOS_SOURCE_DIR;
            std::ranges::replace(root, '\\', '/');
            if (!root.empty() && root.back() != '/')
            {
                root.push_back('/');
            }
            if (StartsWith(path, root))
            {
                return path.substr(root.size());
            }
#endif
            // __FILE__ was not under the configured source directory (a symlinked checkout, a
            // relative compiler path): fall back to the first demo tree in the path.
            for (const std::string_view tree : {"/demos/", "/standalone/"})
            {
                const auto position = path.find(tree);
                if (position != std::string::npos)
                {
                    return path.substr(position + 1);
                }
            }
            return path;
        }

        Registrar::Registrar(DemoInfo info, DemoKind kind, std::string_view file,
                             ConsoleDemoFunction console, GameDemoFactory game)
        {
            RegisteredDemo demo;
            demo.entry.info = std::move(info);
            demo.entry.kind = kind;
            demo.entry.source = RepositoryRelativePath(file);
            demo.console = console;
            demo.game = game;
            Registry::Instance().Add(std::move(demo));
        }
    }
}
