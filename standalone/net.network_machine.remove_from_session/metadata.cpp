// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "net.network_machine.remove_from_session",
    .title = "Remove a player (kick)",
    .description = "The host of a System Link session removes a player's machine with "
                   "NetworkMachine::RemoveFromSession: the host sees GamerLeft, the player (a second process) sees "
                   "its session end with NetworkSessionEndReason::RemovedByHost. Only the host may do it.",
    .apis = {"NetworkMachine::RemoveFromSession", "NetworkGamer::Machine",
             "NetworkSessionEndReason::RemovedByHost", "NetworkSession::SessionEnded", "NetworkSession::GamerLeft",
             "NetworkSession::RemoteGamers"},
    .tags = {"net", "multiplayer", "system-link", "kick", "moderation"},
    .needs = CnaDemos::Need::Network,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Standalone: a process holds one NetworkSession, so the player is a second process (this "
                    "program started again with --player). Discovery uses UDP on the local network.",
})
