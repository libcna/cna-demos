// SPDX-License-Identifier: MS-PL
// CNA diagnostics resources: owned metadata, stable handle identity and FULL lifecycle events.
#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

#include "CNA/Diagnostics/Diagnostics.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    const CNA::Diagnostics::ResourceRecord* FindResource(
        const CNA::Diagnostics::Snapshot& snapshot, CNA::Diagnostics::ResourceId id)
    {
        const auto found = std::find_if(snapshot.resources.begin(), snapshot.resources.end(),
            [id](const CNA::Diagnostics::ResourceRecord& resource) { return resource.id == id; });
        return found == snapshot.resources.end() ? nullptr : &*found;
    }

    const CNA::Diagnostics::EventRecord* FindResourceEvent(
        const CNA::Diagnostics::EventBatch& batch, CNA::Diagnostics::ResourceId id,
        CNA::Diagnostics::EventKind kind)
    {
        const auto found = std::find_if(batch.events.begin(), batch.events.end(),
            [id, kind](const CNA::Diagnostics::EventRecord& event) {
                return event.kind == kind && event.value == static_cast<std::int64_t>(id);
            });
        return found == batch.events.end() ? nullptr : &*found;
    }

    void ResourceMetadata(CnaDemos::DemoContext& ctx)
    {
        using namespace CNA::Diagnostics;

        if (GetBuildMode() != Mode::Full)
        {
            ctx.Skip("resource lifecycle events require CNA built with CNA_DIAGNOSTICS=FULL");
        }
        ctx.Check(SetRuntimeMode(Mode::Full), "FULL diagnostics are active");

        IDiagnosticsProvider& provider = GetProvider();
        const Snapshot baseline = provider.CaptureSnapshot();
        const EventBatch priorEvents = provider.ReadEvents(0, EventHistoryCapacity);
        const std::uint64_t eventCursor = priorEvents.newestAvailableSequence;
        RecordingSession recording = StartRecording(8);

        std::string sourceLabel = "demo albedo";
        std::string sourceFormat = "Color";
        ResourceHandle original(ResourceDescriptor{
            .kind = ResourceKind::Texture2D,
            .label = sourceLabel,
            .format = sourceFormat,
            .width = 64,
            .height = 32,
            .depth = 1,
            .mipCount = 4,
            .estimatedBytes = 10'880,
            .byteAccuracy = Accuracy::Estimated});
        const ResourceId id = original.GetId();
        ctx.Check(id != 0, "the texture metadata received a stable resource ID");

        sourceLabel.assign("caller changed");
        sourceFormat.assign("caller changed");
        const Snapshot registered = provider.CaptureSnapshot();
        const ResourceRecord* texture = FindResource(registered, id);
        ctx.Check(texture != nullptr && texture->kind == ResourceKind::Texture2D &&
                      texture->label == "demo albedo" && texture->format == "Color" &&
                      texture->width == 64 && texture->height == 32 && texture->depth == 1 &&
                      texture->mipCount == 4 && texture->estimatedBytes == 10'880 &&
                      texture->byteAccuracy == Accuracy::Estimated,
                  "the registry owns the complete original texture descriptor");
        ctx.Check(registered.resources.size() == baseline.resources.size() + 1 &&
                      registered.registeredResourceBytes == baseline.registeredResourceBytes + 10'880,
                  "the snapshot adds one resource and its estimated byte count");

        original.Update(ResourceDescriptor{
            .kind = ResourceKind::RenderTargetCube,
            .label = "demo reflection target",
            .format = "Rgba16Float",
            .width = 128,
            .height = 128,
            .depth = 6,
            .mipCount = 1,
            .estimatedBytes = 786'432,
            .byteAccuracy = Accuracy::Exact});
        const Snapshot updated = provider.CaptureSnapshot();
        const ResourceRecord* target = FindResource(updated, id);
        ctx.Check(target != nullptr && target->id == id &&
                      target->kind == ResourceKind::RenderTargetCube &&
                      target->label == "demo reflection target" && target->format == "Rgba16Float" &&
                      target->width == 128 && target->height == 128 && target->depth == 6 &&
                      target->mipCount == 1 && target->estimatedBytes == 786'432 &&
                      target->byteAccuracy == Accuracy::Exact,
                  "Update replaces every metadata field without changing the ID");
        ctx.Check(updated.registeredResourceBytes == baseline.registeredResourceBytes + 786'432,
                  "updating metadata replaces its contribution to the byte aggregate");

        ResourceHandle moved(std::move(original));
        ResourceHandle owner;
        owner = std::move(moved);
        ctx.Check(original.GetId() == 0 && moved.GetId() == 0 && owner.GetId() == id,
                  "move construction and assignment transfer one live registration");
        owner.Reset();
        owner.Reset();
        const Snapshot released = provider.CaptureSnapshot();
        ctx.Check(FindResource(released, id) == nullptr &&
                      released.resources.size() == baseline.resources.size() &&
                      released.registeredResourceBytes == baseline.registeredResourceBytes,
                  "idempotent Reset removes the record and restores the byte aggregate");

        const EventBatch events = provider.ReadEvents(eventCursor, 8);
        const EventRecord* created = FindResourceEvent(events, id, EventKind::ResourceCreated);
        const EventRecord* destroyed = FindResourceEvent(events, id, EventKind::ResourceDestroyed);
        ctx.Check(events.events.size() == 2 && created != nullptr && destroyed != nullptr &&
                      created->sequence < destroyed->sequence && created->category == Category::Core &&
                      destroyed->category == Category::Core,
                  "the provider cursor returns the ordered create and destroy events");
        ctx.Check(created != nullptr && destroyed != nullptr &&
                      provider.ResolveName(created->name) == "Diagnostics/ResourceCreated" &&
                      provider.ResolveName(destroyed->name) == "Diagnostics/ResourceDestroyed",
                  "the provider resolves both lifecycle event names");
        const EventBatch afterCreated = provider.ReadEvents(
            created == nullptr ? 0 : created->sequence, 8);
        ctx.Check(created != nullptr && destroyed != nullptr && afterCreated.events.size() == 1 &&
                      afterCreated.events.front().sequence == destroyed->sequence &&
                      events.eventsDroppedBeforeStart == 0 && events.producerEventsDropped == 0,
                  "the sequence cursor returns only the newer event without drops");

        const Trace trace = StopRecording(recording);
        ctx.Check(trace.GetEvents().size() == 2 && trace.GetDroppedEventCount() == 0,
                  "the bounded recording contains exactly the two lifecycle events");
        ctx.Print("resource {} updated to {} bytes, then released with {} lifecycle events",
                  id, 786'432, trace.GetEvents().size());
    }
}

CNA_DEMOS_CONSOLE(ResourceMetadata, {
    .id = "diagnostics.profiler.resource_metadata",
    .title = "Resource metadata lifecycle",
    .description = "Registers owned texture metadata, updates every field while preserving the resource ID, moves and "
                   "resets the RAII handle, verifies aggregate bytes, and consumes the ordered FULL create/destroy "
                   "events through CNA's provider cursor.",
    .apis = {"CNA::Diagnostics::ResourceDescriptor", "CNA::Diagnostics::ResourceRecord",
             "CNA::Diagnostics::ResourceHandle", "CNA::Diagnostics::ResourceHandle::Update",
             "CNA::Diagnostics::ResourceHandle::Reset", "CNA::Diagnostics::ResourceHandle::GetId",
             "CNA::Diagnostics::ResourceKind", "CNA::Diagnostics::Accuracy",
             "CNA::Diagnostics::Snapshot::registeredResourceBytes", "CNA::Diagnostics::EventBatch",
             "CNA::Diagnostics::IDiagnosticsProvider::ReadEvents",
             "CNA::Diagnostics::IDiagnosticsProvider::ResolveName"},
    .tags = {"diagnostics", "profiler", "resource", "metadata", "lifecycle", "cnaext"},
    .ownProcess = true,
})
