// SPDX-License-Identifier: MS-PL
// CNA diagnostics modes: what the build compiled in (OFF, STATS, FULL) and what runs now.
#include <string>

#include "CNA/Diagnostics/Diagnostics.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    const char* Name(CNA::Diagnostics::Mode mode)
    {
        switch (mode)
        {
            case CNA::Diagnostics::Mode::Off: return "OFF";
            case CNA::Diagnostics::Mode::Stats: return "STATS";
            case CNA::Diagnostics::Mode::Full: return "FULL";
        }
        return "?";
    }

    void Modes(CnaDemos::DemoContext& ctx)
    {
        using CNA::Diagnostics::Mode;
        // The build mode is fixed when CNA is configured (CMake option CNA_DIAGNOSTICS); with
        // OFF the instrumentation macros compile to nothing at all.
        const Mode build = CNA::Diagnostics::GetBuildMode();
        const Mode runtime = CNA::Diagnostics::GetRuntimeMode();
        ctx.Print("build mode {}, runtime mode {}", Name(build), Name(runtime));
        ctx.Check(static_cast<int>(runtime) <= static_cast<int>(build), "the runtime mode never exceeds the build mode");

        // At run time a program may lower the mode (or raise it back up to the build's).
        const bool statsAccepted = CNA::Diagnostics::SetRuntimeMode(Mode::Stats);
        ctx.Check(statsAccepted == (build != Mode::Off), "STATS is accepted exactly when the build has it");
        const bool fullAccepted = CNA::Diagnostics::SetRuntimeMode(Mode::Full);
        ctx.Check(fullAccepted == (build == Mode::Full), "FULL is accepted exactly when the build has it");
        ctx.Check(CNA::Diagnostics::SetRuntimeMode(Mode::Off), "OFF is always accepted");

        const CNA::Diagnostics::Snapshot snapshot = CNA::Diagnostics::GetProvider().CaptureSnapshot();
        ctx.Check(snapshot.buildMode == build && snapshot.runtimeMode == Mode::Off,
                  "a snapshot reports both modes");

        // Diagnostics are process-wide: leave the mode as it was.
        ctx.Check(CNA::Diagnostics::SetRuntimeMode(runtime), "the previous runtime mode is restored");
    }
}

CNA_DEMOS_CONSOLE(Modes, {
    .id = "diagnostics.profiler.modes",
    .title = "Diagnostics modes",
    .description = "Reads CNA's diagnostics build mode (OFF, STATS or FULL, chosen with the CNA_DIAGNOSTICS CMake "
                   "option) and runtime mode, shows which runtime modes SetRuntimeMode accepts for this build, and "
                   "reads both from a provider snapshot.",
    .apis = {"CNA::Diagnostics::GetBuildMode", "CNA::Diagnostics::GetRuntimeMode", "CNA::Diagnostics::SetRuntimeMode",
             "CNA::Diagnostics::GetProvider", "CNA::Diagnostics::IDiagnosticsProvider::CaptureSnapshot",
             "CNA::Diagnostics::Snapshot"},
    .tags = {"diagnostics", "profiler", "instrumentation", "performance", "cnaext"},
})
