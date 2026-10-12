// SPDX-License-Identifier: MS-PL
// Original XML authoring -> the real source pipeline -> custom XNB writer -> runtime reader.
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
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
    int reads = 0;
    struct ProfileSource : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.ProfileSource";
        const std::string& GetTypeName() const override
        {
            static const std::string name(XnaTypeName); return name;
        }
        std::string name;
        int health = 0;
        static void DescribeContent(Intermediate::ContentTypeDescriptor<ProfileSource>& type)
        {
            type.Field("Name", &ProfileSource::name);
            type.Field("Health", &ProfileSource::health);
        }
    };
    // Runtime data has no dependency on XML descriptors or the build-time Object hierarchy.
    struct Profile { std::string name; int health = 0; };
    class ProfileWriter final : public Compiler::ContentTypeWriter<ProfileSource>
    {
    public:
        std::string GetRuntimeReader(Xna::TargetPlatform) const override { return ReaderName; }
        std::string GetRuntimeType(Xna::TargetPlatform) const override { return "CnaDemos.XmlProfile"; }
        std::int32_t getTypeVersionProperty() const override { return 2; }
    protected:
        void Write(Compiler::ContentWriter& output, const TargetCarrier& value) override
        {
            ++writes;
            output.Write(value->name);
            output.Write(value->health);
        }
    };
    class ProfileReader final : public Runtime::ContentTypeReader<Profile>
    {
    public:
        ProfileReader() : ContentTypeReader<Profile>("CnaDemos.XmlProfile") {}
        int getTypeVersionProperty() const override { return 2; }
    protected:
        Profile Read(Runtime::ContentReader& input, std::optional<Profile>) override
        {
            ++reads;
            return {input.ReadString(), input.ReadInt32()};
        }
    };
    struct TemporaryDirectory
    {
        std::filesystem::path path;
        ~TemporaryDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    };
    void AuthorProfile(const std::filesystem::path& path, int health)
    {
        std::ofstream file(path);
        file << "<XnaContent><Asset Type=\"CnaDemos.ProfileSource\"><Name>Sentinel</Name><Health>"
             << health << "</Health></Asset></XnaContent>\n";
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
            AuthorProfile(source / "sentinel.xml", 75);
            // Resolve the game's serializer before XmlImporter encounters its Asset Type.
            (void)Intermediate::IntermediateSerializer::TypeSerializerFor<ProfileSource>();
            const Pipeline::ContentPipelineRegistryFactory factory = [](const Pipeline::ContentCompilerOptions& options) {
                auto compiler = std::make_shared<Compiler::ContentCompiler>();
                (void)compiler->AddTypeWriter<ProfileWriter>();
                auto registry = std::make_shared<Pipeline::ContentPipelineRegistry>();
                Pipeline::RegisterXnaXmlSourceContentPipeline(*registry, compiler, options.xnbContainer);
                return registry;
            };
            const std::vector<std::filesystem::path> arguments{
                "build", source, "-o", output, "--format", "xnb", "--workers", "1"};
            const int built = Pipeline::RunContentCompiler(arguments, factory);
            check(built == 0 && writes == 1 && std::filesystem::exists(output / "sentinel.xnb"),
                  "the XML source route invokes the game's writer and publishes a real XNB");
            const auto manifestPath = output / Pipeline::ContentBuildManifestFileName;
            const auto firstManifest = ReadFile(manifestPath);
            const auto manifest = Pipeline::ContentBuildManifest::Parse(firstManifest);
            const auto* entry = manifest.Find("sentinel");
            check(entry && entry->importer.name == "XmlImporter" && entry->outputs.size() == 1 &&
                  entry->outputs.front().sha256 == Pipeline::ContentFileSha256(output / "sentinel.xnb"),
                  "the compiler manifest records XmlImporter and the digest of its actual published XNB");
            Runtime::ContentTypeReaderManager::AddTypeCreator(ReaderName, [] { return std::make_unique<ProfileReader>(); });
            Runtime::ContentManager content(nullptr, output.string());
            const auto loaded = content.Load<Profile>("sentinel");
            check(loaded.name == "Sentinel" && loaded.health == 75 && reads == 1,
                  "the writer's reader name and version dispatch to the game reader and restore authored fields");
            check(content.Load<Profile>("sentinel").health == 75 && reads == 1,
                  "a repeated runtime Load reuses the cached profile without invoking its reader");
            const int unchanged = Pipeline::RunContentCompiler(arguments, factory);
            check(unchanged == 0 && writes == 1 && ReadFile(manifestPath) == firstManifest,
                  "a fresh XML pipeline registry skips unchanged authored content without calling the writer");
            AuthorProfile(source / "sentinel.xml", 120);
            const int edited = Pipeline::RunContentCompiler(arguments, factory);
            check(edited == 0 && writes == 2 && ReadFile(manifestPath) != firstManifest,
                  "editing the XML source rebuilds its XNB through the registered game writer");
            check(content.Load<Profile>("sentinel").health == 75 && reads == 1,
                  "publishing new bytes does not silently replace an already cached runtime profile");
            content.Unload();
            const auto reloaded = content.Load<Profile>("sentinel");
            check(reloaded.name == "Sentinel" && reloaded.health == 120 && reads == 2,
                  "Unload followed by Load reads the rebuilt XNB and exposes the edited value");
            const auto incompatible = directory / "cnb-output";
            const int refused = Pipeline::RunContentCompiler(
                {"build", source, "-o", incompatible, "--format", "cnb", "--workers", "1"}, factory);
            check(refused == 1 && writes == 2 && !std::filesystem::exists(incompatible / "sentinel.cnb"),
                  "the arbitrary XML object route refuses CNB without inventing a game-specific CNB schema");
        }
        check(!std::filesystem::exists(directory), "the demo removes its private XML sources and compiler outputs");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "XML-to-XNB integration failed: " << error.what() << '\n';
        return 1;
    }
}
