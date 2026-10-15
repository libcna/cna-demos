// SPDX-License-Identifier: MS-PL
// CNA diagnostics counters and gauges: the game's own numbers in CNA's diagnostics snapshot.
#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

#include "CNA/Diagnostics/Diagnostics.hpp"
#include "CNA/Diagnostics/Instrumentation.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    const CNA::Diagnostics::MetricSample* Find(
        const CNA::Diagnostics::Snapshot& snapshot, std::string_view name)
    {
        const auto found = std::find_if(snapshot.metrics.begin(), snapshot.metrics.end(),
            [name](const CNA::Diagnostics::MetricSample& metric) { return metric.name == name; });
        return found == snapshot.metrics.end() ? nullptr : &*found;
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

        // Long-lived systems can keep explicit handles when they need units, accuracy metadata,
        // signed deltas or an ID for correlation with another diagnostics channel.
        const CNA::Diagnostics::CounterHandle uploaded(
            "demo.uploaded_bytes", CNA::Diagnostics::MetricUnit::Bytes,
            CNA::Diagnostics::Accuracy::Estimated);
        const CNA::Diagnostics::GaugeHandle backlog("demo.pending_uploads");
        uploaded.Add(4'096);
        backlog.Set(9);
        backlog.Add(-2);

        const CNA::Diagnostics::Snapshot snapshot = CNA::Diagnostics::GetProvider().CaptureSnapshot();
        const CNA::Diagnostics::MetricSample* spawned = Find(snapshot, "demo.enemies_spawned");
        const CNA::Diagnostics::MetricSample* alive = Find(snapshot, "demo.enemies_alive");
        const CNA::Diagnostics::MetricSample* uploadedSample = Find(snapshot, "demo.uploaded_bytes");
        const CNA::Diagnostics::MetricSample* backlogSample = Find(snapshot, "demo.pending_uploads");
        ctx.Print("build mode {}: {} metrics in the snapshot; demo.enemies_spawned = {}, demo.enemies_alive = {}",
                  build == Mode::Off ? "OFF" : build == Mode::Stats ? "STATS" : "FULL", snapshot.metrics.size(),
                  spawned ? std::to_string(spawned->value) : std::string("(absent)"),
                  alive ? std::to_string(alive->value) : std::string("(absent)"));
        if (build == Mode::Off)
        {
            ctx.Check(spawned == nullptr && alive == nullptr && uploadedSample == nullptr &&
                          backlogSample == nullptr && uploaded.GetId() == 0 && backlog.GetId() == 0,
                      "with diagnostics OFF macros and explicit handles record nothing");
        }
        else
        {
            ctx.Check(spawned != nullptr && spawned->value == 12,
                      "the macro counter added up three waves of four");
            ctx.Check(alive != nullptr && alive->value == 7,
                      "the macro gauge holds the last value set");
            ctx.Check(uploaded.GetId() != 0 && backlog.GetId() != 0 &&
                          uploaded.GetId() != backlog.GetId() && uploadedSample != nullptr &&
                          uploadedSample->value == 4'096 && uploadedSample->kind == CNA::Diagnostics::MetricKind::Counter &&
                          uploadedSample->unit == CNA::Diagnostics::MetricUnit::Bytes &&
                          uploadedSample->accuracy == CNA::Diagnostics::Accuracy::Estimated,
                      "the explicit counter publishes its ID, value, unit and accuracy");
            ctx.Check(backlogSample != nullptr && backlogSample->value == 7 &&
                          backlogSample->kind == CNA::Diagnostics::MetricKind::Gauge,
                      "the explicit gauge supports Set followed by a signed Add");
        }
    }
}

CNA_DEMOS_CONSOLE(Counters, {
    .id = "diagnostics.profiler.counters",
    .title = "Counters and gauges",
    .description = "Uses counter/gauge macros plus explicit handles with units, accuracy, IDs and signed gauge deltas, "
                   "then reads their metric samples from a snapshot: recorded when CNA is built with STATS or FULL, "
                   "compiled away (but still type-checked) with OFF.",
    .apis = {"CNA_DIAGNOSTICS_COUNTER_ADD", "CNA_DIAGNOSTICS_GAUGE_SET", "CNA::Diagnostics::Snapshot",
             "CNA::Diagnostics::MetricSample", "CNA::Diagnostics::CounterHandle",
             "CNA::Diagnostics::CounterHandle::Add", "CNA::Diagnostics::CounterHandle::GetId",
             "CNA::Diagnostics::GaugeHandle", "CNA::Diagnostics::GaugeHandle::Set",
             "CNA::Diagnostics::GaugeHandle::Add", "CNA::Diagnostics::GaugeHandle::GetId",
             "CNA::Diagnostics::MetricUnit", "CNA::Diagnostics::Accuracy",
             "CNA::Diagnostics::IDiagnosticsProvider::CaptureSnapshot"},
    .tags = {"diagnostics", "profiler", "counter", "gauge", "metrics", "cnaext"},
})
