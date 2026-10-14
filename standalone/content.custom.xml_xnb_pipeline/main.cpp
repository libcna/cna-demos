// SPDX-License-Identifier: MS-PL
// Original XML authoring -> the real source pipeline -> custom XNB writer -> runtime reader.
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>

#include "CNA/Content/Pipeline/ContentBuildManifest.hpp"
#include "CNA/Content/Pipeline/ContentCompiler.hpp"
#include "CNA/Content/Pipeline/XnaXmlSourceContentPipeline.hpp"
#include "Microsoft/Xna/Framework/Content/ContentManager.hpp"
#include "Microsoft/Xna/Framework/Content/ContentReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReader.hpp"
#include "Microsoft/Xna/Framework/Content/ContentTypeReaderManager.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Compiler/ContentWriter.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"

namespace Pipeline = CNA::Content::Pipeline;
namespace Xna = Microsoft::Xna::Framework::Content::Pipeline;
namespace Compiler = Xna::Serialization::Compiler;
namespace Intermediate = Xna::Serialization::Intermediate;
namespace Runtime = Microsoft::Xna::Framework::Content;
namespace
{
    constexpr const char* ReaderName = "CnaDemos.XmlProfileReader";
    std::atomic<int> writes{0};
    std::atomic<int> initializations{0};
    std::atomic<int> registrationContracts{0};
    std::atomic<int> initializedWrites{0};
    std::atomic<int> writerContexts{0};
    std::atomic<int> compressionChecks{0};
    std::atomic<int> compressionPlatforms{0};
    int reads = 0;
    struct Vital
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.XmlVital";
        int health = 0;
    };
    struct BadgeSource : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.BadgeSource";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName); return name;
        }
        std::string label;
        int level = 0;
        static void DescribeContent(Intermediate::ContentTypeDescriptor<BadgeSource>& type)
        {
            type.Field("Label", &BadgeSource::label);
            type.Field("Level", &BadgeSource::level);
        }
    };
    struct RareBadgeSource final : BadgeSource
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.RareBadgeSource";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName); return name;
        }
        int glow = 0;
        static void DescribeContent(Intermediate::ContentTypeDescriptor<RareBadgeSource>& type)
        {
            type.BaseType<BadgeSource>();
            type.Field("Glow", &RareBadgeSource::glow);
        }
    };
    struct ProfileSource : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.ProfileSource";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName); return name;
        }
        std::string name;
        int health = 0;
        std::shared_ptr<BadgeSource> badge;
        std::string portrait;
        static void DescribeContent(Intermediate::ContentTypeDescriptor<ProfileSource>& type)
        {
            type.Field("Name", &ProfileSource::name);
            type.Field("Health", &ProfileSource::health);
            type.Field("Badge", &ProfileSource::badge);
            type.Field("Portrait", &ProfileSource::portrait);
        }
    };
    // Runtime data has no dependency on XML descriptors or the build-time Object hierarchy.
    struct Badge
    {
        virtual ~Badge() = default;
        std::string label;
        int level = 0;
    };
    struct RareBadge final : Badge { int glow = 0; };
    struct Profile
    {
        std::string name;
        int health = 0;
        int rawHealth = 0;
        std::shared_ptr<Badge> inlineBadge;
        std::shared_ptr<Badge> sharedBadgeA;
        std::shared_ptr<Badge> sharedBadgeB;
        std::string portrait;
    };
    class VitalWriter final : public Compiler::ContentTypeWriter<Vital>
    {
    public:
        std::string GetRuntimeReader(Xna::TargetPlatform) const override { return "CnaDemos.XmlVitalReader"; }
    protected:
        void Write(Compiler::ContentWriter& output, const Vital& value) override { output.Write(value.health); }
    };
    class BadgeWriter final : public Compiler::ContentTypeWriter<BadgeSource>
    {
    public:
        std::string GetRuntimeReader(Xna::TargetPlatform) const override { return "CnaDemos.XmlBadgeReader"; }
        std::string GetRuntimeType(Xna::TargetPlatform) const override { return "CnaDemos.XmlBadge"; }
    protected:
        void Write(Compiler::ContentWriter& output, const TargetCarrier& value) override
        {
            output.Write(value->label);
            output.Write(value->level);
        }
    };
    class RareBadgeWriter final : public Compiler::ContentTypeWriter<RareBadgeSource>
    {
    public:
        std::string GetRuntimeReader(Xna::TargetPlatform) const override { return "CnaDemos.XmlRareBadgeReader"; }
        std::string GetRuntimeType(Xna::TargetPlatform) const override { return "CnaDemos.XmlRareBadge"; }
    protected:
        void Write(Compiler::ContentWriter& output, const TargetCarrier& value) override
        {
            output.Write(value->label);
            output.Write(value->level);
            output.Write(value->glow);
        }
    };
    class ProfileWriter final : public Compiler::ContentTypeWriter<ProfileSource>
    {
    public:
        std::string GetRuntimeReader(Xna::TargetPlatform) const override { return ReaderName; }
        std::string GetRuntimeType(Xna::TargetPlatform) const override { return "CnaDemos.XmlProfile"; }
        std::int32_t getTypeVersionProperty() const override { return 2; }
    protected:
        void Initialize(Compiler::ContentCompiler& compiler) override
        {
            ++initializations;
            compiler_ = &compiler;
        }
        bool ShouldCompressContent(Xna::TargetPlatform platform, const Xna::ContentObject&) const override
        {
            ++compressionChecks;
            if (platform == Xna::TargetPlatform::WindowsPhone) ++compressionPlatforms;
            return false;
        }
        void Write(Compiler::ContentWriter& output, const TargetCarrier& value) override
        {
            ++writes;
            if (!compiler_) throw std::runtime_error("ProfileWriter::Initialize did not supply its compiler.");
            // Resolving here preserves the workflow while ISSUE-090 keeps the documented
            // Initialize-time caching attempt as a separate visible deadlock reproduction.
            const auto vitalWriter = compiler_->GetTypeWriter(System::Type::From<Vital>());
            ++initializedWrites;
            if (output.getTargetPlatformProperty() == Xna::TargetPlatform::WindowsPhone &&
                output.getTargetProfileProperty() == Microsoft::Xna::Framework::Graphics::GraphicsProfile::HiDef)
                ++writerContexts;
            output.Write(value->name);
            output.WriteObject<Vital>(Vital{value->health}, *vitalWriter);
            output.WriteRawObject<Vital>(Vital{value->health + 1}, *vitalWriter);
            output.WriteObject<BadgeSource>(value->badge);
            output.WriteSharedResource<BadgeSource>(value->badge);
            output.WriteSharedResource<BadgeSource>(value->badge);
            output.WriteExternalReference(Xna::ExternalReference<BadgeSource>(value->portrait));
        }
    private:
        Compiler::ContentCompiler* compiler_ = nullptr;
    };
    class InitializeCacheProbeWriter final : public Compiler::ContentTypeWriter<ProfileSource>
    {
    public:
        std::string GetRuntimeReader(Xna::TargetPlatform) const override { return ReaderName; }
    protected:
        void Initialize(Compiler::ContentCompiler& compiler) override
        {
            // ContentTypeWriter documents Initialize as the place to retrieve and cache nested
            // writers. AddTypeWriter currently calls this while holding the same mutex that
            // GetTypeWriter needs, so the focused registration cannot return.
            nested_ = compiler.GetTypeWriter(System::Type::From<std::string>());
        }
        void Write(Compiler::ContentWriter&, const TargetCarrier&) override {}
    private:
        std::shared_ptr<Compiler::ContentTypeWriterBase> nested_;
    };
    class VitalReader final : public Runtime::ContentTypeReader<Vital>
    {
    public:
        VitalReader() : ContentTypeReader<Vital>(std::string(Vital::XnaTypeName)) {}
    protected:
        Vital Read(Runtime::ContentReader& input, std::optional<Vital>) override
        {
            return {input.ReadInt32()};
        }
    };
    class BadgeReader final : public Runtime::ContentTypeReader<std::shared_ptr<Badge>>
    {
    public:
        BadgeReader() : ContentTypeReader<std::shared_ptr<Badge>>("CnaDemos.XmlBadge") {}
    protected:
        std::shared_ptr<Badge> Read(Runtime::ContentReader& input,
                                    std::optional<std::shared_ptr<Badge>>) override
        {
            auto badge = std::make_shared<Badge>();
            badge->label = input.ReadString();
            badge->level = input.ReadInt32();
            return badge;
        }
    };
    class RareBadgeReader final : public Runtime::ContentTypeReader<std::shared_ptr<Badge>>
    {
    public:
        RareBadgeReader() : ContentTypeReader<std::shared_ptr<Badge>>("CnaDemos.XmlRareBadge") {}
    protected:
        std::shared_ptr<Badge> Read(Runtime::ContentReader& input,
                                    std::optional<std::shared_ptr<Badge>>) override
        {
            auto badge = std::make_shared<RareBadge>();
            badge->label = input.ReadString();
            badge->level = input.ReadInt32();
            badge->glow = input.ReadInt32();
            return badge;
        }
    };
    class ProfileReader final : public Runtime::ContentTypeReader<std::shared_ptr<Profile>>
    {
    public:
        ProfileReader() : ContentTypeReader<std::shared_ptr<Profile>>("CnaDemos.XmlProfile") {}
        int getTypeVersionProperty() const override { return 2; }
    protected:
        std::shared_ptr<Profile> Read(Runtime::ContentReader& input,
                                      std::optional<std::shared_ptr<Profile>>) override
        {
            ++reads;
            auto profile = std::make_shared<Profile>();
            profile->name = input.ReadString();
            profile->health = input.ReadObject<Vital>().health;
            VitalReader rawReader;
            profile->rawHealth = input.ReadRawObject<Vital>(rawReader).health;
            profile->inlineBadge = input.ReadObject<std::shared_ptr<Badge>>();
            input.ReadSharedResource<std::shared_ptr<Badge>>(
                [profile](std::shared_ptr<Badge> badge) { profile->sharedBadgeA = std::move(badge); });
            input.ReadSharedResource<std::shared_ptr<Badge>>(
                [profile](std::shared_ptr<Badge> badge) { profile->sharedBadgeB = std::move(badge); });
            profile->portrait = input.ReadString();
            return profile;
        }
    };
    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };
    void AuthorProfile(const std::filesystem::path& path, int health, const std::filesystem::path& portrait)
    {
        std::ofstream file(path);
        file << "<XnaContent xmlns:CnaDemos=\"CnaDemos\"><Asset Type=\"CnaDemos:ProfileSource\">"
             << "<Name>Sentinel</Name><Health>" << health << "</Health>"
             << "<Badge Type=\"CnaDemos:RareBadgeSource\"><Label>Founder</Label><Level>7</Level>"
             << "<Glow>11</Glow></Badge><Portrait>" << portrait.generic_string()
             << "</Portrait></Asset></XnaContent>\n";
        file.close();
        if (!file) throw std::runtime_error("Could not write original profile XML.");
    }
    std::string ReadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Could not read published compiler data.");
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
}
int main()
{
    try
    {
        bool passed = true;
        const auto check = [&](bool condition, std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = passed && condition;
        };
        const auto directory = std::filesystem::temp_directory_path() /
            ("cna-demos-xml-xnb-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        if (!std::filesystem::create_directory(directory)) throw std::runtime_error("Could not reserve a private XML workspace.");
        {
            TemporaryDirectory cleanup{directory};
            std::cout << "Private XML compiler data: " << directory << '\n';
            const auto source = directory / "source", output = directory / "output";
            std::filesystem::create_directory(source);
            AuthorProfile(source / "sentinel.xml", 75, output / "portraits" / "sentinel.xnb");
            // Resolve the game's serializer before XmlImporter encounters its Asset Type.
            (void)Intermediate::IntermediateSerializer::TypeSerializerFor<ProfileSource>();
            (void)Intermediate::IntermediateSerializer::TypeSerializerFor<RareBadgeSource>();
            const Pipeline::ContentPipelineRegistryFactory factory = [](const Pipeline::ContentCompilerOptions& options) {
                auto compiler = std::make_shared<Compiler::ContentCompiler>();
                (void)compiler->AddTypeWriter<VitalWriter>();
                (void)compiler->AddTypeWriter<BadgeWriter>();
                (void)compiler->AddTypeWriter<RareBadgeWriter>();
                const auto writer = compiler->AddTypeWriter<ProfileWriter>();
                if (writer->getTargetTypeProperty() == System::Type::From<ProfileSource>() &&
                    !writer->getCanDeserializeIntoExistingObjectProperty() &&
                    writer->getTypeVersionProperty() == 2 &&
                    writer->GetRuntimeReader(Xna::TargetPlatform::WindowsPhone) == ReaderName &&
                    writer->GetRuntimeType(Xna::TargetPlatform::WindowsPhone) == "CnaDemos.XmlProfile")
                    ++registrationContracts;
                auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
                Pipeline::RegisterXnaXmlSourceContentPipeline(*registry, compiler, options.xnbContainer);
                return registry;
            };
            const std::vector<std::filesystem::path> arguments{
                "build", source, "-o", output, "--format", "xnb", "--workers", "1",
                "--xnb-platform", "windowsphone", "--xnb-profile", "hidef", "--xnb-compress", "lzx"};
            const int built = Pipeline::RunContentCompiler(arguments, factory);
            check(built == 0 && writes == 1 && std::filesystem::exists(output / "sentinel.xnb"),
                  "the XML source route invokes the game's writer and publishes a real XNB");
            check(initializations == 1 && registrationContracts == 1 && initializedWrites == 1,
                  "the registered writer exposes its exact identity and receives its compiler before Write");
            const auto firstXnb = ReadFile(output / "sentinel.xnb");
            check(compressionChecks == 1 && compressionPlatforms == 1 && writerContexts == 1 &&
                  firstXnb.size() > 6 && firstXnb.substr(0, 3) == "XNB" && firstXnb[3] == 'm' &&
                  (static_cast<unsigned char>(firstXnb[5]) & 0x01u) != 0 &&
                  (static_cast<unsigned char>(firstXnb[5]) & 0x80u) == 0,
                  "the writer sees Windows Phone HiDef and declines the requested root compression");
            const auto manifestPath = output / Pipeline::ContentBuildManifestFileName;
            const auto firstManifest = ReadFile(manifestPath);
            const auto manifest = Pipeline::ContentBuildManifest::Parse(firstManifest);
            const auto* entry = manifest.Find("sentinel");
            check(entry && entry->importer.name == "XmlImporter" && entry->outputs.size() == 1 &&
                  entry->outputs.front().sha256 == Pipeline::ContentFileSha256(output / "sentinel.xnb"),
                  "the compiler manifest records XmlImporter and the digest of its actual published XNB");
            Runtime::ContentTypeReaderManager::AddTypeCreator(
                "CnaDemos.XmlVitalReader", [] { return std::make_unique<VitalReader>(); });
            Runtime::ContentTypeReaderManager::AddTypeCreator(
                "CnaDemos.XmlBadgeReader", [] { return std::make_unique<BadgeReader>(); });
            Runtime::ContentTypeReaderManager::AddTypeCreator(
                "CnaDemos.XmlRareBadgeReader", [] { return std::make_unique<RareBadgeReader>(); });
            Runtime::ContentTypeReaderManager::AddTypeCreator(ReaderName, [] { return std::make_unique<ProfileReader>(); });
            Runtime::ContentManager content(nullptr, output.string());
            const auto loaded = content.Load<std::shared_ptr<Profile>>("sentinel");
            check(loaded && loaded->name == "Sentinel" && loaded->health == 75 && loaded->rawHealth == 76 && reads == 1,
                  "ordinary, typed and raw writer paths dispatch to game readers and restore authored fields");
            const auto inlineRare = loaded ? std::dynamic_pointer_cast<RareBadge>(loaded->inlineBadge) : nullptr;
            const auto sharedRare = loaded ? std::dynamic_pointer_cast<RareBadge>(loaded->sharedBadgeA) : nullptr;
            check(inlineRare && sharedRare && inlineRare->label == "Founder" && inlineRare->level == 7 &&
                  inlineRare->glow == 11 && sharedRare->glow == 11 && loaded->inlineBadge != loaded->sharedBadgeA,
                  "base-typed badge writes dispatch to the derived reader for independent inline and shared values");
            check(loaded && loaded->sharedBadgeA == loaded->sharedBadgeB,
                  "two shared-resource references resolve to one deferred badge identity");
            check(loaded && loaded->portrait == "portraits/sentinel",
                  "the external reference is relative to the compiled profile and omits the XNB extension");
            check(content.Load<std::shared_ptr<Profile>>("sentinel") == loaded && reads == 1,
                  "a repeated runtime Load reuses the cached profile without invoking its reader");
            const int unchanged = Pipeline::RunContentCompiler(arguments, factory);
            check(unchanged == 0 && initializations == 2 && writes == 1 && compressionChecks == 1 &&
                  ReadFile(manifestPath) == firstManifest,
                  "a fresh XML pipeline registry skips unchanged authored content without calling the writer");
            AuthorProfile(source / "sentinel.xml", 120, output / "portraits" / "sentinel.xnb");
            const int edited = Pipeline::RunContentCompiler(arguments, factory);
            check(edited == 0 && initializations == 3 && writes == 2 && compressionChecks == 2 &&
                  initializedWrites == 2 && writerContexts == 2 && ReadFile(manifestPath) != firstManifest,
                  "editing the XML source rebuilds its XNB through the registered game writer");
            check(content.Load<std::shared_ptr<Profile>>("sentinel") == loaded && loaded->health == 75 && reads == 1,
                  "publishing new bytes does not silently replace an already cached runtime profile");
            content.Unload();
            const auto reloaded = content.Load<std::shared_ptr<Profile>>("sentinel");
            check(reloaded && reloaded != loaded && reloaded->name == "Sentinel" && reloaded->health == 120 &&
                  reloaded->rawHealth == 121 && reloaded->sharedBadgeA == reloaded->sharedBadgeB && reads == 2,
                  "Unload followed by Load reads the rebuilt XNB and exposes the edited value");
            const auto incompatible = directory / "cnb-output";
            const int refused = Pipeline::RunContentCompiler(
                {"build", source, "-o", incompatible, "--format", "cnb", "--workers", "1"}, factory);
            check(refused == 1 && writes == 2 && !std::filesystem::exists(incompatible / "sentinel.cnb"),
                  "the arbitrary XML object route refuses CNB without inventing a game-specific CNB schema");
        }
        check(!std::filesystem::exists(directory), "the demo removes its private XML sources and compiler outputs");

        std::promise<bool> probePromise;
        auto probeResult = probePromise.get_future();
        std::thread probe([promise = std::move(probePromise)]() mutable {
            try
            {
                Compiler::ContentCompiler compiler;
                (void)compiler.AddTypeWriter<InitializeCacheProbeWriter>();
                promise.set_value(true);
            }
            catch (...)
            {
                promise.set_value(false);
            }
        });
        const bool initializeReturned = probeResult.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
        const bool initializeSucceeded = initializeReturned && probeResult.get();
        if (initializeReturned) probe.join(); else probe.detach();
        check(initializeSucceeded,
              "Initialize can retrieve and cache a nested writer without deadlocking AddTypeWriter (CNA-DEMOS-ISSUE-090)");
        if (!initializeReturned)
        {
            // The blocked registration owns compiler state on the detached thread. Terminate the
            // focused process after flushing its result instead of running destructors against it.
            std::cout.flush();
            std::cerr.flush();
            std::_Exit(1);
        }
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "XML-to-XNB integration failed: " << error.what() << '\n';
        return 1;
    }
}
