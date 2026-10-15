// SPDX-License-Identifier: MS-PL
// CNA Inspector's explicit asynchronous and bounded resource-preview provider contract.
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "CNA/Inspector/Agent.hpp"
#include "CNA/Inspector/Client.hpp"
#include "CnaDemos/Demo.hpp"

namespace
{
    using namespace CNA::Inspector;

    class PreviewDiagnosticsProvider final : public CNA::Diagnostics::IDiagnosticsProvider
    {
    public:
        CNA::Diagnostics::Snapshot CaptureSnapshot() override
        {
            ++snapshotCalls;
            return snapshot;
        }

        CNA::Diagnostics::EventBatch ReadEvents(std::uint64_t, std::size_t) override
        {
            return {};
        }

        std::string ResolveName(CNA::Diagnostics::NameId) override
        {
            return {};
        }

        CNA::Diagnostics::Snapshot snapshot;
        std::atomic<int> snapshotCalls{0};
    };

    class PreviewProvider final : public IResourcePreviewProvider
    {
    public:
        PreviewResponse RequestPreview(const PreviewRequest& request) override
        {
            ++requestCalls;
            lastResourceId.store(request.resourceId);
            lastMaximumWidth.store(request.maximumWidth);
            lastMaximumHeight.store(request.maximumHeight);
            lastMaximumBytes.store(request.maximumBytes);
            if (returnOversizedImage)
            {
                PreviewResponse invalid;
                invalid.status = PreviewStatus::Ready;
                invalid.width = request.maximumWidth + 1;
                invalid.height = 1;
                invalid.mimeType = "image/png";
                invalid.encodedImage = {0x89, 0x50, 0x4E, 0x47};
                return invalid;
            }
            PreviewResponse pending;
            pending.status = PreviewStatus::Pending;
            pending.ticket = 42;
            pending.message = "readback pending";
            return pending;
        }

        PreviewResponse PollPreview(std::uint64_t ticket) override
        {
            ++pollCalls;
            lastPollTicket.store(ticket);
            if (pollCalls.load() <= pendingPolls)
            {
                PreviewResponse pending;
                pending.status = PreviewStatus::Pending;
                pending.ticket = ticket;
                pending.message = "readback still pending";
                return pending;
            }
            PreviewResponse ready;
            ready.status = PreviewStatus::Ready;
            ready.ticket = ticket;
            ready.width = 2;
            ready.height = 1;
            ready.mimeType = "image/png";
            ready.encodedImage = {0x89, 0x50, 0x4E, 0x47};
            ready.message = "readback complete";
            return ready;
        }

        std::atomic<int> requestCalls{0};
        std::atomic<int> pollCalls{0};
        std::atomic<CNA::Diagnostics::ResourceId> lastResourceId{0};
        std::atomic<std::uint32_t> lastMaximumWidth{0};
        std::atomic<std::uint32_t> lastMaximumHeight{0};
        std::atomic<std::uint32_t> lastMaximumBytes{0};
        std::atomic<std::uint64_t> lastPollTicket{0};
        int pendingPolls = 1;
        bool returnOversizedImage = false;
    };

    void PreviewPipeline(CnaDemos::DemoContext& ctx)
    {
        PreviewDiagnosticsProvider diagnostics;
        diagnostics.snapshot.buildMode = CNA::Diagnostics::Mode::Stats;
        diagnostics.snapshot.runtimeMode = CNA::Diagnostics::Mode::Stats;
        diagnostics.snapshot.resources.push_back({17, CNA::Diagnostics::ResourceKind::Texture2D,
            "Scene color", "Color", 320, 180, 1, 1, 230400,
            CNA::Diagnostics::Accuracy::Exact});
        diagnostics.snapshot.resources.push_back({18, CNA::Diagnostics::ResourceKind::VertexBuffer,
            "Geometry", "VertexPositionColor", 12, 0, 0, 0, 288,
            CNA::Diagnostics::Accuracy::Exact});

        auto previews = std::make_shared<PreviewProvider>();
        AgentConfiguration agentConfiguration;
        agentConfiguration.authenticationToken = "preview-demo-token";
        agentConfiguration.applicationName = "Inspector preview demo";
        agentConfiguration.previewProvider = previews;
        agentConfiguration.maximumPendingPreviews = 1;
        agentConfiguration.previewCooldownMilliseconds = 60000;
        std::string error;
        auto agent = Agent::Start(diagnostics, agentConfiguration, error);
        ctx.Check(agent != nullptr && error.empty(),
                  "agent starts with an explicitly installed preview provider");
        if (!agent)
        {
            ctx.Fail("the remaining preview checks require a running agent");
            return;
        }

        ClientConfiguration clientConfiguration;
        clientConfiguration.host = agent->GetBindAddress();
        clientConfiguration.port = agent->GetPort();
        clientConfiguration.authenticationToken = agent->GetAuthenticationToken();
        clientConfiguration.clientName = "preview-consumer";
        clientConfiguration.timeoutMilliseconds = 2000;
        Client client;
        error.clear();
        const bool connected = client.Connect(clientConfiguration, error);
        ctx.Check(connected && error.empty() && client.IsConnected()
                      && HasCapability(client.GetSession().capabilities,
                                       Capability::ResourcePreviews),
                  "preview-provider presence is negotiated as a session capability");
        if (!connected)
        {
            ctx.Fail("the remaining preview checks require a connected client");
            agent->Stop();
            return;
        }

        PreviewResponse response;
        error.clear();
        const PreviewRequest request{17, 64, 64, 4096};
        const bool requested = client.RequestPreview(request, response, error);
        ctx.Check(requested && error.empty() && response.status == PreviewStatus::Pending
                      && response.ticket == 42 && response.message == "readback pending"
                      && previews->requestCalls.load() == 1
                      && previews->lastResourceId.load() == 17
                      && previews->lastMaximumWidth.load() == 64
                      && previews->lastMaximumHeight.load() == 64
                      && previews->lastMaximumBytes.load() == 4096,
                  "explicit texture request reaches the provider with exact negotiated bounds");
        ctx.Check(diagnostics.snapshotCalls.load() == 2,
                  "preview request checks current resource metadata after handshake");

        error.clear();
        ctx.Check(!client.RequestPreview({17, 32, 32, 2048}, response, error)
                      && !error.empty() && previews->requestCalls.load() == 1 && client.IsConnected(),
                  "pending-preview bound applies before invoking the provider again");

        error.clear();
        const bool firstPoll = client.PollPreview({42}, response, error);
        ctx.Check(firstPoll && error.empty() && response.status == PreviewStatus::Pending
                      && response.ticket == 42 && response.message == "readback still pending"
                      && previews->pollCalls.load() == 1,
                  "provider can keep the same asynchronous ticket pending across a poll");

        error.clear();
        const bool polled = client.PollPreview({response.ticket}, response, error);
        ctx.Check(polled && error.empty() && response.status == PreviewStatus::Ready
                      && response.ticket == 42 && response.width == 2 && response.height == 1
                      && response.mimeType == "image/png"
                      && response.encodedImage
                          == std::vector<std::uint8_t>({0x89, 0x50, 0x4E, 0x47})
                      && response.message == "readback complete"
                      && previews->pollCalls.load() == 2 && previews->lastPollTicket.load() == 42,
                  "poll returns the provider's bounded ready image and stable ticket");

        error.clear();
        ctx.Check(!client.PollPreview({42}, response, error) && !error.empty()
                      && previews->pollCalls.load() == 2 && client.IsConnected(),
                  "completed ticket is retired without invoking the provider again or disconnecting");

        error.clear();
        ctx.Check(!client.RequestPreview({17, 64, 64, 4096}, response, error)
                      && !error.empty() && previews->requestCalls.load() == 1 && client.IsConnected(),
                  "preview cooldown reports busy after completion without invoking the provider");

        error.clear();
        ctx.Check(!client.RequestPreview({0, 64, 64, 4096}, response, error)
                      && !error.empty() && previews->requestCalls.load() == 1 && client.IsConnected(),
                  "invalid resource identity is refused before preview-provider access");

        client.Disconnect();
        agent->Stop();

        previews->returnOversizedImage = true;
        previews->pendingPolls = 0;
        AgentConfiguration invalidConfiguration = agentConfiguration;
        invalidConfiguration.authenticationToken = "invalid-preview-token";
        error.clear();
        auto invalidAgent = Agent::Start(diagnostics, invalidConfiguration, error);
        Client invalidClient;
        bool invalidOutputContained = false;
        if (invalidAgent)
        {
            ClientConfiguration invalidClientConfiguration;
            invalidClientConfiguration.host = invalidAgent->GetBindAddress();
            invalidClientConfiguration.port = invalidAgent->GetPort();
            invalidClientConfiguration.authenticationToken = invalidAgent->GetAuthenticationToken();
            invalidClientConfiguration.timeoutMilliseconds = 2000;
            error.clear();
            if (invalidClient.Connect(invalidClientConfiguration, error))
            {
                error.clear();
                invalidOutputContained = !invalidClient.RequestPreview(
                    {17, 1, 1, 4096}, response, error)
                    && !error.empty() && previews->requestCalls.load() == 2
                    && invalidClient.IsConnected();
            }
        }
        ctx.Check(invalidOutputContained,
                  "provider image exceeding the caller's dimensions fails without breaking the session");
        error.clear();
        ctx.Check(invalidClient.Ping(error) && error.empty(),
                  "agent remains responsive after rejecting invalid provider output");

        invalidClient.Disconnect();
        if (invalidAgent) invalidAgent->Stop();
        ctx.Check(!agent->IsRunning() && invalidAgent && !invalidAgent->IsRunning(),
                  "both preview sessions shut down cleanly");
        ctx.Print("Preview ticket 42 completed as {}x{} {} ({} bytes)",
                  2, 1, "image/png", 4);
    }
}

CNA_DEMOS_CONSOLE(PreviewPipeline, {
    .id = "inspector.agent.preview_pipeline",
    .title = "Inspector preview pipeline",
    .description = "Negotiates an explicit preview provider, polls one asynchronous texture readback and enforces caller bounds.",
    .apis = {"CNA::Inspector::IResourcePreviewProvider::RequestPreview",
             "CNA::Inspector::IResourcePreviewProvider::PollPreview",
             "CNA::Inspector::AgentConfiguration::previewProvider",
             "CNA::Inspector::AgentConfiguration::maximumPendingPreviews",
             "CNA::Inspector::AgentConfiguration::previewCooldownMilliseconds",
             "CNA::Inspector::Client::RequestPreview", "CNA::Inspector::Client::PollPreview",
             "CNA::Inspector::PreviewRequest", "CNA::Inspector::PreviewPollRequest",
             "CNA::Inspector::PreviewResponse", "CNA::Inspector::PreviewStatus"},
    .tags = {"diagnostics", "inspector", "preview", "resource", "asynchronous", "bounds"},
    .needs = CnaDemos::Need::Network,
})
