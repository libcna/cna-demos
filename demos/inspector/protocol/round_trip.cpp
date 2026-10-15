// SPDX-License-Identifier: MS-PL
// CNA Inspector protocol identities, message round trips and strict decoder boundaries.
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "CNA/Inspector/Protocol.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    using namespace CNA::Inspector;

    template<typename Exception, typename Function>
    bool Throws(Function&& function)
    {
        try
        {
            function();
        }
        catch (const Exception&)
        {
            return true;
        }
        return false;
    }

    void ProtocolRoundTrip(CnaDemos::DemoContext& ctx)
    {
        ctx.Check(ProtocolMajorVersion == 1 && ProtocolMinorVersion == 0
                      && MaximumPayloadBytes == 8U * 1024U * 1024U
                      && MaximumEventsPerResponse == 4096U
                      && MaximumPreviewBytes == 4U * 1024U * 1024U,
                  "protocol version and public bounds are exact");

        const auto allCapabilities = static_cast<std::uint64_t>(Capability::Snapshots)
            | static_cast<std::uint64_t>(Capability::Events)
            | static_cast<std::uint64_t>(Capability::ResourceMetadata)
            | static_cast<std::uint64_t>(Capability::CpuZones)
            | static_cast<std::uint64_t>(Capability::Accuracy)
            | static_cast<std::uint64_t>(Capability::Discontinuities)
            | static_cast<std::uint64_t>(Capability::ResourcePreviews);
        const auto allParts = static_cast<std::uint32_t>(SnapshotPart::Metrics)
            | static_cast<std::uint32_t>(SnapshotPart::Frames)
            | static_cast<std::uint32_t>(SnapshotPart::Resources);
        ctx.Check(HasCapability(allCapabilities, Capability::CpuZones)
                      && !HasCapability(0, Capability::CpuZones)
                      && HasSnapshotPart(allParts, SnapshotPart::Resources)
                      && !HasSnapshotPart(0, SnapshotPart::Resources),
                  "capability and snapshot masks expose their requested bits");

        PacketHeader header;
        header.type = MessageType::EventsResponse;
        header.payloadBytes = 1234;
        header.requestId = 0x1122334455667788ULL;
        const auto headerBytes = ProtocolCodec::EncodeHeader(header);
        PacketHeader decodedHeader;
        std::string error;
        const bool headerDecoded = ProtocolCodec::DecodeHeader(headerBytes, decodedHeader, error);
        ctx.Check(headerBytes.size() == 24 && headerDecoded && decodedHeader.majorVersion == 1
                      && decodedHeader.minorVersion == 0
                      && decodedHeader.type == MessageType::EventsResponse
                      && decodedHeader.payloadBytes == 1234
                      && decodedHeader.requestId == 0x1122334455667788ULL,
                  "the fixed header round-trips all public fields in exactly 24 bytes");

        auto damagedHeader = headerBytes;
        damagedHeader[0] = 0;
        error.clear();
        const bool badMagicRejected = !ProtocolCodec::DecodeHeader(damagedHeader, decodedHeader, error);
        damagedHeader = headerBytes;
        damagedHeader[8] = 0xFF;
        damagedHeader[9] = 0xFF;
        error.clear();
        const bool badTypeRejected = !ProtocolCodec::DecodeHeader(damagedHeader, decodedHeader, error);
        error.clear();
        const bool shortHeaderRejected = !ProtocolCodec::DecodeHeader(
            std::span<const std::uint8_t>(headerBytes.data(), headerBytes.size() - 1), decodedHeader, error);
        ctx.Check(badMagicRejected && badTypeRejected && shortHeaderRejected,
                  "the header decoder rejects bad magic, unknown types and non-24-byte input");

        HelloRequest hello;
        hello.minimumMajorVersion = 1;
        hello.maximumMajorVersion = 2;
        hello.requestedCapabilities = allCapabilities;
        hello.clientName = "protocol-demo";
        hello.authenticationToken = "owned-token";
        const Packet helloPacket = ProtocolCodec::Encode(hello, 11);
        HelloRequest decodedHello;
        error.clear();
        const bool helloDecoded = ProtocolCodec::Decode(helloPacket, decodedHello, error);
        ctx.Check(helloDecoded && helloPacket.header.type == MessageType::ClientHello
                      && helloPacket.header.requestId == 11
                      && decodedHello.minimumMajorVersion == 1
                      && decodedHello.maximumMajorVersion == 2
                      && decodedHello.requestedCapabilities == allCapabilities
                      && decodedHello.clientName == "protocol-demo"
                      && decodedHello.authenticationToken == "owned-token",
                  "client hello preserves version range, capabilities, identity and token");

        HelloResponse session;
        session.selectedMajorVersion = 1;
        session.selectedMinorVersion = 0;
        session.providerInterfaceVersion = 1;
        session.capabilities = allCapabilities;
        session.maximumPayloadBytes = static_cast<std::uint32_t>(MaximumPayloadBytes);
        session.maximumEventsPerResponse = static_cast<std::uint32_t>(MaximumEventsPerResponse);
        session.maximumPreviewBytes = static_cast<std::uint32_t>(MaximumPreviewBytes);
        session.localOnly = true;
        session.applicationName = "Protocol demo";
        session.cnaVersion = "0.1.0";
        session.targetPlatform = "DESKTOP_GL";
        session.platformBackend = "SDL3";
        session.graphicsRenderer = "SOFTWARE";
        session.buildConfiguration = "Debug";
        session.metadata = {{"scene", "wire"}, {"owner", "demo"}};
        const Packet sessionPacket = ProtocolCodec::Encode(session, 12);
        HelloResponse decodedSession;
        error.clear();
        const bool sessionDecoded = ProtocolCodec::Decode(sessionPacket, decodedSession, error);
        ctx.Check(sessionDecoded && decodedSession.capabilities == allCapabilities
                      && decodedSession.localOnly && decodedSession.applicationName == "Protocol demo"
                      && decodedSession.platformBackend == "SDL3"
                      && decodedSession.graphicsRenderer == "SOFTWARE"
                      && decodedSession.metadata.size() == 2
                      && decodedSession.metadata[1].key == "owner"
                      && decodedSession.metadata[1].value == "demo",
                  "server hello preserves limits, platform identity and ordered metadata");

        SnapshotRequest snapshotRequest;
        snapshotRequest.parts = allParts;
        const Packet snapshotRequestPacket = ProtocolCodec::Encode(snapshotRequest, 13);
        SnapshotRequest decodedSnapshotRequest;
        error.clear();
        ctx.Check(ProtocolCodec::Decode(snapshotRequestPacket, decodedSnapshotRequest, error)
                      && decodedSnapshotRequest.parts == allParts,
                  "snapshot request preserves every selectable part");

        SnapshotResponse snapshotResponse;
        snapshotResponse.includedParts = allParts;
        snapshotResponse.metricCount = 1;
        snapshotResponse.frameCount = 1;
        snapshotResponse.resourceCount = 1;
        auto& snapshot = snapshotResponse.snapshot;
        snapshot.buildMode = CNA::Diagnostics::Mode::Full;
        snapshot.runtimeMode = CNA::Diagnostics::Mode::Stats;
        snapshot.currentFrameNumber = 27;
        snapshot.malformedZoneCount = 2;
        snapshot.malformedFrameCount = 3;
        snapshot.producerEventsDropped = 4;
        snapshot.eventHistoryOverwrites = 5;
        snapshot.sourceCollectionFailures = 6;
        snapshot.profilerOwnedBytes = 700;
        snapshot.registeredResourceBytes = 800;
        snapshot.metrics.push_back({9, "Graphics/DrawCalls", 12,
            CNA::Diagnostics::MetricKind::FrameCounter, CNA::Diagnostics::MetricUnit::Count,
            CNA::Diagnostics::Accuracy::Exact});
        CNA::Diagnostics::FrameSample frame;
        frame.frameNumber = 27;
        frame.startTimestampNs = 1000;
        frame.durationNs = 16666667;
        frame.framesPerSecond = 60.0;
        frame.metrics = snapshot.metrics;
        snapshot.recentFrames.push_back(frame);
        snapshot.resources.push_back({17, CNA::Diagnostics::ResourceKind::Texture2D, "Albedo",
            "Color", 64, 32, 1, 2, 8192, CNA::Diagnostics::Accuracy::Estimated});
        const Packet snapshotPacket = ProtocolCodec::Encode(snapshotResponse, 14);
        SnapshotResponse decodedSnapshot;
        error.clear();
        const bool snapshotDecoded = ProtocolCodec::Decode(snapshotPacket, decodedSnapshot, error);
        ctx.Check(snapshotDecoded && decodedSnapshot.includedParts == allParts
                      && decodedSnapshot.metricCount == 1 && decodedSnapshot.frameCount == 1
                      && decodedSnapshot.resourceCount == 1
                      && decodedSnapshot.snapshot.currentFrameNumber == 27
                      && decodedSnapshot.snapshot.sourceCollectionFailures == 6
                      && decodedSnapshot.snapshot.metrics.size() == 1
                      && decodedSnapshot.snapshot.metrics[0].name == "Graphics/DrawCalls"
                      && decodedSnapshot.snapshot.metrics[0].kind
                          == CNA::Diagnostics::MetricKind::FrameCounter,
                  "snapshot response preserves mode, counts, loss state and metric data");
        ctx.Check(snapshotDecoded && decodedSnapshot.snapshot.recentFrames.size() == 1
                      && decodedSnapshot.snapshot.recentFrames[0].framesPerSecond == 60.0
                      && decodedSnapshot.snapshot.recentFrames[0].metrics[0].value == 12
                      && decodedSnapshot.snapshot.resources.size() == 1
                      && decodedSnapshot.snapshot.resources[0].id == 17
                      && decodedSnapshot.snapshot.resources[0].byteAccuracy
                          == CNA::Diagnostics::Accuracy::Estimated,
                  "snapshot response preserves frame history and resource metadata");

        EventsRequest eventsRequest{41, 128};
        const Packet eventsRequestPacket = ProtocolCodec::Encode(eventsRequest, 15);
        EventsRequest decodedEventsRequest;
        error.clear();
        ctx.Check(ProtocolCodec::Decode(eventsRequestPacket, decodedEventsRequest, error)
                      && decodedEventsRequest.afterSequence == 41
                      && decodedEventsRequest.maximumEvents == 128,
                  "event request preserves its cursor and bound");

        EventsResponse eventsResponse;
        eventsResponse.oldestAvailableSequence = 40;
        eventsResponse.newestAvailableSequence = 80;
        eventsResponse.eventsDroppedBeforeStart = 12;
        eventsResponse.producerEventsDropped = 7;
        eventsResponse.events.push_back({{80, 1000, 20, 9, 4, 5, 0, -3, 7,
            CNA::Diagnostics::EventKind::Zone, CNA::Diagnostics::Category::Update}, "Game/Update"});
        const Packet eventsPacket = ProtocolCodec::Encode(eventsResponse, 16);
        EventsResponse decodedEvents;
        error.clear();
        const bool eventsDecoded = ProtocolCodec::Decode(eventsPacket, decodedEvents, error);
        ctx.Check(eventsDecoded && decodedEvents.oldestAvailableSequence == 40
                      && decodedEvents.newestAvailableSequence == 80
                      && decodedEvents.eventsDroppedBeforeStart == 12
                      && decodedEvents.producerEventsDropped == 7
                      && decodedEvents.events.size() == 1
                      && decodedEvents.events[0].event.value == -3
                      && decodedEvents.events[0].name == "Game/Update",
                  "event response preserves cursor discontinuities and resolved event data");

        PreviewRequest previewRequest{17, 320, 180, 4096};
        const Packet previewRequestPacket = ProtocolCodec::Encode(previewRequest, 17);
        PreviewRequest decodedPreviewRequest;
        error.clear();
        ctx.Check(ProtocolCodec::Decode(previewRequestPacket, decodedPreviewRequest, error)
                      && decodedPreviewRequest.resourceId == 17
                      && decodedPreviewRequest.maximumWidth == 320
                      && decodedPreviewRequest.maximumHeight == 180
                      && decodedPreviewRequest.maximumBytes == 4096,
                  "preview request preserves its resource and output bounds");

        PreviewPollRequest pollRequest{42};
        const Packet pollPacket = ProtocolCodec::Encode(pollRequest, 18);
        PreviewPollRequest decodedPoll;
        error.clear();
        ctx.Check(ProtocolCodec::Decode(pollPacket, decodedPoll, error) && decodedPoll.ticket == 42,
                  "preview poll preserves its nonzero ticket");

        PreviewResponse previewResponse;
        previewResponse.status = PreviewStatus::Ready;
        previewResponse.ticket = 42;
        previewResponse.width = 2;
        previewResponse.height = 1;
        previewResponse.mimeType = "image/png";
        previewResponse.encodedImage = {0x89, 0x50, 0x4E, 0x47};
        previewResponse.message = "ready";
        const Packet previewPacket = ProtocolCodec::Encode(previewResponse, 19);
        PreviewResponse decodedPreview;
        error.clear();
        ctx.Check(ProtocolCodec::Decode(previewPacket, decodedPreview, error)
                      && decodedPreview.status == PreviewStatus::Ready
                      && decodedPreview.ticket == 42 && decodedPreview.width == 2
                      && decodedPreview.height == 1 && decodedPreview.mimeType == "image/png"
                      && decodedPreview.encodedImage == previewResponse.encodedImage
                      && decodedPreview.message == "ready",
                  "ready preview preserves its bounded image and message");

        ErrorResponse failure{ErrorCode::Busy, "try later"};
        const Packet failurePacket = ProtocolCodec::Encode(failure, 20);
        ErrorResponse decodedFailure;
        error.clear();
        ctx.Check(ProtocolCodec::Decode(failurePacket, decodedFailure, error)
                      && decodedFailure.code == ErrorCode::Busy
                      && decodedFailure.message == "try later",
                  "structured error preserves its code and message");

        Packet malformed = eventsRequestPacket;
        malformed.header.type = MessageType::SnapshotRequest;
        error.clear();
        const bool wrongTypeRejected = !ProtocolCodec::Decode(malformed, decodedEventsRequest, error);
        malformed = eventsRequestPacket;
        malformed.payload.push_back(0);
        error.clear();
        const bool wrongLengthRejected = !ProtocolCodec::Decode(malformed, decodedEventsRequest, error);
        malformed = eventsRequestPacket;
        malformed.payload.push_back(0);
        ++malformed.header.payloadBytes;
        error.clear();
        const bool trailingByteRejected = !ProtocolCodec::Decode(malformed, decodedEventsRequest, error);
        ctx.Check(wrongTypeRejected && wrongLengthRejected && trailingByteRejected,
                  "message decoders reject wrong type, length mismatch and trailing bytes");

        auto invalidEvents = ProtocolCodec::Encode(
            EventsRequest{0, static_cast<std::uint32_t>(MaximumEventsPerResponse + 1)}, 21);
        error.clear();
        const bool eventBoundRejected = !ProtocolCodec::Decode(invalidEvents, decodedEventsRequest, error);
        auto invalidPreview = ProtocolCodec::Encode(PreviewRequest{}, 22);
        error.clear();
        const bool previewBoundRejected = !ProtocolCodec::Decode(invalidPreview, decodedPreviewRequest, error);
        auto invalidPoll = ProtocolCodec::Encode(PreviewPollRequest{}, 23);
        error.clear();
        const bool pollBoundRejected = !ProtocolCodec::Decode(invalidPoll, decodedPoll, error);
        ctx.Check(eventBoundRejected && previewBoundRejected && pollBoundRejected,
                  "request decoders enforce event, resource and ticket bounds");

        PreviewResponse invalidPending;
        invalidPending.status = PreviewStatus::Pending;
        const Packet invalidPendingPacket = ProtocolCodec::Encode(invalidPending, 24);
        error.clear();
        const bool pendingTicketRejected = !ProtocolCodec::Decode(
            invalidPendingPacket, decodedPreview, error);
        HelloRequest longHello;
        longHello.clientName.assign(129, 'x');
        ctx.Check(pendingTicketRejected
                      && Throws<std::length_error>([&] { (void) ProtocolCodec::Encode(longHello, 25); }),
                  "preview state and encoded string limits are enforced");

        ctx.Print("Protocol v{}.{}: {}-byte payloads, {} events, {}-byte previews",
                  ProtocolMajorVersion, ProtocolMinorVersion, MaximumPayloadBytes,
                  MaximumEventsPerResponse, MaximumPreviewBytes);
    }
}

CNA_DEMOS_CONSOLE(ProtocolRoundTrip, {
    .id = "inspector.protocol.round_trip",
    .title = "Inspector protocol round trip",
    .description = "Round-trips every Inspector version-one payload family and demonstrates strict header, type, length and request bounds.",
    .apis = {"CNA::Inspector::ProtocolMajorVersion", "CNA::Inspector::ProtocolMinorVersion",
             "CNA::Inspector::MaximumPayloadBytes", "CNA::Inspector::MaximumEventsPerResponse",
             "CNA::Inspector::MaximumPreviewBytes", "CNA::Inspector::MessageType",
             "CNA::Inspector::Capability", "CNA::Inspector::SnapshotPart",
             "CNA::Inspector::ErrorCode", "CNA::Inspector::PreviewStatus",
             "CNA::Inspector::PacketHeader", "CNA::Inspector::Packet",
             "CNA::Inspector::MetadataEntry", "CNA::Inspector::HelloRequest",
             "CNA::Inspector::HelloResponse", "CNA::Inspector::SnapshotRequest",
             "CNA::Inspector::SnapshotResponse", "CNA::Inspector::EventsRequest",
             "CNA::Inspector::ResolvedEvent", "CNA::Inspector::EventsResponse",
             "CNA::Inspector::PreviewRequest", "CNA::Inspector::PreviewPollRequest",
             "CNA::Inspector::PreviewResponse", "CNA::Inspector::ErrorResponse",
             "CNA::Inspector::ProtocolCodec::EncodeHeader",
             "CNA::Inspector::ProtocolCodec::DecodeHeader", "CNA::Inspector::ProtocolCodec::Encode",
             "CNA::Inspector::ProtocolCodec::Decode", "CNA::Inspector::HasCapability",
             "CNA::Inspector::HasSnapshotPart"},
    .tags = {"diagnostics", "inspector", "protocol", "serialization", "bounds"},
})
