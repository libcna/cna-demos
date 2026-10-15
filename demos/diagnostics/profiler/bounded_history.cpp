// SPDX-License-Identifier: MS-PL
// CNA diagnostics bounds: observable history overwrite, producer drops and malformed-use counts.
#include <algorithm>
#include <cstdint>
#include <utility>

#include "CNA/Diagnostics/Diagnostics.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    void BoundedHistory(CnaDemos::DemoContext& ctx)
    {
        using namespace CNA::Diagnostics;

        if (GetBuildMode() != Mode::Full)
        {
            ctx.Skip("bounded event history requires CNA built with CNA_DIAGNOSTICS=FULL");
        }
        ctx.Check(SetRuntimeMode(Mode::Full), "FULL diagnostics are active");

        IDiagnosticsProvider& provider = GetProvider();
        const NameHandle seedName("demo.bounds.seed");
        const NameHandle historyName("demo.bounds.history");
        ctx.Check(seedName.GetId() != 0 && historyName.GetId() != 0 &&
                      seedName.GetId() != historyName.GetId(),
                  "FULL name handles receive distinct nonzero IDs");

        // Establish a nonzero cursor, then add three more events than the whole process history can
        // retain. Draining every half thread-buffer keeps this test about history overwrite rather
        // than producer overflow.
        MarkEvent(seedName);
        const EventBatch beforeHistory = provider.ReadEvents(0, EventHistoryCapacity);
        const Snapshot beforeHistorySnapshot = provider.CaptureSnapshot();
        const std::uint64_t cursor = beforeHistory.newestAvailableSequence;
        for (std::size_t index = 0; index < EventHistoryCapacity + 3; ++index)
        {
            MarkEvent(historyName, Category::Application, static_cast<std::int64_t>(index));
            if ((index + 1) % (ThreadEventCapacity / 2) == 0)
                (void)provider.ReadEvents(0, 0);
        }
        const EventBatch retained = provider.ReadEvents(cursor, 4);
        const Snapshot afterHistory = provider.CaptureSnapshot();
        ctx.Check(retained.eventsDroppedBeforeStart == 3 && retained.events.size() == 4 &&
                      retained.oldestAvailableSequence == cursor + 4 &&
                      retained.newestAvailableSequence == cursor + EventHistoryCapacity + 3,
                  "a stale cursor reports three overwritten events and the exact retained bounds");
        ctx.Check(retained.events[0].value == 3 && retained.events[1].value == 4 &&
                      retained.events[2].value == 5 && retained.events[3].value == 6 &&
                      std::all_of(retained.events.begin(), retained.events.end(),
                          [&historyName](const EventRecord& event) {
                              return event.kind == EventKind::Marker && event.name == historyName.GetId();
                          }),
                  "the bounded read starts at the first retained marker and respects its maximum");
        ctx.Check(afterHistory.eventHistoryOverwrites ==
                      beforeHistorySnapshot.eventHistoryOverwrites + beforeHistory.events.size() + 3,
                  "the snapshot counts every process-history overwrite");

        // Filling one producer buffer without a consumer retains its capacity and drops the excess
        // seven markers. Accepted events still form a contiguous provider sequence.
        const std::uint64_t producerCursor = retained.newestAvailableSequence;
        const std::uint64_t producerDropsBefore = afterHistory.producerEventsDropped;
        for (std::size_t index = 0; index < ThreadEventCapacity + 7; ++index)
            MarkEvent(historyName, Category::Application, static_cast<std::int64_t>(index));
        const EventBatch producerBatch = provider.ReadEvents(producerCursor, EventHistoryCapacity);
        ctx.Check(producerBatch.events.size() == ThreadEventCapacity &&
                      producerBatch.events.front().value == 0 &&
                      producerBatch.events.back().value == static_cast<std::int64_t>(ThreadEventCapacity - 1) &&
                      producerBatch.producerEventsDropped == producerDropsBefore + 7,
                  "a full producer buffer retains its capacity and reports seven dropped events");

        RecordingSession zeroCapacity = StartRecording(0);
        ctx.Check(!zeroCapacity.IsActive(), "a zero-capacity recording is inactive");
        RecordingSession original = StartRecording(3);
        RecordingSession recording(std::move(original));
        ctx.Check(!original.IsActive() && recording.IsActive(),
                  "moving a recording session transfers its only active cursor");
        const NameHandle recordingName("demo.bounds.recording");
        for (std::int64_t value = 0; value < 5; ++value)
            MarkEvent(recordingName, Category::Application, value);
        const Trace bounded = StopRecording(recording);
        ctx.Check(!recording.IsActive() && bounded.GetEvents().size() == 3 &&
                      bounded.GetDroppedEventCount() == 2 &&
                      bounded.GetEvents().front().value == 2 && bounded.GetEvents().back().value == 4,
                  "a three-event recording retains the newest three of five and reports two drops");

        const Snapshot beforeMalformedZone = provider.CaptureSnapshot();
        RecordingSession malformedRecording = StartRecording(4);
        const NameHandle zoneName("demo.bounds.manual_zone");
        const ZoneToken zone = BeginZone(zoneName, Category::Update);
        const bool firstEnd = EndZone(zone);
        const bool repeatedEnd = EndZone(zone);
        const Trace malformedTrace = StopRecording(malformedRecording);
        const Snapshot afterMalformedZone = provider.CaptureSnapshot();
        const bool hasMalformed = std::any_of(malformedTrace.GetEvents().begin(), malformedTrace.GetEvents().end(),
            [](const EventRecord& event) { return event.kind == EventKind::Malformed; });
        const bool hasZone = std::any_of(malformedTrace.GetEvents().begin(), malformedTrace.GetEvents().end(),
            [&zoneName](const EventRecord& event) {
                return event.kind == EventKind::Zone && event.name == zoneName.GetId();
            });
        ctx.Check(firstEnd && !repeatedEnd && hasZone && hasMalformed &&
                      afterMalformedZone.malformedZoneCount == beforeMalformedZone.malformedZoneCount + 1,
                  "re-ending a manual zone is refused, counted and emitted without losing the valid zone");

        const std::uint64_t malformedFramesBefore = afterMalformedZone.malformedFrameCount;
        const std::uint64_t frameBefore = afterMalformedZone.currentFrameNumber;
        BeginFrame();
        BeginFrame();
        EndFrame();
        EndFrame();
        const Snapshot afterMalformedFrame = provider.CaptureSnapshot();
        ctx.Check(afterMalformedFrame.malformedFrameCount == malformedFramesBefore + 2 &&
                      afterMalformedFrame.currentFrameNumber == frameBefore + 1,
                  "duplicate begin/end calls report two malformed transitions while one frame completes");

        const std::uint64_t boundedFramesBefore = afterMalformedFrame.currentFrameNumber;
        for (std::size_t index = 0; index < FrameHistoryCapacity + 2; ++index)
        {
            FrameScope frame;
        }
        const Snapshot boundedFrames = provider.CaptureSnapshot();
        ctx.Check(boundedFrames.recentFrames.size() == FrameHistoryCapacity &&
                      boundedFrames.recentFrames.front().frameNumber == boundedFramesBefore + 3 &&
                      boundedFrames.recentFrames.back().frameNumber ==
                          boundedFramesBefore + FrameHistoryCapacity + 2,
                  "frame history retains the newest documented capacity in increasing order");

        ctx.Print("retained {} history events, dropped 7 producer events and counted malformed zone/frame use",
                  EventHistoryCapacity);
    }
}

CNA_DEMOS_CONSOLE(BoundedHistory, {
    .id = "diagnostics.profiler.bounded_history",
    .title = "Bounded history and misuse reporting",
    .description = "Overfills CNA's documented process history and one producer buffer, follows the provider cursor "
                   "and drop counters, bounds a moved recording session, and demonstrates contained malformed "
                   "manual-zone and frame transitions. Requires a CNA_DIAGNOSTICS=FULL build.",
    .apis = {"CNA::Diagnostics::ThreadEventCapacity", "CNA::Diagnostics::EventHistoryCapacity",
             "CNA::Diagnostics::FrameHistoryCapacity",
             "CNA::Diagnostics::NameHandle::GetId", "CNA::Diagnostics::EventBatch::oldestAvailableSequence",
             "CNA::Diagnostics::EventBatch::newestAvailableSequence",
             "CNA::Diagnostics::EventBatch::eventsDroppedBeforeStart",
             "CNA::Diagnostics::EventBatch::producerEventsDropped",
             "CNA::Diagnostics::Snapshot::producerEventsDropped",
             "CNA::Diagnostics::Snapshot::eventHistoryOverwrites",
             "CNA::Diagnostics::Snapshot::malformedZoneCount",
             "CNA::Diagnostics::Snapshot::malformedFrameCount",
             "CNA::Diagnostics::RecordingSession::IsActive",
             "CNA::Diagnostics::BeginFrame", "CNA::Diagnostics::EndFrame"},
    .tags = {"diagnostics", "profiler", "history", "overflow", "recording", "validation", "cnaext"},
    .ownProcess = true,
})
