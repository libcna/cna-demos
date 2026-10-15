// SPDX-License-Identifier: MS-PL
// CNA diagnostics sources: publish non-blocking subsystem statistics at frame boundaries.
#include <algorithm>
#include <memory>
#include <string_view>
#include <utility>

#include "CNA/Diagnostics/Diagnostics.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    const CNA::Diagnostics::MetricSample* FindMetric(
        const CNA::Diagnostics::Snapshot& snapshot, std::string_view name)
    {
        const auto found = std::find_if(snapshot.metrics.begin(), snapshot.metrics.end(),
            [name](const CNA::Diagnostics::MetricSample& metric) { return metric.name == name; });
        return found == snapshot.metrics.end() ? nullptr : &*found;
    }

    const CNA::Diagnostics::MetricSample* FindFrameMetric(
        const CNA::Diagnostics::FrameSample& frame, std::string_view name)
    {
        const auto found = std::find_if(frame.metrics.begin(), frame.metrics.end(),
            [name](const CNA::Diagnostics::MetricSample& metric) { return metric.name == name; });
        return found == frame.metrics.end() ? nullptr : &*found;
    }

    class RendererSource final : public CNA::Diagnostics::IDiagnosticsSource
    {
    public:
        void Collect(CNA::Diagnostics::FrameStatisticsSink& sink) override
        {
            ++calls;
            sink.Set(gpuTime, 1'000 + calls * 100);
            sink.Add(completedQueries, calls);
        }

        int calls = 0;
        CNA::Diagnostics::GaugeHandle gpuTime{
            "demo.source.gpu_time", CNA::Diagnostics::MetricUnit::Nanoseconds,
            CNA::Diagnostics::Accuracy::Estimated};
        CNA::Diagnostics::FrameCounterHandle completedQueries{
            "demo.source.completed_queries", CNA::Diagnostics::MetricUnit::Count,
            CNA::Diagnostics::Accuracy::Exact};
    };

    class ThrowingSource final : public CNA::Diagnostics::IDiagnosticsSource
    {
    public:
        void Collect(CNA::Diagnostics::FrameStatisticsSink&) override
        {
            throw 42;
        }
    };

    void FrameSources(CnaDemos::DemoContext& ctx)
    {
        using namespace CNA::Diagnostics;

        if (GetBuildMode() == Mode::Off)
        {
            ctx.Skip("frame statistics sources require CNA built with CNA_DIAGNOSTICS=STATS or FULL");
        }
        ctx.Check(SetRuntimeMode(Mode::Stats), "STATS diagnostics are active");

        SourceRegistration nullRegistration = RegisterSource({});
        ctx.Check(!nullRegistration.IsRegistered(), "a null diagnostics source returns an empty registration");

        auto source = std::make_shared<RendererSource>();
        std::weak_ptr<RendererSource> weakSource = source;
        ctx.Check(source->gpuTime.GetId() != 0 && source->completedQueries.GetId() != 0 &&
                      source->gpuTime.GetId() != source->completedQueries.GetId(),
                  "the source pre-registers distinct gauge and frame-counter handles");

        SourceRegistration registration = RegisterSource(source);
        SourceRegistration moved(std::move(registration));
        SourceRegistration owner;
        owner = std::move(moved);
        ctx.Check(!registration.IsRegistered() && !moved.IsRegistered() && owner.IsRegistered(),
                  "registration moves transfer one live source registration");

        source.reset();
        std::shared_ptr<RendererSource> retained = weakSource.lock();
        ctx.Check(retained != nullptr && retained->calls == 0,
                  "the registration retains the source without collecting before a frame");

        const Snapshot baseline = GetProvider().CaptureSnapshot();
        BeginFrame();
        EndFrame();
        const Snapshot first = GetProvider().CaptureSnapshot();
        const MetricSample* firstGauge = FindMetric(first, "demo.source.gpu_time");
        const FrameSample* firstFrame = first.recentFrames.empty() ? nullptr : &first.recentFrames.back();
        const MetricSample* firstQueries = firstFrame == nullptr
            ? nullptr : FindFrameMetric(*firstFrame, "demo.source.completed_queries");
        ctx.Check(retained != nullptr && retained->calls == 1 &&
                      first.currentFrameNumber == baseline.currentFrameNumber + 1,
                  "the source is collected exactly once at the first frame boundary");
        ctx.Check(firstGauge != nullptr && firstGauge->value == 1'100 &&
                      firstGauge->kind == MetricKind::Gauge &&
                      firstGauge->unit == MetricUnit::Nanoseconds &&
                      firstGauge->accuracy == Accuracy::Estimated,
                  "the sink publishes the estimated nanosecond gauge with its metadata");
        ctx.Check(firstQueries != nullptr && firstQueries->value == 1 &&
                      firstQueries->kind == MetricKind::FrameCounter &&
                      firstQueries->unit == MetricUnit::Count &&
                      firstQueries->accuracy == Accuracy::Exact,
                  "the sink publishes the exact completed-query count into the frame sample");

        BeginFrame();
        EndFrame();
        const Snapshot second = GetProvider().CaptureSnapshot();
        const MetricSample* secondGauge = FindMetric(second, "demo.source.gpu_time");
        const FrameSample* secondFrame = second.recentFrames.empty() ? nullptr : &second.recentFrames.back();
        const MetricSample* secondQueries = secondFrame == nullptr
            ? nullptr : FindFrameMetric(*secondFrame, "demo.source.completed_queries");
        ctx.Check(retained != nullptr && retained->calls == 2 && secondGauge != nullptr &&
                      secondGauge->value == 1'200 && secondQueries != nullptr && secondQueries->value == 2,
                  "the next frame refreshes the gauge and starts a fresh frame-counter value");
        ctx.Check(firstFrame != nullptr && secondFrame != nullptr &&
                      secondFrame->frameNumber == firstFrame->frameNumber + 1 &&
                      second.currentFrameNumber == baseline.currentFrameNumber + 2,
                  "manual BeginFrame and EndFrame publish consecutive frame numbers");

        retained.reset();
        owner.Reset();
        owner.Reset();
        ctx.Check(!owner.IsRegistered() && weakSource.expired(),
                  "idempotent Reset unregisters and releases the retained source");

        BeginFrame();
        EndFrame();
        const Snapshot unregistered = GetProvider().CaptureSnapshot();
        const MetricSample* staleGauge = FindMetric(unregistered, "demo.source.gpu_time");
        const FrameSample* unregisteredFrame = unregistered.recentFrames.empty()
            ? nullptr : &unregistered.recentFrames.back();
        const MetricSample* zeroQueries = unregisteredFrame == nullptr
            ? nullptr : FindFrameMetric(*unregisteredFrame, "demo.source.completed_queries");
        ctx.Check(staleGauge != nullptr && staleGauge->value == 1'200 &&
                      zeroQueries != nullptr && zeroQueries->value == 0,
                  "after unregistration the gauge stays stale and the frame counter resets to zero");

        const std::uint64_t failuresBefore = unregistered.sourceCollectionFailures;
        SourceRegistration throwing = RegisterSource(std::make_shared<ThrowingSource>());
        const std::uint64_t frameBeforeFailure = unregistered.currentFrameNumber;
        BeginFrame();
        EndFrame();
        const Snapshot isolatedFailure = GetProvider().CaptureSnapshot();
        ctx.Check(throwing.IsRegistered() &&
                      isolatedFailure.sourceCollectionFailures == failuresBefore + 1 &&
                      isolatedFailure.currentFrameNumber == frameBeforeFailure + 1,
                  "a throwing source is counted while the enclosing frame still completes");
        throwing.Reset();

        ctx.Print("collected two source frames, then isolated one failing source callback");
    }
}

CNA_DEMOS_CONSOLE(FrameSources, {
    .id = "diagnostics.profiler.frame_sources",
    .title = "Frame statistics sources",
    .description = "Registers a retained subsystem source, publishes an estimated gauge and exact frame counter only "
                   "at manual frame boundaries, verifies per-frame reset and RAII moves/unregistration, and shows that "
                   "a failing source is counted without preventing frame completion.",
    .apis = {"CNA::Diagnostics::IDiagnosticsSource", "CNA::Diagnostics::FrameStatisticsSink",
             "CNA::Diagnostics::FrameStatisticsSink::Set", "CNA::Diagnostics::FrameStatisticsSink::Add",
             "CNA::Diagnostics::SourceRegistration", "CNA::Diagnostics::SourceRegistration::IsRegistered",
             "CNA::Diagnostics::SourceRegistration::Reset", "CNA::Diagnostics::RegisterSource",
             "CNA::Diagnostics::GaugeHandle", "CNA::Diagnostics::GaugeHandle::GetId",
             "CNA::Diagnostics::FrameCounterHandle", "CNA::Diagnostics::FrameCounterHandle::GetId",
             "CNA::Diagnostics::BeginFrame", "CNA::Diagnostics::EndFrame",
             "CNA::Diagnostics::MetricUnit", "CNA::Diagnostics::Accuracy",
             "CNA::Diagnostics::Snapshot::sourceCollectionFailures"},
    .tags = {"diagnostics", "profiler", "source", "frame", "counter", "gauge", "cnaext"},
    .ownProcess = true,
})
