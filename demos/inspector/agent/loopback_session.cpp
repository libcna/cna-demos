// SPDX-License-Identifier: MS-PL
// CNA Inspector agent/client loopback negotiation, demand-driven diagnostics and lifecycle.
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <string>

#include "CNA/Inspector/Agent.hpp"
#include "CNA/Inspector/Client.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    using namespace CNA::Inspector;

    class DemoProvider final : public CNA::Diagnostics::IDiagnosticsProvider
    {
    public:
        CNA::Diagnostics::Snapshot CaptureSnapshot() override
        {
            ++snapshotCalls;
            return snapshot;
        }

        CNA::Diagnostics::EventBatch ReadEvents(std::uint64_t afterSequence,
                                                 std::size_t maximumEvents) override
        {
            ++eventCalls;
            lastAfterSequence.store(afterSequence);
            lastMaximumEvents.store(maximumEvents);
            return events;
        }

        std::string ResolveName(CNA::Diagnostics::NameId id) override
        {
            ++resolveCalls;
            return id == 7 ? "Game/Update" : "unknown";
        }

        CNA::Diagnostics::Snapshot snapshot;
        CNA::Diagnostics::EventBatch events;
        std::atomic<int> snapshotCalls{0};
        std::atomic<int> eventCalls{0};
        std::atomic<int> resolveCalls{0};
        std::atomic<std::uint64_t> lastAfterSequence{0};
        std::atomic<std::size_t> lastMaximumEvents{0};
    };

    AgentConfiguration MakeAgentConfiguration()
    {
        AgentConfiguration configuration;
        configuration.bindAddress = "127.0.0.1";
        configuration.port = 0;
        configuration.authenticationToken = "cna-demos-inspector-token";
        configuration.applicationName = "cna-demos Inspector session";
        configuration.platformBackend = "demo-platform";
        configuration.buildConfiguration = "focused-validation";
        configuration.metadata = {{"scenario", "loopback"}, {"owner", "cna-demos"}};
        configuration.maximumRequestsPerSecond = 64;
        configuration.maximumPendingPreviews = 2;
        configuration.previewCooldownMilliseconds = 50;
        return configuration;
    }

    ClientConfiguration MakeClientConfiguration(const Agent& agent)
    {
        ClientConfiguration configuration;
        configuration.host = agent.GetBindAddress();
        configuration.port = agent.GetPort();
        configuration.authenticationToken = agent.GetAuthenticationToken();
        configuration.clientName = "cna-demos";
        configuration.timeoutMilliseconds = 2000;
        return configuration;
    }

    void LoopbackSession(CnaDemos::DemoContext& ctx)
    {
        ClientConfiguration clientDefaults;
        AgentConfiguration agentDefaults;
        ctx.Check(clientDefaults.host == "127.0.0.1" && clientDefaults.port == 0
                      && clientDefaults.authenticationToken.empty()
                      && clientDefaults.clientName == "cna-inspector"
                      && clientDefaults.timeoutMilliseconds == 3000,
                  "client configuration has safe disconnected loopback defaults");
        ctx.Check(agentDefaults.bindAddress == "127.0.0.1" && agentDefaults.port == 0
                      && !agentDefaults.allowRemote && agentDefaults.authenticationToken.empty()
                      && agentDefaults.applicationName == "CNA application"
                      && agentDefaults.maximumRequestsPerSecond == 64
                      && agentDefaults.maximumPendingPreviews == 4
                      && agentDefaults.previewCooldownMilliseconds == 250,
                  "agent configuration defaults to local explicit bounded service");

        DemoProvider provider;
        provider.snapshot.buildMode = CNA::Diagnostics::Mode::Full;
        provider.snapshot.runtimeMode = CNA::Diagnostics::Mode::Full;
        provider.snapshot.currentFrameNumber = 91;
        provider.snapshot.producerEventsDropped = 3;
        provider.snapshot.eventHistoryOverwrites = 4;
        provider.snapshot.metrics.push_back({1, "Graphics/DrawCalls", 12,
            CNA::Diagnostics::MetricKind::FrameCounter, CNA::Diagnostics::MetricUnit::Count,
            CNA::Diagnostics::Accuracy::Exact});
        CNA::Diagnostics::FrameSample frame;
        frame.frameNumber = 91;
        frame.durationNs = 16666667;
        frame.framesPerSecond = 60.0;
        frame.metrics = provider.snapshot.metrics;
        provider.snapshot.recentFrames.push_back(frame);
        provider.snapshot.resources.push_back({17, CNA::Diagnostics::ResourceKind::Texture2D,
            "Albedo", "Color", 64, 32, 1, 1, 8192, CNA::Diagnostics::Accuracy::Estimated});
        provider.events.oldestAvailableSequence = 11;
        provider.events.newestAvailableSequence = 20;
        provider.events.eventsDroppedBeforeStart = 2;
        provider.events.producerEventsDropped = 5;
        provider.events.events.push_back({20, 100, 30, 91, 3, 4, 0, -2, 7,
            CNA::Diagnostics::EventKind::Zone, CNA::Diagnostics::Category::Update});

        std::string error;
        auto agent = Agent::Start(provider, MakeAgentConfiguration(), error);
        ctx.Check(agent != nullptr && error.empty(), "agent starts over the injected provider");
        if (!agent)
        {
            ctx.Fail("the remaining session checks require a running agent");
            return;
        }
        ctx.Check(agent->IsRunning() && agent->GetBindAddress() == "127.0.0.1"
                      && agent->GetPort() != 0
                      && agent->GetAuthenticationToken() == "cna-demos-inspector-token",
                  "agent exposes running state, loopback endpoint, ephemeral port and owned token");
        ctx.Check(provider.snapshotCalls.load() == 0 && provider.eventCalls.load() == 0,
                  "starting an idle agent does not poll the diagnostics provider");

        Client rejectedClient;
        auto rejectedConfiguration = MakeClientConfiguration(*agent);
        rejectedConfiguration.authenticationToken = "wrong-token";
        error.clear();
        ctx.Check(!rejectedClient.Connect(rejectedConfiguration, error)
                      && !rejectedClient.IsConnected() && !error.empty()
                      && provider.snapshotCalls.load() == 0,
                  "bad authentication is rejected before provider access");

        Client client;
        error.clear();
        const bool connected = client.Connect(MakeClientConfiguration(*agent), error);
        ctx.Check(connected && client.IsConnected() && error.empty()
                      && provider.snapshotCalls.load() == 1,
                  "client authenticates and negotiation samples the provider once");
        if (!connected)
        {
            ctx.Fail("the remaining client requests require a negotiated session");
            agent->Stop();
            return;
        }

        const auto& session = client.GetSession();
        ctx.Check(session.selectedMajorVersion == ProtocolMajorVersion
                      && session.selectedMinorVersion == ProtocolMinorVersion
                      && session.providerInterfaceVersion
                          == CNA::Diagnostics::IDiagnosticsProvider::InterfaceVersion
                      && session.localOnly
                      && session.maximumPayloadBytes == MaximumPayloadBytes
                      && session.maximumEventsPerResponse == MaximumEventsPerResponse
                      && session.maximumPreviewBytes == MaximumPreviewBytes,
                  "negotiated session exposes exact protocol, provider and local limits");
        ctx.Check(session.applicationName == "cna-demos Inspector session"
                      && session.platformBackend == "demo-platform"
                      && session.buildConfiguration == "focused-validation"
                      && !session.cnaVersion.empty() && !session.targetPlatform.empty()
                      && !session.graphicsRenderer.empty()
                      && session.metadata.size() == 2
                      && session.metadata[0].key == "scenario"
                      && session.metadata[0].value == "loopback",
                  "negotiated session owns application, CNA, platform, renderer and metadata identity");
        ctx.Check(HasCapability(session.capabilities, Capability::Snapshots)
                      && HasCapability(session.capabilities, Capability::Events)
                      && HasCapability(session.capabilities, Capability::ResourceMetadata)
                      && HasCapability(session.capabilities, Capability::CpuZones)
                      && HasCapability(session.capabilities, Capability::Accuracy)
                      && HasCapability(session.capabilities, Capability::Discontinuities)
                      && !HasCapability(session.capabilities, Capability::ResourcePreviews),
                  "full provider negotiates diagnostics capabilities without an absent preview source");

        SnapshotRequest metricsOnly;
        metricsOnly.parts = static_cast<std::uint32_t>(SnapshotPart::Metrics);
        SnapshotResponse snapshotResponse;
        error.clear();
        const bool captured = client.CaptureSnapshot(metricsOnly, snapshotResponse, error);
        ctx.Check(captured && error.empty() && snapshotResponse.includedParts == metricsOnly.parts
                      && snapshotResponse.metricCount == 1 && snapshotResponse.frameCount == 1
                      && snapshotResponse.resourceCount == 1
                      && snapshotResponse.snapshot.metrics.size() == 1
                      && snapshotResponse.snapshot.metrics[0].value == 12
                      && snapshotResponse.snapshot.recentFrames.empty()
                      && snapshotResponse.snapshot.resources.empty(),
                  "selective snapshot returns requested metrics and total unfiltered counts");
        ctx.Check(provider.snapshotCalls.load() == 2,
                  "snapshot data is captured only when negotiation or a request demands it");

        EventsResponse eventsResponse;
        error.clear();
        const bool read = client.ReadEvents({10, 64}, eventsResponse, error);
        ctx.Check(read && error.empty() && provider.eventCalls.load() == 1
                      && provider.lastAfterSequence.load() == 10
                      && provider.lastMaximumEvents.load() == 64
                      && provider.resolveCalls.load() == 1
                      && eventsResponse.eventsDroppedBeforeStart == 2
                      && eventsResponse.producerEventsDropped == 5
                      && eventsResponse.events.size() == 1
                      && eventsResponse.events[0].name == "Game/Update"
                      && eventsResponse.events[0].event.value == -2,
                  "event request forwards cursor and bound and returns resolved loss-aware events");

        PreviewResponse unavailable;
        error.clear();
        ctx.Check(client.RequestPreview({17, 64, 64, 4096}, unavailable, error)
                      && error.empty() && unavailable.status == PreviewStatus::Unavailable
                      && !unavailable.message.empty(),
                  "missing preview provider is a normal unavailable response");
        error.clear();
        ctx.Check(client.Ping(error) && error.empty() && client.IsConnected(),
                  "ping confirms that an unavailable optional preview leaves the session connected");

        client.Disconnect();
        client.Disconnect();
        error.clear();
        SnapshotResponse disconnectedResponse;
        ctx.Check(!client.IsConnected()
                      && !client.CaptureSnapshot({}, disconnectedResponse, error)
                      && !error.empty(),
                  "disconnect is idempotent and requests fail safely while disconnected");

        error.clear();
        ctx.Check(client.Connect(MakeClientConfiguration(*agent), error)
                      && client.IsConnected() && client.Ping(error),
                  "the same client reconnects and resumes requests after disconnect");
        client.Disconnect();

        agent->Stop();
        agent->Stop();
        ctx.Check(!agent->IsRunning(), "agent stop is idempotent and joins its listener thread");

        AgentConfiguration tokenConfiguration = MakeAgentConfiguration();
        tokenConfiguration.authenticationToken.clear();
        error.clear();
        auto generatedTokenAgent = Agent::Start(tokenConfiguration, error);
        const bool generatedTokenValid = generatedTokenAgent
            && generatedTokenAgent->GetAuthenticationToken().size() == 64
            && std::all_of(generatedTokenAgent->GetAuthenticationToken().begin(),
                           generatedTokenAgent->GetAuthenticationToken().end(),
                           [](unsigned char character) {
                               return (character >= '0' && character <= '9')
                                   || (character >= 'a' && character <= 'f');
                           });
        ctx.Check(generatedTokenValid,
                  "omitting a token generates a 256-bit lowercase hexadecimal session secret");
        Client processClient;
        bool processCapabilitiesMatch = false;
        if (generatedTokenAgent)
        {
            error.clear();
            if (processClient.Connect(MakeClientConfiguration(*generatedTokenAgent), error))
            {
                const auto buildMode = CNA::Diagnostics::GetBuildMode();
                const auto capabilities = processClient.GetSession().capabilities;
                processCapabilitiesMatch = HasCapability(capabilities, Capability::Snapshots)
                    && HasCapability(capabilities, Capability::Accuracy)
                    && HasCapability(capabilities, Capability::Discontinuities)
                    && HasCapability(capabilities, Capability::ResourceMetadata)
                        == (buildMode >= CNA::Diagnostics::Mode::Stats)
                    && HasCapability(capabilities, Capability::Events)
                        == (buildMode >= CNA::Diagnostics::Mode::Full)
                    && HasCapability(capabilities, Capability::CpuZones)
                        == (buildMode >= CNA::Diagnostics::Mode::Full);
                processClient.Disconnect();
            }
            generatedTokenAgent->Stop();
        }
        ctx.Check(processCapabilitiesMatch,
                  "process-provider overload negotiates capabilities from the compiled diagnostics mode");

        AgentConfiguration statsConfiguration = MakeAgentConfiguration();
        statsConfiguration.authenticationToken = "stats-only-token";
        provider.snapshot.buildMode = CNA::Diagnostics::Mode::Stats;
        provider.snapshot.runtimeMode = CNA::Diagnostics::Mode::Stats;
        error.clear();
        auto statsAgent = Agent::Start(provider, statsConfiguration, error);
        Client statsClient;
        bool statsBoundaryObserved = false;
        if (statsAgent)
        {
            error.clear();
            if (statsClient.Connect(MakeClientConfiguration(*statsAgent), error))
            {
                const auto capabilities = statsClient.GetSession().capabilities;
                const int eventCallsBefore = provider.eventCalls.load();
                EventsResponse unavailableEvents;
                error.clear();
                statsBoundaryObserved = HasCapability(capabilities, Capability::Snapshots)
                    && HasCapability(capabilities, Capability::ResourceMetadata)
                    && !HasCapability(capabilities, Capability::Events)
                    && !HasCapability(capabilities, Capability::CpuZones)
                    && !statsClient.ReadEvents({0, 1}, unavailableEvents, error)
                    && !error.empty() && provider.eventCalls.load() == eventCallsBefore
                    && statsClient.IsConnected();
                std::string pingError;
                statsBoundaryObserved = statsBoundaryObserved && statsClient.Ping(pingError);
                statsClient.Disconnect();
            }
            statsAgent->Stop();
        }
        ctx.Check(statsBoundaryObserved,
                  "STATS negotiation omits FULL event capabilities and refuses before provider access");

        AgentConfiguration rateConfiguration = MakeAgentConfiguration();
        rateConfiguration.authenticationToken = "rate-limit-token";
        rateConfiguration.maximumRequestsPerSecond = 2;
        provider.snapshot.buildMode = CNA::Diagnostics::Mode::Full;
        provider.snapshot.runtimeMode = CNA::Diagnostics::Mode::Full;
        error.clear();
        auto rateAgent = Agent::Start(provider, rateConfiguration, error);
        Client rateClient;
        bool boundedRateObserved = false;
        if (rateAgent)
        {
            error.clear();
            if (rateClient.Connect(MakeClientConfiguration(*rateAgent), error))
            {
                std::string firstError;
                std::string secondError;
                std::string thirdError;
                boundedRateObserved = rateClient.Ping(firstError) && firstError.empty()
                    && rateClient.Ping(secondError) && secondError.empty()
                    && !rateClient.Ping(thirdError) && !thirdError.empty()
                    && rateClient.IsConnected();
                rateClient.Disconnect();
            }
            rateAgent->Stop();
        }
        ctx.Check(boundedRateObserved,
                  "request-rate backpressure reports busy without disconnecting the client");

        AgentConfiguration remoteConfiguration = MakeAgentConfiguration();
        remoteConfiguration.bindAddress = "0.0.0.0";
        error.clear();
        ctx.Check(Agent::Start(provider, remoteConfiguration, error) == nullptr
                      && error.find("allowRemote") != std::string::npos,
                  "non-loopback binding requires explicit remote authorization");

        ctx.Print("Negotiated local Inspector protocol v{}.{} with {} metadata entries",
                  ProtocolMajorVersion, ProtocolMinorVersion, session.metadata.size());
    }
}

CNA_DEMOS_CONSOLE(LoopbackSession, {
    .id = "inspector.agent.loopback_session",
    .title = "Inspector loopback session",
    .description = "Starts an explicitly owned Inspector agent, negotiates an authenticated client and requests diagnostics on demand.",
    .apis = {"CNA::Inspector::IResourcePreviewProvider", "CNA::Inspector::AgentConfiguration",
             "CNA::Inspector::AgentConfiguration::bindAddress",
             "CNA::Inspector::AgentConfiguration::port",
             "CNA::Inspector::AgentConfiguration::allowRemote",
             "CNA::Inspector::AgentConfiguration::authenticationToken",
             "CNA::Inspector::AgentConfiguration::applicationName",
             "CNA::Inspector::AgentConfiguration::platformBackend",
             "CNA::Inspector::AgentConfiguration::buildConfiguration",
             "CNA::Inspector::AgentConfiguration::metadata",
             "CNA::Inspector::AgentConfiguration::maximumRequestsPerSecond",
             "CNA::Inspector::Agent::Start", "CNA::Inspector::Agent::Stop",
             "CNA::Inspector::Agent::IsRunning", "CNA::Inspector::Agent::GetBindAddress",
             "CNA::Inspector::Agent::GetPort", "CNA::Inspector::Agent::GetAuthenticationToken",
             "CNA::Inspector::ClientConfiguration", "CNA::Inspector::ClientConfiguration::host",
             "CNA::Inspector::ClientConfiguration::port",
             "CNA::Inspector::ClientConfiguration::authenticationToken",
             "CNA::Inspector::ClientConfiguration::clientName",
             "CNA::Inspector::ClientConfiguration::timeoutMilliseconds",
             "CNA::Inspector::Client::Client",
             "CNA::Inspector::Client::Connect", "CNA::Inspector::Client::Disconnect",
             "CNA::Inspector::Client::IsConnected", "CNA::Inspector::Client::GetSession",
             "CNA::Inspector::Client::CaptureSnapshot", "CNA::Inspector::Client::ReadEvents",
             "CNA::Inspector::Client::RequestPreview", "CNA::Inspector::Client::Ping"},
    .tags = {"diagnostics", "inspector", "agent", "client", "authentication", "loopback"},
    .needs = CnaDemos::Need::Network,
})
