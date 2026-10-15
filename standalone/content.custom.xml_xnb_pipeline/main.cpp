// SPDX-License-Identifier: MS-PL
// Original XML authoring -> the real source pipeline -> custom XNB writer -> runtime reader.
#include <algorithm>
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
#include "CNA/Content/Pipeline/XnaPipelineBridge.hpp"
#include "CNA/Content/Pipeline/XnaXmlSourceContentPipeline.hpp"
#include "CNA/Content/Pipeline/XnbOutputContentPipeline.hpp"
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
    std::atomic<int> compilerInventories{0};
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
    class ExistingInt32XnbWriter final : public Pipeline::ContentTypeWriter
    {
    public:
        Pipeline::ContentComponentIdentity Identity() const override
        {
            return {"CnaDemos.ExistingInt32XnbWriter", "1"};
        }
        Pipeline::ContentOutputFormat OutputFormat() const override
        {
            return Pipeline::ContentOutputFormat::Xnb;
        }
        std::vector<Pipeline::ContentWriterSchemaIdentity> OutputSchemaIdentities() const override
        {
            return {{static_cast<std::uint32_t>(Pipeline::XnbOutputAssetId::XnaObject), 1u,
                     "System.Int32", {"CnaDemos.ExistingInt32Xnb", "1"}}};
        }
        std::string InputType() const override { return "System.Int32"; }
        Pipeline::ContentWriteResult Write(const Pipeline::ContentValue&, const std::string&) const override
        {
            throw std::logic_error("The collision sentinel is inspection-only.");
        }
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
    enum class WriterProbeMode { RawNull, MissingDynamic, ForeignWriter };
    struct WriterProbeSource : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.WriterProbeSource";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName); return name;
        }
        WriterProbeMode mode = WriterProbeMode::RawNull;
        std::shared_ptr<BadgeSource> badge;
    };
    std::shared_ptr<Compiler::ContentTypeWriterBase> foreignVitalWriter;
    class WriterProbeWriter final : public Compiler::ContentTypeWriter<WriterProbeSource>
    {
    public:
        std::string GetRuntimeReader(Xna::TargetPlatform) const override { return "CnaDemos.UnusedProbeReader"; }
    protected:
        void Write(Compiler::ContentWriter& output, const TargetCarrier& value) override
        {
            switch (value->mode)
            {
                case WriterProbeMode::RawNull:
                    output.WriteRawObject<BadgeSource>(nullptr);
                    break;
                case WriterProbeMode::MissingDynamic:
                    output.WriteObject<BadgeSource>(value->badge);
                    break;
                case WriterProbeMode::ForeignWriter:
                    output.WriteObject<Vital>(Vital{5}, *foreignVitalWriter);
                    break;
            }
        }
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
    void AuthorProfile(const std::filesystem::path& path, int health, const std::filesystem::path& portrait,
                       const bool includeBadge)
    {
        std::ofstream file(path);
        file << "<XnaContent xmlns:CnaDemos=\"CnaDemos\"><Asset Type=\"CnaDemos:ProfileSource\">"
             << "<Name>Sentinel</Name><Health>" << health << "</Health>";
        if (includeBadge)
        {
            file << "<Badge Type=\"CnaDemos:RareBadgeSource\"><Label>Founder</Label><Level>7</Level>"
                 << "<Glow>11</Glow></Badge><Portrait>" << portrait.generic_string() << "</Portrait>";
        }
        else
        {
            file << "<Badge Null=\"true\" /><Portrait></Portrait>";
        }
        file << "</Asset></XnaContent>\n";
        file.close();
        if (!file) throw std::runtime_error("Could not write original profile XML.");
    }
    std::string ReadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file) throw std::runtime_error("Could not read published compiler data.");
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }
    void WriteFile(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes)
    {
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        file.close();
        if (!file) throw std::runtime_error("Could not write compiled probe data.");
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
            std::filesystem::create_directory(output);

            auto directCompiler = std::make_shared<Compiler::ContentCompiler>();
            (void)directCompiler->AddTypeWriter<VitalWriter>();
            Pipeline::ContentPipelineRegistry directRegistry;
            const auto existingInt32 = std::make_shared<ExistingInt32XnbWriter>();
            directRegistry.RegisterWriter(existingInt32);
            CNA::Internal::Xnb::XnbFileOptions directOptions;
            directOptions.compression = CNA::Internal::Xnb::XnbOutputCompression::Lzx;
            directOptions.readerNameStyle = CNA::Internal::Xnb::XnbReaderNameStyle::Portable;
            Pipeline::RegisterXnaXnbOutput(directRegistry, directCompiler, directOptions);
            const auto directNames = directCompiler->KnownTypeNames();
            bool directInventory = directRegistry.Writers().size() == directNames.size();
            for (const auto& name : directNames)
            {
                const auto registered = directRegistry.ResolveWriter(name, {}, Pipeline::ContentOutputFormat::Xnb);
                directInventory &= registered->InputType() == name &&
                    registered->OutputFormat() == Pipeline::ContentOutputFormat::Xnb;
            }
            check(directInventory &&
                      directRegistry.ResolveWriter("System.Int32", {}, Pipeline::ContentOutputFormat::Xnb) == existingInt32,
                  "direct XNB registration covers every compiler type while preserving a pre-registered route");

            const auto firstDirectWriters = directRegistry.Writers();
            Pipeline::RegisterXnaXnbOutput(directRegistry, directCompiler, directOptions);
            const auto vitalWriter =
                directRegistry.ResolveWriter("CnaDemos.XmlVital", {}, Pipeline::ContentOutputFormat::Xnb);
            const auto vitalSchemas = vitalWriter->OutputSchemaIdentities();
            check(directRegistry.Writers() == firstDirectWriters &&
                      vitalWriter->Identity() == Pipeline::ContentComponentIdentity{
                          "CNA.XnaObjectXnbWriter[CnaDemos.XmlVital]",
                          "1;" + Pipeline::XnbOutputOptionsDigest(directOptions)} &&
                      vitalSchemas.size() == 1u &&
                      vitalSchemas.front().assetTypeId ==
                          static_cast<std::uint32_t>(Pipeline::XnbOutputAssetId::XnaObject) &&
                      vitalSchemas.front().assetSchemaVersion == 1u &&
                      vitalSchemas.front().assetTypeName == "CnaDemos.XmlVital",
                  "repeated direct registration is idempotent and exposes the configured writer identity and schema");

            Pipeline::ContentBuildEnvironment directEnvironment;
            directEnvironment.targetPlatform = Pipeline::ContentTargetPlatform::WindowsPhone;
            directEnvironment.targetProfile = Microsoft::Xna::Framework::Graphics::GraphicsProfile::HiDef;
            directEnvironment.outputDirectory = output;
            const auto directVital = vitalWriter->Write(
                Pipeline::ContentValue::Create("CnaDemos.XmlVital", Vital{63}),
                "direct-registration", directEnvironment);
            WriteFile(output / "direct-registration.xnb", directVital.bytes);
            Runtime::ContentTypeReaderManager::AddTypeCreator(
                "CnaDemos.XmlVitalReader", [] { return std::make_unique<VitalReader>(); });
            Runtime::ContentManager directContent(nullptr, output.string());
            check(directVital.assetTypeId == static_cast<std::uint32_t>(Pipeline::XnbOutputAssetId::XnaObject) &&
                      directVital.assetSchemaVersion == 1u && directVital.assetTypeName == "CnaDemos.XmlVital" &&
                      directVital.bytes.size() > 6u &&
                      (static_cast<unsigned char>(directVital.bytes[5]) & 0x80u) != 0u &&
                      directContent.Load<Vital>("direct-registration").health == 63,
                  "a directly registered writer applies configured compression and produces a loadable typed XNB");
            std::cout << "Direct XNB target: "
                      << (directVital.bytes[3] == 'm' ? "WindowsPhone" :
                          directVital.bytes[3] == 'x' ? "Xbox360" : "Windows")
                      << '/' << ((static_cast<unsigned char>(directVital.bytes[5]) & 0x01u) != 0u
                                     ? "HiDef" : "Reach") << '\n';
            check(directVital.bytes[3] == 'm' &&
                      (static_cast<unsigned char>(directVital.bytes[5]) & 0x01u) != 0u,
                  "the directly registered writer applies the build environment's Windows Phone and HiDef values (CNA-DEMOS-ISSUE-111)");

            bool nullCompilerRefused = false;
            try
            {
                Pipeline::ContentPipelineRegistry nullRegistry;
                Pipeline::RegisterXnaXnbOutput(nullRegistry, {}, {});
            }
            catch (const std::invalid_argument& error)
            {
                nullCompilerRefused = std::string(error.what()) ==
                    "RegisterXnaXnbOutput(): compiler must not be null.";
            }
            check(nullCompilerRefused, "direct XNB registration refuses a null compiler before mutating its registry");

            AuthorProfile(source / "sentinel.xml", 75, output / "portraits" / "sentinel.xnb", true);
            // Resolve the game's serializer before XmlImporter encounters its Asset Type.
            (void)Intermediate::IntermediateSerializer::TypeSerializerFor<ProfileSource>();
            (void)Intermediate::IntermediateSerializer::TypeSerializerFor<RareBadgeSource>();
            std::shared_ptr<Compiler::ContentCompiler> latestCompiler;
            const Pipeline::ContentPipelineRegistryFactory factory = [&](const Pipeline::ContentCompilerOptions& options) {
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
                const auto names = compiler->KnownTypeNames();
                if (std::is_sorted(names.begin(), names.end()) &&
                    std::binary_search(names.begin(), names.end(), "System.Int32") &&
                    std::binary_search(names.begin(), names.end(), "CnaDemos.XmlVital") &&
                    std::binary_search(names.begin(), names.end(), "CnaDemos.BadgeSource") &&
                    std::binary_search(names.begin(), names.end(), "CnaDemos.RareBadgeSource") &&
                    std::binary_search(names.begin(), names.end(), "CnaDemos.ProfileSource"))
                    ++compilerInventories;
                latestCompiler = compiler;
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
            check(compilerInventories == 1,
                  "the compiler reports a sorted inventory containing built-in and all game-owned writer types");
            bool lateWriterRefused = false;
            try { (void)latestCompiler->AddTypeWriter<WriterProbeWriter>(); }
            catch (const Xna::PipelineException& error)
            {
                lateWriterRefused = std::string(error.what()).find("cannot be added after the compiler has compiled") !=
                                    std::string::npos;
            }
            bool unknownWriterRefused = false;
            try { (void)latestCompiler->GetTypeWriter(System::Type::From<WriterProbeSource>()); }
            catch (const Xna::PipelineException& error)
            {
                unknownWriterRefused = std::string(error.what()).find("no type writer is registered") != std::string::npos;
            }
            check(lateWriterRefused && unknownWriterRefused &&
                  latestCompiler->GetTypeWriter(System::Type::From<Vital>())->getTargetTypeProperty() ==
                      System::Type::From<Vital>(),
                  "a completed compile freezes registration while known lookup stays usable and unknown lookup is refused");
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
            AuthorProfile(source / "sentinel.xml", 120, output / "portraits" / "sentinel.xnb", false);
            const int edited = Pipeline::RunContentCompiler(arguments, factory);
            check(edited == 0 && initializations == 3 && writes == 2 && compressionChecks == 2 &&
                  initializedWrites == 2 && writerContexts == 2 && ReadFile(manifestPath) != firstManifest,
                  "editing the XML source rebuilds its XNB through the registered game writer");
            check(content.Load<std::shared_ptr<Profile>>("sentinel") == loaded && loaded->health == 75 && reads == 1,
                  "publishing new bytes does not silently replace an already cached runtime profile");
            content.Unload();
            const auto reloaded = content.Load<std::shared_ptr<Profile>>("sentinel");
            check(reloaded && reloaded != loaded && reloaded->name == "Sentinel" && reloaded->health == 120 &&
                  reloaded->rawHealth == 121 && reads == 2,
                  "Unload followed by Load reads the rebuilt XNB and exposes the edited value");
            check(reloaded && !reloaded->inlineBadge && !reloaded->sharedBadgeA &&
                  !reloaded->sharedBadgeB && reloaded->portrait.empty(),
                  "null ordinary/shared references and an empty external reference round trip without fixups");

            Compiler::ContentCompiler compressedCompiler;
            (void)compressedCompiler.AddTypeWriter<VitalWriter>(Compiler::ContentTypeWriterAttribute{});
            Compiler::CompileOptions compressedOptions;
            compressedOptions.assetName = "default-compressed";
            compressedOptions.outputDirectory = output;
            compressedOptions.compressContent = true;
            const auto compressed = compressedCompiler.Compile<Vital>(Vital{91}, compressedOptions);
            WriteFile(output / "default-compressed.xnb", compressed.bytes);
            Runtime::ContentManager compressedContent(nullptr, output.string());
            const auto compressedVital = compressedContent.Load<Vital>("default-compressed");
            check(compressed.bytes.size() > 6 &&
                  (static_cast<unsigned char>(compressed.bytes[5]) & 0x80u) != 0 &&
                  compressedVital.health == 91,
                  "the default writer compression policy honors requested LZX and the runtime restores its payload");
            const auto incompatible = directory / "cnb-output";
            const int refused = Pipeline::RunContentCompiler(
                {"build", source, "-o", incompatible, "--format", "cnb", "--workers", "1"}, factory);
            check(refused == 1 && writes == 2 && !std::filesystem::exists(incompatible / "sentinel.cnb"),
                  "the arbitrary XML object route refuses CNB without inventing a game-specific CNB schema");
        }
        check(!std::filesystem::exists(directory), "the demo removes its private XML sources and compiler outputs");

        const auto compileProbe = [](const WriterProbeMode mode) {
            std::unique_ptr<Compiler::ContentCompiler> foreignCompiler;
            if (mode == WriterProbeMode::ForeignWriter)
            {
                foreignCompiler = std::make_unique<Compiler::ContentCompiler>();
                foreignVitalWriter = foreignCompiler->AddTypeWriter<VitalWriter>();
            }
            Compiler::ContentCompiler compiler;
            (void)compiler.AddTypeWriter<VitalWriter>();
            (void)compiler.AddTypeWriter<BadgeWriter>();
            (void)compiler.AddTypeWriter<WriterProbeWriter>();
            auto value = std::make_shared<WriterProbeSource>();
            value->mode = mode;
            if (mode == WriterProbeMode::MissingDynamic)
                value->badge = std::make_shared<RareBadgeSource>();
            Compiler::CompileOptions options;
            options.assetName = "writer-probe";
            std::string diagnostic;
            try { (void)compiler.Compile<WriterProbeSource>(value, options); }
            catch (const Xna::PipelineException& error) { diagnostic = error.what(); }
            foreignVitalWriter.reset();
            return diagnostic;
        };
        const auto rawNull = compileProbe(WriterProbeMode::RawNull);
        check(rawNull.find("null reference has no raw form") != std::string::npos,
              "WriteRawObject refuses a null reference because raw payloads have no type marker");
        const auto missingDynamic = compileProbe(WriterProbeMode::MissingDynamic);
        check(missingDynamic.find("No ContentTypeWriter is registered for 'CnaDemos.RareBadgeSource'") != std::string::npos &&
              missingDynamic.find("declared as 'CnaDemos.BadgeSource'") != std::string::npos,
              "base-typed dynamic dispatch refuses a derived object with no exact writer");
        const auto foreignWriter = compileProbe(WriterProbeMode::ForeignWriter);
        check(foreignWriter.find("does not belong to the compiler writing this file") != std::string::npos,
              "an explicit nested writer from another compiler is refused before emitting its value");

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
