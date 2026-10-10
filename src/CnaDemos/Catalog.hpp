// SPDX-License-Identifier: MS-PL
#pragma once

#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

#include "CnaDemos/Registry.hpp"

namespace CnaDemos
{
    /** Version strings compiled into the executable. */
    [[nodiscard]] std::string_view CnaDemosVersion();
    [[nodiscard]] std::string_view CnaVersion();

    /** Names of the renderers compiled into this build, default first. */
    [[nodiscard]] std::vector<std::string> CompiledRenderers();

    /** The whole catalog as JSON (schema documented in docs/ARCHITECTURE.md). */
    void WriteCatalogJson(std::ostream& out);

    /** Escapes a string for a JSON string literal (without the surrounding quotes). */
    [[nodiscard]] std::string JsonEscape(std::string_view text);

    /** Human-readable multi-line description of one demo (--info and the browser). */
    void WriteInfo(const RegisteredDemo& demo, std::ostream& out);

    /**
     * Validates the registry: ID syntax and uniqueness, ID/source-path agreement, no demo ID
     * that is also a group of other demos, required metadata present, referenced assets
     * present under `assetRoot`. Returns the problems found (empty = valid).
     */
    [[nodiscard]] std::vector<std::string> ValidateRegistry(const std::string& assetRoot);
}
