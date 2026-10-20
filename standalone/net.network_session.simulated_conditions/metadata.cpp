// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Demo.hpp"

CNA_DEMOS_STANDALONE({
    .id = "net.network_session.simulated_conditions",
    .title = "Simulated latency and packet loss",
    .description = "A second process sends finite unreliable and reliable bursts through a baseline, "
                   "200 ms mean SimulatedLatency, and a complete SimulatedPacketLoss outage followed by recovery: "
                   "exact delivery counts distinguish dropped packets from delayed ones while the host samples its "
                   "measured sent and received bandwidth.",
    .apis = {"NetworkSession::SimulatedLatency", "NetworkSession::SimulatedPacketLoss", "SendDataOptions::None",
             "SendDataOptions::Reliable", "LocalNetworkGamer::SendData", "LocalNetworkGamer::ReceiveData",
             "LocalNetworkGamer::IsDataAvailable", "PacketWriter::PacketWriter", "PacketReader::PacketReader",
             "NetworkSession::BytesPerSecondSent", "NetworkSession::BytesPerSecondReceived"},
    .tags = {"net", "multiplayer", "system-link", "latency", "packet-loss", "testing"},
    .needs = CnaDemos::Need::Network,
    .assets = {"fonts/pixel_font.cnb"},
    .restrictions = "Standalone: a process holds one NetworkSession, so the sender is a second process (this "
                    "program started again with --sender). Discovery uses UDP on the local network; "
                    "temporary files coordinate local test phases independently of the impaired connection.",
})
