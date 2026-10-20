// SPDX-License-Identifier: MS-PL
// ISSUE-012: no Game, no routing opt-in, and no manual built-in registration.
#include <gtest/gtest.h>
#include <array>
#include <functional>
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ResourceContentManager.hpp"
#include "Microsoft/Xna/Framework/Curve.hpp"
#include "System/IO/BinaryWriter.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/Resources/ResourceManager.hpp"

using namespace Microsoft::Xna::Framework::Content;
namespace
{
    struct StreamPayload { int value = 0; };
    using Payload = std::shared_ptr<StreamPayload>;
    class StreamPayloadReader final : public ContentTypeReader<Payload>
    {
    public:
        StreamPayloadReader() : ContentTypeReader<Payload>("Repair.StreamPayload") { }
    protected:
        Payload Read(ContentReader& input, std::optional<Payload>) override
        {
            return std::make_shared<StreamPayload>(StreamPayload{input.ReadInt32()});
        }
    };
    std::vector<std::uint8_t> Xnb(const std::string& readerName,
                                const std::function<void(System::IO::BinaryWriter&)>& payload)
    {
        System::IO::MemoryStream body;
        System::IO::BinaryWriter writer(&body, true);
        writer.Write7BitEncodedInt(1);
        writer.Write(readerName); writer.Write(std::int32_t{0});
        writer.Write7BitEncodedInt(0);
        writer.Write7BitEncodedInt(1);
        payload(writer);
        writer.Flush();
        const auto data = body.ToArray();
        System::IO::MemoryStream file;
        System::IO::BinaryWriter header(&file, true);
        for (const auto byte : std::array<std::uint8_t, 6>{'X', 'N', 'B', 'w', 5, 0}) header.Write(byte);
        header.Write(static_cast<std::int32_t>(data.size() + 10));
        for (const auto byte : data) header.Write(byte);
        header.Flush();
        const auto result = file.ToArray();
        return {result.begin(), result.end()};
    }
    class StreamManager final : public ContentManager
    {
    public:
        using ContentManager::ReadAsset;
        int opens = 0;
        bool fail = false;
        StreamManager() : ContentManager(nullptr, "repair-no-disk-stream-assets"),
            bytes_(Xnb("Repair.StreamPayloadReader", [](auto& writer) { writer.Write(std::int32_t{42}); })) { }
    protected:
        std::unique_ptr<System::IO::Stream> OpenStream(const std::string&) override
        {
            ++opens;
            if (fail) throw System::InvalidOperationException("stream override refusal");
            return std::make_unique<System::IO::MemoryStream>(bytes_.data(), static_cast<std::int32_t>(bytes_.size()), false);
        }
    private:
        std::vector<std::uint8_t> bytes_;
    };
    class ContentStreamRoutingTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            ContentTypeReaderManager::ClearTypeCreators();
            ContentTypeReaderManager::ClearTargetTypeAssociationsEXT();
            ContentTypeReaderManager::AddTypeCreator("Repair.StreamPayloadReader", [] { return std::make_unique<StreamPayloadReader>(); });
        }
        void TearDown() override
        {
            ContentTypeReaderManager::ClearTypeCreators();
            ContentTypeReaderManager::ClearTargetTypeAssociationsEXT();
        }
    };

TEST_F(ContentStreamRoutingTest, DirectReadAssetAlreadyUsesTheVirtualStream)
{
    StreamManager manager;
    const auto value = manager.ReadAsset<Payload>("asset", {});
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->value, 42);
    EXPECT_EQ(manager.opens, 1);
    manager.fail = true;
    EXPECT_THROW((void)manager.ReadAsset<Payload>("other", {}), System::InvalidOperationException);
    EXPECT_EQ(manager.opens, 2);
}

TEST_F(ContentStreamRoutingTest, OrdinaryLoadUsesTheVirtualStreamAndCachesItsInstance)
{
    StreamManager manager;
    Payload value;
    ASSERT_NO_THROW(value = manager.Load<Payload>("asset"));
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->value, 42);
    EXPECT_EQ(manager.opens, 1);
    EXPECT_EQ(manager.Load<Payload>("asset"), value);
    EXPECT_EQ(manager.opens, 1);
}

TEST_F(ContentStreamRoutingTest, OrdinaryLoadPreservesTheOverrideException)
{
    StreamManager manager;
    manager.fail = true;
    EXPECT_THROW((void)manager.Load<Payload>("asset"), System::InvalidOperationException);
    EXPECT_EQ(manager.opens, 1);
}

TEST_F(ContentStreamRoutingTest, ColdResourceCurveLoadsAndExplicitWithdrawalRemainsEffective)
{
    using Microsoft::Xna::Framework::Curve;
    const auto bytes = Xnb("Microsoft.Xna.Framework.Content.CurveReader", [](auto& writer) {
        writer.Write(std::int32_t{0}); writer.Write(std::int32_t{0}); // Constant loops.
        writer.Write(std::int32_t{1});
        writer.Write(0.0f); writer.Write(42.0f); writer.Write(0.0f); writer.Write(0.0f);
        writer.Write(std::int32_t{0}); // Smooth continuity.
    });
    int lookups = 0;
    System::Resources::ResourceManager resources("Repair.EmbeddedCurve",
        [](std::string_view, std::string_view, std::string_view) -> std::optional<std::string> { return std::nullopt; },
        [&](std::string_view, std::string_view, std::string_view name) -> std::optional<std::any> {
            ++lookups;
            return name == "curve" ? std::optional<std::any>(bytes) : std::nullopt;
        });
    ResourceContentManager manager(nullptr, &resources);
    std::optional<Curve> curve;
    ASSERT_NO_THROW(curve.emplace(manager.Load<Curve>("curve")));
    ASSERT_TRUE(curve.has_value());
    EXPECT_EQ(curve->getKeysProperty().getCountProperty(), 1);
    EXPECT_EQ(curve->Evaluate(0.0f), 42.0f);
    EXPECT_EQ(manager.Load<Curve>("curve").Evaluate(0.0f), 42.0f);
    EXPECT_EQ(lookups, 1);
    ASSERT_TRUE(ContentTypeReaderManager::RemoveTypeCreatorEXT("Microsoft.Xna.Framework.Content.CurveReader"));
    manager.Unload();
    EXPECT_THROW((void)manager.Load<Curve>("curve"), ContentLoadException);
    EXPECT_THROW((void)manager.Load<Curve>("missing"), ContentLoadException);
}
}
