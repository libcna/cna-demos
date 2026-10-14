// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "net.network_session.find_and_join",
    .title = "Find and join a System Link game",
    .description = "Two processes on one machine: the first hosts a System Link session, the second finds it with "
                   "NetworkSession::Find (host gamertag, public/private slots, sparse properties and "
                   "QualityOfService), joins it, disposes the search results and leaves; the host sees GamerJoined "
                   "and GamerLeft.",
    .apis = {"NetworkSession::Find", "NetworkSession::Join", "AvailableNetworkSessionCollection",
             "AvailableNetworkSessionCollection::Dispose", "AvailableNetworkSessionCollection::IsDisposed",
             "AvailableNetworkSession::HostGamertag",
             "AvailableNetworkSession::CurrentGamerCount", "AvailableNetworkSession::OpenPublicGamerSlots",
             "AvailableNetworkSession::OpenPrivateGamerSlots", "AvailableNetworkSession::SessionProperties",
             "AvailableNetworkSession::QualityOfService", "QualityOfService::AverageRoundtripTime",
             "QualityOfService::MinimumRoundtripTime", "QualityOfService::IsAvailable", "NetworkSession::GamerLeft",
             "GamerLeftEventArgs::Gamer", "NetworkSession::GamerJoined", "NetworkSession::Create",
             "NetworkSessionType::SystemLink"},
    .tags = {"net", "multiplayer", "system-link", "lan", "lobby"},
    .needs = CnaDemos::Need::Network,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Standalone: a process holds one NetworkSession, so the joiner is a second process (this program "
                    "started again with --joiner). Discovery uses UDP on the local network; a firewall that blocks "
                    "it makes the demo fail.",
})
