// SPDX-License-Identifier: MS-PL
#pragma once

// The one header a demo includes. It provides the metadata types, DemoContext, DemoGame and
// the three registration macros. See docs/DEMO_AUTHORING.md.

#include <memory>

#include "CnaDemos/DemoContext.hpp"
#include "CnaDemos/DemoGame.hpp"
#include "CnaDemos/Metadata.hpp"
#include "CnaDemos/Registry.hpp"

#define CNA_DEMOS_CONCAT_IMPL(a, b) a##b
#define CNA_DEMOS_CONCAT(a, b) CNA_DEMOS_CONCAT_IMPL(a, b)

/**
 * Registers a Game demo:
 *
 *     class SpriteRotation final : public CnaDemos::DemoGame { ... };
 *     CNA_DEMOS_GAME(SpriteRotation, { .id = "...", .title = "...", ... })
 *
 * The second argument is the brace-enclosed designated initializer of a CnaDemos::DemoInfo.
 */
#define CNA_DEMOS_GAME(GameClass, ...)                                                          \
    namespace                                                                                   \
    {                                                                                           \
        const ::CnaDemos::Detail::Registrar CNA_DEMOS_CONCAT(cnaDemosRegistrar_, __LINE__){     \
            ::CnaDemos::DemoInfo __VA_ARGS__, ::CnaDemos::DemoKind::Game, __FILE__, nullptr,   \
            []() -> std::unique_ptr<::CnaDemos::DemoGame> { return std::make_unique<GameClass>(); }}; \
    }

/**
 * Registers a console demo -- a function `void Name(CnaDemos::DemoContext& ctx)` that runs
 * in-process without a Game or a window:
 *
 *     void Vector2Length(CnaDemos::DemoContext& ctx) { ... }
 *     CNA_DEMOS_CONSOLE(Vector2Length, { .id = "...", ... })
 */
#define CNA_DEMOS_CONSOLE(Function, ...)                                                        \
    namespace                                                                                   \
    {                                                                                           \
        const ::CnaDemos::Detail::Registrar CNA_DEMOS_CONCAT(cnaDemosRegistrar_, __LINE__){     \
            ::CnaDemos::DemoInfo __VA_ARGS__, ::CnaDemos::DemoKind::Console, __FILE__,         \
            &Function, nullptr};                                                                \
    }

/**
 * Registers the metadata of a standalone demo. Used only in standalone/<id>/metadata.cpp;
 * the program itself is standalone/<id>/main.cpp, built as its own executable.
 */
#define CNA_DEMOS_STANDALONE(...)                                                               \
    namespace                                                                                   \
    {                                                                                           \
        const ::CnaDemos::Detail::Registrar CNA_DEMOS_CONCAT(cnaDemosRegistrar_, __LINE__){     \
            ::CnaDemos::DemoInfo __VA_ARGS__, ::CnaDemos::DemoKind::Standalone, __FILE__,      \
            nullptr, nullptr};                                                                  \
    }
