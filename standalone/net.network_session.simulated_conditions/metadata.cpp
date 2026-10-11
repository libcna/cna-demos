// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "net.network_session.simulated_conditions",
    .title = "Simulated latency and packet loss",
    .description = "A second process streams unreliable and reliable packets to the host, which receives them as "
                   "they are, then with NetworkSession::SimulatedLatency (200 ms) and SimulatedPacketLoss (0.3): "
                   "measured delay, unreliable packets lost, reliable ones delivered.",
    .apis = {"NetworkSession::SimulatedLatency", "NetworkSession::SimulatedPacketLoss", "SendDataOptions::None",
             "SendDataOptions::Reliable", "LocalNetworkGamer::SendData", "LocalNetworkGamer::ReceiveData",
             "LocalNetworkGamer::IsDataAvailable", "PacketWriter::PacketWriter", "PacketReader::PacketReader"},
    .tags = {"net", "multiplayer", "system-link", "latency", "packet-loss", "testing"},
    .needs = CnaDemos::Need::Network,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Standalone: a process holds one NetworkSession, so the sender is a second process (this "
                    "program started again with --sender). Discovery uses UDP on the local network.",
    .knownIssue = "CNA-DEMOS-ISSUE-030",
})
