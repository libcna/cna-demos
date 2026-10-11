// SPDX-License-Identifier: MS-PL
#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace CnaDemos
{
    /** How a demo is executed. Chosen by the registration macro, never written by hand. */
    enum class DemoKind
    {
        Console,    ///< A function run in-process; no Game, no window (CNA_DEMOS_CONSOLE).
        Game,       ///< A DemoGame subclass run in-process (CNA_DEMOS_GAME).
        Standalone  ///< A separate executable under standalone/<id>/ (CNA_DEMOS_STANDALONE).
    };

    /** What an unattended run of a demo proves. */
    enum class Automation
    {
        /** An unattended run exercises the feature and its checks decide pass/fail. */
        Automated,
        /**
         * An unattended run only proves the demo starts, runs a bounded number of frames and
         * shuts down cleanly. The feature itself needs a person (keyboard, mouse, gamepad, ...).
         */
        SmokeOnly,
        /**
         * Not run unattended at all: it needs hardware, a service, credentials or a person
         * from the first frame. Batch runs list it as "manual" instead of running it.
         */
        Manual
    };

    /**
     * Environment requirements, as bit flags. A demo whose needs the current environment
     * cannot meet is reported as SKIPPED (exit code 77) with the reason -- never as passed.
     */
    enum class Need : std::uint32_t
    {
        None = 0,
        ThreeD = 1u << 0,          ///< GraphicsCapability::ThreeD (vertex buffers, effects, depth).
        CustomEffects = 1u << 1,   ///< GraphicsCapability::CustomEffects.
        Audio = 1u << 3,          ///< A working audio output device.
        Keyboard = 1u << 4,        ///< Meaningful only with a person at a keyboard.
        Mouse = 1u << 5,
        GamePad = 1u << 6,
        Touch = 1u << 7,
        Microphone = 1u << 8,
        Network = 1u << 9,         ///< Opens sockets (loopback counts).
        Window = 1u << 10,         ///< Needs a real window, not a windowless renderer.
        Video = 1u << 11,          ///< CNA built with video decoding (CNA_ENABLE_VIDEO).
        Instancing = 1u << 12,     ///< GraphicsCapability::Instancing.
        MultipleRenderTargets = 1u << 13,
        Texture3D = 1u << 14,
        OcclusionQuery = 1u << 15,
        StencilBuffer = 1u << 16,
        WireFrame = 1u << 17,
        MultiSampling = 1u << 18,
        CompiledEffects = 1u << 19,
        FloatRenderTargets = 1u << 20,
        AnisotropicFiltering = 1u << 21,
        IndirectDraw = 1u << 22,
        ComputeShaders = 1u << 23,
    };

    [[nodiscard]] constexpr Need operator|(Need a, Need b)
    {
        return static_cast<Need>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
    }

    [[nodiscard]] constexpr bool HasNeed(Need set, Need flag)
    {
        return (static_cast<std::uint32_t>(set) & static_cast<std::uint32_t>(flag)) != 0;
    }

    /** Every Need flag with its stable textual name (used by --info, the catalog and filters). */
    struct NeedName
    {
        Need need;
        std::string_view name;
    };
    [[nodiscard]] const std::vector<NeedName>& AllNeeds();
    [[nodiscard]] std::vector<std::string_view> NeedNames(Need needs);

    [[nodiscard]] std::string_view ToString(DemoKind kind);
    [[nodiscard]] std::string_view ToString(Automation automation);

    /**
     * Metadata a demo author writes, with designated initializers, in the registration macro:
     *
     *     CNA_DEMOS_GAME(SpriteRotation, {
     *         .id = "graphics.sprite_batch.rotation",
     *         .title = "Sprite rotation",
     *         .description = "Rotates a sprite about its centre with SpriteBatch::Draw.",
     *         .apis = {"SpriteBatch::Draw"},
     *     })
     *
     * Category, subcategory, kind and source file are derived, never written: the category is
     * the first ID segment, the subcategory the segments between it and the last one.
     */
    struct DemoInfo
    {
        /** Stable machine-readable identity: lower_snake segments joined by '.'. */
        std::string_view id;
        /** Short human title (sentence case, no trailing period). */
        std::string_view title;
        /** One or two sentences saying what the demo shows. Ends with a period. */
        std::string_view description;
        /** CNA API members the demo is about, as Type::Member (XNA member names). */
        std::vector<std::string_view> apis;
        /** Extra search keywords (lower case). */
        std::vector<std::string_view> tags = {};
        Automation automation = Automation::Automated;
        Need needs = Need::None;
        /**
         * Frames a Game demo draws in an unattended run before it exits. 0 means the
         * framework default (DefaultAutomatedFrames). Ignored for console demos.
         */
        int frames = 0;
        /**
         * True when the captured frame is identical on every run and machine: a candidate for
         * screenshot regression. Static scenes qualify; animation driven by GameTime does not,
         * because the fixed-step loop runs extra catch-up Updates whenever a frame is late, so
         * the game time reached at frame N depends on machine speed.
         */
        bool deterministic = false;
        /** Files the demo loads, relative to assets/. Validated to exist. */
        std::vector<std::string_view> assets = {};
        /** Renderer/platform restrictions and the reason for them, if any. */
        std::string_view restrictions = {};
        /**
         * ID of an entry in docs/CNA_ISSUES.md ("CNA-DEMOS-ISSUE-NNN") when the demo is known to fail
         * because of a recorded CNA defect. The demo still checks the documented behaviour and
         * still reports FAILED; batch runs and CTest treat that failure as expected, and a PASS as
         * a signal that the issue may be fixed (see docs/AUTOMATION.md).
         */
        std::string_view knownIssue = {};
        /**
         * True when the demo sets up state that CNA allows only once per process -- gamer
         * services (GamerServicesComponent initializes GamerServicesDispatcher, which XNA and
         * CNA refuse to initialize twice). Batch runs and the browser start such a demo in a
         * fresh cna-demos process; `cna-demos <id>` already is one and runs it directly.
         */
        bool ownProcess = false;
    };

    /** A registered demo: the author's metadata plus what the registry derived. */
    struct DemoEntry
    {
        DemoInfo info;
        DemoKind kind = DemoKind::Console;
        /** Path of the registering source file relative to the repository root. */
        std::string source;

        [[nodiscard]] std::string_view Category() const;
        /** Middle segments joined by '.', empty for a two-segment ID. */
        [[nodiscard]] std::string_view Subcategory() const;
        /** Name of the separate executable of a standalone demo ("" otherwise). */
        [[nodiscard]] std::string Executable() const;
    };

    /** Frames an unattended Game demo draws when its metadata leaves frames at 0. */
    inline constexpr int DefaultAutomatedFrames = 60;

    /** "cna-demo-" + id with '.' replaced by '-' (mirrored by cmake/CnaDemosCategories.cmake). */
    [[nodiscard]] std::string StandaloneExecutableName(std::string_view id);
}
