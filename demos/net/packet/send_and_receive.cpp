// SPDX-License-Identifier: MS-PL
// PacketWriter, LocalNetworkGamer::SendData / ReceiveData and PacketReader in a System Link session.
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CnaDemos/Demo.hpp"
#include "CnaDemos/GamerServicesDemo.hpp"
#include "Microsoft/Xna/Framework/Color.hpp"
#include "Microsoft/Xna/Framework/GamerServices/Gamer.hpp"
#include "Microsoft/Xna/Framework/GamerServices/NetworkNotAvailableException.hpp"
#include "Microsoft/Xna/Framework/GamerServices/SignedInGamerCollection.hpp"
#include "Microsoft/Xna/Framework/Graphics/SamplerState.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteBatch.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteFont.hpp"
#include "Microsoft/Xna/Framework/Graphics/SpriteSortMode.hpp"
#include "Microsoft/Xna/Framework/Matrix.hpp"
#include "Microsoft/Xna/Framework/Net/LocalNetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkGamer.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSession.hpp"
#include "Microsoft/Xna/Framework/Net/NetworkSessionType.hpp"
#include "Microsoft/Xna/Framework/Net/PacketReader.hpp"
#include "Microsoft/Xna/Framework/Net/PacketWriter.hpp"
#include "Microsoft/Xna/Framework/Net/SendDataOptions.hpp"
#include "Microsoft/Xna/Framework/Quaternion.hpp"
#include "Microsoft/Xna/Framework/Vector2.hpp"
#include "Microsoft/Xna/Framework/Vector3.hpp"
#include "Microsoft/Xna/Framework/Vector4.hpp"

using namespace Microsoft::Xna::Framework;
using namespace Microsoft::Xna::Framework::GamerServices;
using namespace Microsoft::Xna::Framework::Graphics;
using namespace Microsoft::Xna::Framework::Net;

namespace
{
    class SendAndReceiveDemo final : public CnaDemos::GamerServicesDemo
    {
    public:
        SendAndReceiveDemo() : GamerServicesDemo("Demo Player,Second Player") {}

    protected:
        void LoadContent() override
        {
            spriteBatch_ = std::make_unique<SpriteBatch>(getGraphicsDeviceProperty());
            font_.emplace(getContentProperty().Load<SpriteFont>("fonts/pixel_font"));
        }

        void UnloadContent() override
        {
            if (session_)
            {
                session_->Dispose();
                session_.reset();
            }
        }

        void Update(GameTime& gameTime) override
        {
            DemoGame::Update(gameTime);
            auto& ctx = Context();
            if (!session_)
            {
                if (Gamer::getSignedInGamersProperty()->getCountProperty() < 2)
                {
                    if (++waited_ == 30)
                    {
                        throw CnaDemos::SkipDemo("two signed-in players are needed (CNA_GAMER_SERVICES_AUTO_SIGN_IN)");
                    }
                    return;
                }
                // A System Link session is a real network session on the local network (it opens
                // UDP sockets); other machines could find and join it. Both players are local here,
                // so packets between them never leave this machine.
                try
                {
                    session_.reset(NetworkSession::Create(NetworkSessionType::SystemLink, 2, 4));
                }
                catch (const NetworkNotAvailableException& unavailable)
                {
                    // XNA's unavailable-network exception matches this demo's declared Need::Network.
                    throw CnaDemos::SkipDemo(std::string("no network: ") + unavailable.what());
                }
            }
            session_->Update();
            LocalNetworkGamer* sender = session_->getLocalGamersProperty()[0];
            LocalNetworkGamer* receiver = session_->getLocalGamersProperty()[1];

            if (++updates_ == 5)
            {
                // A typical state packet: what changed this frame, in a fixed order.
                PacketWriter writer;
                writer.Write(0);                              // payload kind: complete state
                writer.Write(Vector3(12.5f, 0.0f, -3.25f)); // position
                writer.Write(1.5f);                       // heading in radians
                writer.Write(42);                         // score
                writer.Write(std::string("jump"));        // last action
                writer.Write(Color::Orange);              // team colour
                writer.Write(123456.25);                  // authoritative simulation time
                writer.Write(Vector2(-2.5f, 0.75f));      // planar velocity
                writer.Write(Vector4(0.25f, 0.5f, 0.75f, 1.0f)); // material tint
                writer.Write(Quaternion(0.0f, 0.0f, 0.0f, 1.0f)); // orientation
                writer.Write(Matrix::CreateTranslation(10.0f, 20.0f, -30.0f)); // world transform
                ctx.Print("packet: {} bytes", writer.getLengthProperty());
                // To one gamer, reliably and in order (SendDataOptions say what the game needs).
                sender->SendData(writer, SendDataOptions::ReliableInOrder, receiver);
                // XNA sends the bytes before Position and rewinds the writer: the same writer is
                // reused for every packet, its old bytes overwritten.
                ctx.Check(writer.getPositionProperty() == 0, "SendData rewinds the writer for the next packet");

                const std::array modes = {
                    SendDataOptions::InOrder, SendDataOptions::ReliableInOrder, SendDataOptions::Chat};
                for (std::size_t i = 0; i < modes.size(); ++i)
                {
                    PacketWriter marker;
                    marker.Write(static_cast<int>(i + 1));
                    marker.Write("mode-" + std::to_string(i + 1));
                    sender->SendData(marker, modes[i], receiver);
                }
            }

            // Everyone drains their incoming packets each frame.
            while (receiver->getIsDataAvailableProperty())
            {
                PacketReader reader;
                NetworkGamer* from = nullptr;
                const int bytes = receiver->ReceiveData(reader, from);
                const int kind = reader.ReadInt32();
                if (kind != 0)
                {
                    if (kind >= 1 && kind <= static_cast<int>(receivedModes_.size()))
                    {
                        receivedModes_[static_cast<std::size_t>(kind - 1)] = reader.ReadString();
                    }
                    ctx.Check(from == sender && reader.getPositionProperty() == reader.getLengthProperty(),
                              "the option-specific marker arrives intact from the selected sender");
                    continue;
                }
                const Vector3 position = reader.ReadVector3();
                const float heading = reader.ReadSingle();
                const int score = reader.ReadInt32();
                const std::string action = reader.ReadString();
                const Color team = reader.ReadColor();
                const double simulationTime = reader.ReadDouble();
                const Vector2 velocity = reader.ReadVector2();
                const Vector4 tint = reader.ReadVector4();
                const Quaternion orientation = reader.ReadQuaternion();
                const Matrix world = reader.ReadMatrix();
                received_ = {from->getGamertagProperty() + " -> " + receiver->getGamertagProperty() + ", " +
                                 std::to_string(bytes) + " bytes",
                             "position (" + std::to_string(position.X) + ", " + std::to_string(position.Y) + ", " +
                                 std::to_string(position.Z) + ")",
                             "heading " + std::to_string(heading) + ", score " + std::to_string(score),
                             "action \"" + action + "\", team colour"};
                for (const std::string& line : received_)
                {
                    ctx.Print("{}", line);
                }
                ctx.Check(from == sender, "ReceiveData names the sender");
                ctx.Check(position == Vector3(12.5f, 0.0f, -3.25f) && heading == 1.5f && score == 42 &&
                              action == "jump" && team == Color::Orange,
                          "every value reads back exactly, in the order written");
                ctx.Check(simulationTime == 123456.25 && velocity == Vector2(-2.5f, 0.75f) &&
                              tint == Vector4(0.25f, 0.5f, 0.75f, 1.0f) &&
                              orientation == Quaternion(0.0f, 0.0f, 0.0f, 1.0f) &&
                              world == Matrix::CreateTranslation(10.0f, 20.0f, -30.0f),
                          "double, Vector2, Vector4, Quaternion and Matrix payloads round-trip exactly");
                ctx.Check(reader.getPositionProperty() == reader.getLengthProperty(), "the whole packet was read");
                teamColor_ = team;
            }
        }

        void EndRun() override
        {
            if (Context().Automated())
            {
                Context().Check(!received_.empty(), "the packet arrived");
                Context().Check(receivedModes_ == std::array<std::string, 3>{"mode-1", "mode-2", "mode-3"},
                                "InOrder, ReliableInOrder and Chat packets all reach the local recipient");
            }
            DemoGame::EndRun();
        }

        void Draw(const GameTime& gameTime) override
        {
            getGraphicsDeviceProperty().Clear(Color(28, 34, 46));
            spriteBatch_->Begin(SpriteSortMode::Deferred, nullptr, &SamplerState::PointClamp, nullptr, nullptr);
            spriteBatch_->DrawString(*font_, "PacketWriter -> SendData -> ReceiveData -> PacketReader",
                                     Vector2(20.0f, 30.0f), Color::White);
            float y = 80.0f;
            if (received_.empty())
            {
                spriteBatch_->DrawString(*font_, "waiting...", Vector2(20.0f, y), Color::LightGray);
            }
            for (const std::string& line : received_)
            {
                spriteBatch_->DrawString(*font_, line, Vector2(20.0f, y), teamColor_);
                y += 28.0f;
            }
            spriteBatch_->End();
            DemoGame::Draw(gameTime);
        }

    private:
        std::unique_ptr<NetworkSession> session_;
        int waited_ = 0;
        int updates_ = 0;
        std::vector<std::string> received_;
        std::array<std::string, 3> receivedModes_;
        Color teamColor_ = Color::LightGray;
        std::unique_ptr<SpriteBatch> spriteBatch_;
        std::optional<SpriteFont> font_;
    };
}

CNA_DEMOS_GAME(SendAndReceiveDemo, {
    .id = "net.packet.send_and_receive",
    .title = "Sending game data",
    .description = "In a System Link session of two local players, the first writes a state packet with "
                   "PacketWriter (scalar, colour, vector, quaternion and matrix state) and sends it reliably in order to "
                   "the second, who receives it with ReceiveData and reads the values back with PacketReader.",
    .apis = {"PacketWriter::PacketWriter", "PacketWriter::Write", "PacketWriter::Length", "PacketWriter::Position",
             "LocalNetworkGamer::SendData", "LocalNetworkGamer::ReceiveData", "LocalNetworkGamer::IsDataAvailable",
             "PacketReader::PacketReader", "PacketReader::ReadVector3", "PacketReader::ReadColor",
             "PacketReader::Position", "PacketReader::Length", "SendDataOptions", "NetworkSessionType",
             "SendDataOptions::InOrder", "SendDataOptions::ReliableInOrder", "SendDataOptions::Chat",
             "PacketReader::ReadSingle", "PacketReader::ReadDouble", "PacketReader::ReadVector2",
             "PacketReader::ReadVector4", "PacketReader::ReadQuaternion", "PacketReader::ReadMatrix"},
    .tags = {"net", "packet", "serialization", "system-link", "multiplayer"},
    .needs = CnaDemos::Need::Network,
    .frames = 30,
    .assets = {"fonts/pixel_font.cnb"},
    .ownProcess = true,
})
