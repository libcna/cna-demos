// SPDX-License-Identifier: MS-PL
// CNA full diagnostics: record one frame containing nested zones and markers, then export it.
#include <algorithm>
#include <sstream>
#include <string>
#include <string_view>

#include "CNA/Diagnostics/Diagnostics.hpp"
#include "CNA/Diagnostics/Instrumentation.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    const CNA::Diagnostics::EventRecord* FindEvent(
        const CNA::Diagnostics::Trace& trace, std::string_view name,
        CNA::Diagnostics::EventKind kind)
    {
        const auto found = std::find_if(trace.GetEvents().begin(), trace.GetEvents().end(),
            [&trace, name, kind](const CNA::Diagnostics::EventRecord& event) {
                return event.kind == kind && trace.ResolveName(event.name) == name;
            });
        return found == trace.GetEvents().end() ? nullptr : &*found;
    }

    const CNA::Diagnostics::MetricSample* FindMetric(
        const CNA::Diagnostics::FrameSample& frame, std::string_view name)
    {
        const auto found = std::find_if(frame.metrics.begin(), frame.metrics.end(),
            [name](const CNA::Diagnostics::MetricSample& metric) { return metric.name == name; });
        return found == frame.metrics.end() ? nullptr : &*found;
    }

    void TraceRecording(CnaDemos::DemoContext& ctx)
    {
        using namespace CNA::Diagnostics;

        if (GetBuildMode() != Mode::Full)
        {
            ctx.Skip("trace recording requires CNA built with CNA_DIAGNOSTICS=FULL");
        }
        ctx.Check(SetRuntimeMode(Mode::Full), "FULL diagnostics are active");

        RecordingSession recording = StartRecording(16);
        ctx.Check(recording.IsActive(), "the bounded recording session started");

        const NameHandle outerName("demo.trace.outer");
        const NameHandle valueMarkerName("demo.trace.value");
        ZoneToken outerToken{};
        {
            FrameScope frame;
            CNA_DIAGNOSTICS_FRAME_COUNTER_ADD("demo.trace.items", 3);
            outerToken = BeginZone(outerName, Category::Update);
            {
                CNA_PROFILE_SCOPE_CATEGORY("demo.trace.inner", Category::Draw);
                MarkEvent(valueMarkerName, Category::Content, 42);
                CNA_DIAGNOSTICS_EVENT_CATEGORY("demo.trace.ready", Category::Application);
            }
            ctx.Check(EndZone(outerToken), "the manual outer zone ended in stack order");
        }

        const Snapshot snapshot = GetProvider().CaptureSnapshot();
        ctx.Check(!snapshot.recentFrames.empty(), "the frame scope published a frame sample");
        const FrameSample* frame = snapshot.recentFrames.empty() ? nullptr : &snapshot.recentFrames.back();
        const MetricSample* items = frame == nullptr ? nullptr : FindMetric(*frame, "demo.trace.items");
        ctx.Check(frame != nullptr && frame->durationNs > 0 && frame->framesPerSecond > 0.0,
                  "the completed frame has a measured duration and rate");
        ctx.Check(items != nullptr && items->kind == MetricKind::FrameCounter && items->value == 3,
                  "the frame retained its three-item counter");

        Trace trace = StopRecording(recording);
        ctx.Check(!recording.IsActive(), "stopping consumes the recording session");
        ctx.Check(trace.GetVersion() == Trace::FormatVersion && trace.GetDroppedEventCount() == 0,
                  "the trace has the current version and lost no events");

        const EventRecord* outer = FindEvent(trace, "demo.trace.outer", EventKind::Zone);
        const EventRecord* inner = FindEvent(trace, "demo.trace.inner", EventKind::Zone);
        const EventRecord* value = FindEvent(trace, "demo.trace.value", EventKind::Marker);
        const EventRecord* ready = FindEvent(trace, "demo.trace.ready", EventKind::Marker);
        const EventRecord* frameEvent = FindEvent(trace, "Frame", EventKind::Frame);
        ctx.Check(trace.GetEvents().size() == 5 && outer != nullptr && inner != nullptr &&
                      value != nullptr && ready != nullptr && frameEvent != nullptr,
                  "the recording contains two zones, two markers and one frame");
        ctx.Check(outer != nullptr && inner != nullptr && outer->category == Category::Update &&
                      inner->category == Category::Draw && inner->parentCorrelationId == outer->correlationId,
                  "the nested zones retain their categories and parent relationship");
        ctx.Check(value != nullptr && value->category == Category::Content && value->value == 42 &&
                      ready != nullptr && ready->category == Category::Application,
                  "direct and macro markers retain their categories and payload");
        ctx.Check(frame != nullptr && frameEvent != nullptr && frameEvent->frameNumber == frame->frameNumber &&
                      frameEvent->durationNs == frame->durationNs && frameEvent->category == Category::Core,
                  "the trace frame matches the snapshot frame");

        std::stringstream binary(std::ios::in | std::ios::out | std::ios::binary);
        const bool wroteBinary = trace.WriteBinary(binary);
        binary.seekg(0);
        const Trace decoded = Trace::ReadBinary(binary);
        ctx.Check(wroteBinary && decoded.GetEvents().size() == trace.GetEvents().size() &&
                      FindEvent(decoded, "demo.trace.value", EventKind::Marker) != nullptr,
                  "the compact binary trace round-trips its events and names");

        std::ostringstream chrome;
        const bool wroteChrome = decoded.WriteChromeTrace(chrome);
        const std::string json = chrome.str();
        ctx.Check(wroteChrome && json.find("\"traceEvents\"") != std::string::npos &&
                      json.find("demo.trace.inner") != std::string::npos &&
                      json.find("\"ph\":\"X\"") != std::string::npos &&
                      json.find("\"ph\":\"i\"") != std::string::npos,
                  "the Chrome export contains duration and instant events");

        ctx.Print("recorded {} events for frame {} and round-tripped {} binary events",
                  trace.GetEvents().size(), frame == nullptr ? 0 : frame->frameNumber,
                  decoded.GetEvents().size());
    }
}

CNA_DEMOS_CONSOLE(TraceRecording, {
    .id = "diagnostics.profiler.trace_recording",
    .title = "Recording a profiled frame",
    .description = "Records one frame with a frame counter, nested manual/macro CPU zones and direct/macro markers, "
                   "correlates the trace with the frame snapshot, and round-trips the bounded trace through CNA's "
                   "binary format and Chrome Trace JSON. Requires a CNA_DIAGNOSTICS=FULL build.",
    .apis = {"CNA::Diagnostics::FrameScope", "CNA_DIAGNOSTICS_FRAME_COUNTER_ADD",
             "CNA::Diagnostics::NameHandle", "CNA::Diagnostics::BeginZone", "CNA::Diagnostics::EndZone",
             "CNA::Diagnostics::ZoneToken", "CNA_PROFILE_SCOPE_CATEGORY", "CNA::Diagnostics::ZoneScope",
             "CNA::Diagnostics::MarkEvent", "CNA_DIAGNOSTICS_EVENT_CATEGORY",
             "CNA::Diagnostics::RecordingSession", "CNA::Diagnostics::StartRecording",
             "CNA::Diagnostics::StopRecording", "CNA::Diagnostics::Trace",
             "CNA::Diagnostics::Trace::WriteBinary", "CNA::Diagnostics::Trace::ReadBinary",
             "CNA::Diagnostics::Trace::WriteChromeTrace", "CNA::Diagnostics::IDiagnosticsProvider::CaptureSnapshot",
             "CNA::Diagnostics::FrameSample", "CNA::Diagnostics::EventRecord"},
    .tags = {"diagnostics", "profiler", "frame", "zone", "marker", "trace", "cnaext"},
    .ownProcess = true,
})
