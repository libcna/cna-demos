// SPDX-License-Identifier: MS-PL
// CNA diagnostics counters and gauges: the game's own numbers in CNA's diagnostics snapshot.
#include <cstdint>
#include <optional>
#include <string>

#include "CNA/Diagnostics/Diagnostics.hpp"
#include "CNA/Diagnostics/Instrumentation.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    std::optional<std::int64_t> Find(const CNA::Diagnostics::Snapshot& snapshot, const std::string& name)
    {
        for (const auto& metric : snapshot.metrics)
        {
            if (metric.name == name)
            {
                return metric.value;
            }
        }
        return std::nullopt;
    }

    void Counters(CnaDemos::DemoContext& ctx)
    {
        using CNA::Diagnostics::Mode;
        const Mode build = CNA::Diagnostics::GetBuildMode();

        // The macros are how game code counts things: free when the build is OFF (the arguments
        // are still compiled, so a mistake shows in every build), recorded with STATS or FULL.
        for (int wave = 0; wave < 3; ++wave)
        {
            CNA_DIAGNOSTICS_COUNTER_ADD("demo.enemies_spawned", 4);
        }
        CNA_DIAGNOSTICS_GAUGE_SET("demo.enemies_alive", 7);

        const CNA::Diagnostics::Snapshot snapshot = CNA::Diagnostics::GetProvider().CaptureSnapshot();
        const auto spawned = Find(snapshot, "demo.enemies_spawned");
        const auto alive = Find(snapshot, "demo.enemies_alive");
        ctx.Print("build mode {}: {} metrics in the snapshot; demo.enemies_spawned = {}, demo.enemies_alive = {}",
                  build == Mode::Off ? "OFF" : build == Mode::Stats ? "STATS" : "FULL", snapshot.metrics.size(),
                  spawned ? std::to_string(*spawned) : std::string("(absent)"),
                  alive ? std::to_string(*alive) : std::string("(absent)"));
        if (build == Mode::Off)
        {
            ctx.Check(!spawned && !alive, "with diagnostics OFF the macros recorded nothing");
        }
        else
        {
            ctx.Check(spawned && *spawned == 12, "the counter added up three waves of four");
            ctx.Check(alive && *alive == 7, "the gauge holds the last value set");
        }
    }
}

CNA_DEMOS_CONSOLE(Counters, {
    .id = "diagnostics.profiler.counters",
    .title = "Counters and gauges",
    .description = "Counts spawned enemies with CNA_DIAGNOSTICS_COUNTER_ADD and sets an alive-enemy gauge with "
                   "CNA_DIAGNOSTICS_GAUGE_SET, then looks for them in the diagnostics snapshot: recorded when CNA is "
                   "built with STATS or FULL, compiled away (but still type-checked) with OFF.",
    .apis = {"CNA_DIAGNOSTICS_COUNTER_ADD", "CNA_DIAGNOSTICS_GAUGE_SET", "CNA::Diagnostics::Snapshot",
             "CNA::Diagnostics::MetricSample", "CNA::Diagnostics::IDiagnosticsProvider::CaptureSnapshot"},
    .tags = {"diagnostics", "profiler", "counter", "gauge", "metrics", "cnaext"},
})
