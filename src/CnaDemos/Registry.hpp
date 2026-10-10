// SPDX-License-Identifier: MS-PL
#pragma once

#include <functional>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "CnaDemos/Metadata.hpp"

namespace CnaDemos
{
    class DemoContext;
    class DemoGame;

    /** Body of a console demo. */
    using ConsoleDemoFunction = void (*)(DemoContext&);
    /** Creates the Game of a Game demo. Called with DemoContext::Current() already installed. */
    using GameDemoFactory = std::unique_ptr<DemoGame> (*)();

    /** A registered demo together with how to run it. */
    struct RegisteredDemo
    {
        DemoEntry entry;
        ConsoleDemoFunction console = nullptr;
        GameDemoFactory game = nullptr;
    };

    /**
     * The process-wide demo registry. Demos add themselves during static initialisation
     * through the CNA_DEMOS_* macros; nothing reads the registry before main() starts.
     * Registration order is unspecified, so every query returns demos sorted by ID.
     */
    class Registry
    {
    public:
        [[nodiscard]] static Registry& Instance();

        void Add(RegisteredDemo demo);

        /** All demos, sorted by ID. */
        [[nodiscard]] std::span<const RegisteredDemo> All();

        /** The demo with exactly this ID, or nullptr. */
        [[nodiscard]] const RegisteredDemo* Find(std::string_view id);

        /** Demos whose ID equals `prefix` or starts with `prefix` + '.'. */
        [[nodiscard]] std::vector<const RegisteredDemo*> WithPrefix(std::string_view prefix);

        /** Demos carrying `tag` (exact, case-sensitive). */
        [[nodiscard]] std::vector<const RegisteredDemo*> WithTag(std::string_view tag);

        /**
         * Case-insensitive substring search over ID, title, description, tags and APIs.
         * Every whitespace-separated word of `text` must match somewhere.
         */
        [[nodiscard]] std::vector<const RegisteredDemo*> Search(std::string_view text);

        /** Distinct categories (first ID segment), sorted. */
        [[nodiscard]] std::vector<std::string_view> Categories();

    private:
        void SortIfNeeded();

        std::vector<RegisteredDemo> demos_;
        bool sorted_ = true;
    };

    namespace Detail
    {
        /** Turns __FILE__ into a repository-relative path ("demos/math/vector2/basics.cpp"). */
        [[nodiscard]] std::string RepositoryRelativePath(std::string_view file);

        struct Registrar
        {
            Registrar(DemoInfo info, DemoKind kind, std::string_view file,
                      ConsoleDemoFunction console, GameDemoFactory game);
        };
    }
}
