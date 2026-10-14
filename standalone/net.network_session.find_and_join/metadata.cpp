// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "net.network_session.find_and_join",
    .title = "Find and join a System Link game",
    .description = "Two processes on one machine: the first hosts a System Link session, the second finds and joins "
                   "it with the Begin/End pairs (host gamertag, public/private slots, sparse properties and "
                   "QualityOfService), disposes the search results and leaves; the host sees GamerJoined and "
                   "GamerLeft, then retains the departed gamer in PreviousGamers.",
    .apis = {"NetworkSession::BeginFind", "NetworkSession::EndFind", "NetworkSession::BeginJoin",
             "NetworkSession::EndJoin", "NetworkSession::Join", "AvailableNetworkSessionCollection",
             "AvailableNetworkSessionCollection::Dispose", "AvailableNetworkSessionCollection::IsDisposed",
             "AvailableNetworkSession::HostGamertag",
             "AvailableNetworkSession::CurrentGamerCount", "AvailableNetworkSession::OpenPublicGamerSlots",
             "AvailableNetworkSession::OpenPrivateGamerSlots", "AvailableNetworkSession::SessionProperties",
             "AvailableNetworkSession::QualityOfService", "QualityOfService::AverageRoundtripTime",
             "QualityOfService::MinimumRoundtripTime", "QualityOfService::IsAvailable",
             "QualityOfService::BytesPerSecondDownstream", "QualityOfService::BytesPerSecondUpstream",
             "NetworkSession::GamerLeft",
             "GamerLeftEventArgs::Gamer", "NetworkSession::GamerJoined", "NetworkSession::Create",
             "NetworkGamer::HasLeftSession", "NetworkGamer::RoundtripTime", "NetworkGamer::Session",
             "NetworkSession::PreviousGamers",
             "NetworkSessionType::SystemLink"},
    .tags = {"net", "multiplayer", "system-link", "lan", "lobby"},
    .needs = CnaDemos::Need::Network,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Standalone: a process holds one NetworkSession, so the joiner is a second process (this program "
                    "started again with --joiner). Discovery uses UDP on the local network; a firewall that blocks "
                    "it makes the demo fail.",
})
