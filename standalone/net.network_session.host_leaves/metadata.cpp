// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "net.network_session.host_leaves",
    .title = "When the host leaves",
    .description = "The host of a System Link session leaves once a player (a second process) has joined -- "
                   "twice. Without host migration the player's session ends (SessionEnded, HostEndedSession); with "
                   "AllowHostMigration the player becomes the host (HostChanged, GamerLeft) and the session goes on.",
    .apis = {"NetworkSession::AllowHostMigration", "NetworkSession::HostChanged", "HostChangedEventArgs::OldHost",
             "HostChangedEventArgs::NewHost", "NetworkSession::SessionEnded",
             "NetworkSessionEndedEventArgs::EndReason", "NetworkSessionEndReason", "NetworkSession::GamerLeft",
             "GamerLeftEventArgs::Gamer", "NetworkSession::IsHost", "NetworkSession::Find", "NetworkSession::Join",
             "NetworkSessionProperties"},
    .tags = {"net", "multiplayer", "system-link", "host-migration"},
    .needs = CnaDemos::Need::Network,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Standalone: a process holds one NetworkSession, so each player is a second process (this "
                    "program started again with --player). Discovery uses UDP on the local network.",
})
