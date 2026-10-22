// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "net.network_session.join_refused",
    .title = "A join refused: game in progress",
    .description = "Two processes: the second finds a System Link session in its lobby, the host starts the game, "
                   "and NetworkSession::Join fails with a NetworkSessionJoinException whose JoinError is "
                   "SessionNotJoinable; once the host sets AllowJoinInProgress, the second Join enters the game.",
    .apis = {"NetworkSessionJoinException", "NetworkSessionJoinException::JoinError",
             "NetworkSessionJoinException::GetObjectData", "NetworkSessionJoinError",
             "NetworkSessionJoinError::SessionNotFound", "NetworkSessionJoinError::SessionNotJoinable",
             "NetworkSession::AllowJoinInProgress", "NetworkSession::StartGame", "NetworkSession::SessionState",
             "NetworkSession::Join", "NetworkSession::Find"},
    .tags = {"net", "multiplayer", "system-link", "lobby", "error", "join-in-progress"},
    .needs = CnaDemos::Need::Network,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Standalone: a process holds one NetworkSession, so the joiner is a second process (this program "
                    "started again with --joiner); the two signal each step through files in a temporary directory. "
                    "Discovery uses UDP on the local network; a firewall that blocks it makes the demo fail. "
                    "The refusal and late-join Playing assertions protect resolved CNA-DEMOS-ISSUE-032 and "
                    "CNA-DEMOS-ISSUE-033.",
})
