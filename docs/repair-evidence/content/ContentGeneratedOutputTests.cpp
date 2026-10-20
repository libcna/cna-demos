// SPDX-License-Identifier: MS-PL
// ISSUE-047: output-root authorization must not grant arbitrary input access.
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>
#include "CNA/Content/Cnb/CnbCurveCodec.hpp"
#include "CNA/Content/Pipeline/ContentBuildManifest.hpp"
#include "CNA/Content/Pipeline/ContentPipeline.hpp"
#include "CNA/Content/Pipeline/XnaPipelineBridge.hpp"
#include "CNA/Internal/PathUtf8.hpp"

namespace
{
    namespace Canon = CNA::Content::Pipeline;
    namespace fs = std::filesystem;

    class QuietBuildLogger final : public Canon::ContentBuildLogger
    {
    public:
        void Log(const Canon::ContentLogMessage&) override {}
    };

    class ContentGeneratedOutputTest : public ::testing::Test
    {
    protected:
        fs::path root;
        bool ownsRoot = false;
        Canon::ContentProcessorParameters parameters;
        Canon::ContentSourceRootCapabilities capabilities;
        Canon::ContentDependencyCollector dependencies;
        QuietBuildLogger logger;

        void SetUp() override
        {
            const char* parent = std::getenv("CNA_GENERATED_OUTPUT_PROBE_ROOT");
            ASSERT_NE(parent, nullptr);
            ASSERT_TRUE(fs::path(parent).is_absolute());
            root = fs::path(parent) /
                ::testing::UnitTest::GetInstance()->current_test_info()->name();
            ASSERT_TRUE(fs::create_directory(root));
            ownsRoot = true;
            for (const auto* directory : {"source", "output", "output-extra", "outside"})
            {
                ASSERT_TRUE(fs::create_directory(root / directory));
            }
            Write(root / "source" / "input.txt", "authored input");
        }

        void TearDown() override
        {
            if (ownsRoot)
            {
                std::error_code error;
                fs::remove_all(root, error);
                EXPECT_FALSE(error) << error.message();
            }
        }

        static void Write(const fs::path& file, const std::string& bytes)
        {
            std::ofstream stream(file, std::ios::binary);
            stream.exceptions(std::ios::badbit | std::ios::failbit);
            stream << bytes;
            stream.close();
        }

        Canon::ContentProcessorContext Context(const fs::path& output)
        {
            Canon::ContentBuildEnvironment environment;
            environment.outputDirectory = output;
            return Canon::ContentProcessorContext(
                root / "source", root / "source" / "input.txt", "asset",
                "GeneratedOutputProbe", parameters, capabilities, dependencies,
                logger, Canon::ContentOutputFormat::Cnb, environment);
        }

        void ExpectRecorded(const fs::path& file)
        {
            const auto files = dependencies.DeploymentFiles();
            ASSERT_EQ(files.size(), 1u);
            EXPECT_EQ(files[0].source, fs::canonical(file));
            EXPECT_EQ(files[0].outputPath, "report.txt");
            std::ifstream stream(file, std::ios::binary);
            const std::string bytes{std::istreambuf_iterator<char>(stream), {}};
            EXPECT_EQ(bytes, "generated report");
        }

        Canon::ContentBuildResult ResultWithSupportFile(const fs::path& file, bool input)
        {
            namespace Cnb = CNA::Content::Cnb;
            Canon::ContentBuildResult result;
            result.source = fs::canonical(root / "source" / "input.txt");
            result.logicalName = "asset";
            result.importer = {"Probe.Importer", "1"};
            result.processor = {"Probe.Processor", "1"};
            result.writer = {"Probe.Writer", "1"};
            result.output.bytes = Cnb::EncodeCurveToCnb(
                Microsoft::Xna::Framework::Curve{}, result.logicalName);
            result.output.assetTypeId = Cnb::CnbAssetTypeId::Curve;
            result.output.assetTypeName = "Microsoft.Xna.Framework.Curve";
            result.output.assetSchemaVersion = Cnb::CnbCurveSchemaVersion;
            result.writerSchemas = {{result.output.assetTypeId, result.output.assetSchemaVersion,
                                     result.output.assetTypeName, {"Probe.CurveCodec", "1"}}};
            result.dependencies = {{Canon::ContentDependencyKind::PrimarySource,
                                    CNA::Internal::PathToGenericUtf8(result.source), {}}};
            if (input)
            {
                result.dependencies.push_back({Canon::ContentDependencyKind::SourceFile,
                                               CNA::Internal::PathToGenericUtf8(fs::canonical(file)), {}});
            }
            result.deploymentFiles = {{fs::canonical(file), "report.txt", {}}};
            return result;
        }

        Canon::ContentBuildManifestEntry ManifestEntry(const Canon::ContentBuildResult& result)
        {
            return Canon::MakeContentBuildManifestEntry(
                result, root / "source", root / "output", root / "output" / "asset.cnb");
        }
    };
}

TEST_F(ContentGeneratedOutputTest, RegistersFileInSeparateConfiguredOutputRoot)
{
    const auto file = root / "output" / "report.txt";
    Write(file, "generated report");
    auto canonical = Context(root / "output");
    Canon::XnaBridgeProcessorContext context(canonical);
    ASSERT_NO_THROW(context.AddOutputFile(CNA::Internal::PathToUtf8(file)));
    ExpectRecorded(file);
}

TEST_F(ContentGeneratedOutputTest, CanonicalizesConfiguredOutputRoot)
{
    const auto file = root / "output" / "report.txt";
    Write(file, "generated report");
    auto canonical = Context(root / "output" / ".." / "output" / ".");
    Canon::XnaBridgeProcessorContext context(canonical);
    ASSERT_NO_THROW(context.AddOutputFile(CNA::Internal::PathToUtf8(file)));
    ExpectRecorded(file);
}

TEST_F(ContentGeneratedOutputTest, RetainsSourceContainedDeploymentSupport)
{
    const auto file = root / "source" / "report.txt";
    Write(file, "generated report");
    auto canonical = Context(root / "output");
    Canon::XnaBridgeProcessorContext context(canonical);
    ASSERT_NO_THROW(context.AddOutputFile(CNA::Internal::PathToUtf8(file)));
    ExpectRecorded(file);
}

TEST_F(ContentGeneratedOutputTest, RefusesUnrelatedOutsideFile)
{
    const auto file = root / "outside" / "report.txt";
    Write(file, "generated report");
    auto canonical = Context(root / "output");
    Canon::XnaBridgeProcessorContext context(canonical);
    EXPECT_THROW(context.AddOutputFile(CNA::Internal::PathToUtf8(file)), std::invalid_argument);
    EXPECT_TRUE(dependencies.DeploymentFiles().empty());
}

TEST_F(ContentGeneratedOutputTest, RefusesOutputRootPrefixLookalike)
{
    const auto file = root / "output-extra" / "report.txt";
    Write(file, "generated report");
    auto canonical = Context(root / "output");
    Canon::XnaBridgeProcessorContext context(canonical);
    EXPECT_THROW(context.AddOutputFile(CNA::Internal::PathToUtf8(file)), std::invalid_argument);
    EXPECT_TRUE(dependencies.DeploymentFiles().empty());
}

TEST_F(ContentGeneratedOutputTest, RefusesOutputSymlinkEscapingBothAuthorizedRoots)
{
    const auto target = root / "outside" / "report.txt";
    const auto link = root / "output" / "report.txt";
    Write(target, "generated report");
    fs::create_symlink(target, link);
    auto canonical = Context(root / "output");
    Canon::XnaBridgeProcessorContext context(canonical);
    EXPECT_THROW(context.AddOutputFile(CNA::Internal::PathToUtf8(link)), std::invalid_argument);
    EXPECT_TRUE(dependencies.DeploymentFiles().empty());
}

TEST_F(ContentGeneratedOutputTest, EmptyOutputRootDoesNotGrantOutsideAccess)
{
    const auto file = root / "outside" / "report.txt";
    Write(file, "generated report");
    auto canonical = Context({});
    Canon::XnaBridgeProcessorContext context(canonical);
    EXPECT_THROW(context.AddOutputFile(CNA::Internal::PathToUtf8(file)), std::invalid_argument);
    EXPECT_TRUE(dependencies.DeploymentFiles().empty());
}

TEST_F(ContentGeneratedOutputTest, GeneratedManifestRoundTripsWithoutMakingOutputAnInput)
{
    const auto file = root / "output" / "report.txt";
    Write(file, "generated report");
    const auto result = ResultWithSupportFile(file, false);
    Canon::ContentBuildManifestEntry entry;
    ASSERT_NO_THROW(entry = ManifestEntry(result));
    ASSERT_EQ(entry.dependencies.size(), 1u);
    EXPECT_EQ(entry.dependencies[0].kind, Canon::ContentDependencyKind::PrimarySource);
    ASSERT_EQ(entry.deploymentFiles.size(), 1u);
    EXPECT_EQ(entry.deploymentFiles[0].source, "report.txt");
    EXPECT_EQ(entry.deploymentFiles[0].path, "report.txt");
    EXPECT_TRUE(entry.deploymentFiles[0].sourceRoot.empty());
    EXPECT_EQ(entry.deploymentFiles[0].sha256, Canon::ContentFileSha256(file));
    Canon::RefreshContentBuildDirectFingerprint(entry, root / "source");
    Canon::RefreshContentBuildEffectiveFingerprint(entry);
    Canon::ContentBuildManifest manifest;
    ASSERT_NO_THROW(manifest.Set(entry));
    const auto serialized = manifest.Serialize();
    EXPECT_EQ(serialized.find(CNA::Internal::PathToGenericUtf8(root)), std::string::npos);
    const auto restored = Canon::ContentBuildManifest::Parse(serialized);
    ASSERT_NE(restored.Find("asset"), nullptr);
    EXPECT_EQ(*restored.Find("asset"), *manifest.Find("asset"));

    const auto direct = entry.directFingerprint;
    Write(file, "changed output");
    EXPECT_EQ(Canon::ComputeContentBuildDirectFingerprint(entry, root / "source"), direct);
    ASSERT_TRUE(fs::remove(file));
    EXPECT_EQ(Canon::ComputeContentBuildDirectFingerprint(entry, root / "source"), direct);
    // Published-output digests detect tampering/deletion separately; neither
    // event invents a changed source input. Real input bytes must still matter.
    Write(root / "source" / "input.txt", "changed authored input");
    EXPECT_NE(Canon::ComputeContentBuildDirectFingerprint(entry, root / "source"), direct);
}

TEST_F(ContentGeneratedOutputTest, SourceBackedManifestRetainsByteHashedInputDependency)
{
    const auto file = root / "source" / "report.txt";
    Write(file, "generated report");
    auto entry = ManifestEntry(ResultWithSupportFile(file, true));
    ASSERT_EQ(entry.dependencies.size(), 2u);
    Canon::RefreshContentBuildDirectFingerprint(entry, root / "source");
    Canon::RefreshContentBuildEffectiveFingerprint(entry);
    Canon::ContentBuildManifest manifest;
    ASSERT_NO_THROW(manifest.Set(entry));
    const auto direct = entry.directFingerprint;
    Write(file, "changed source support");
    EXPECT_NE(Canon::ComputeContentBuildDirectFingerprint(entry, root / "source"), direct);
    ASSERT_TRUE(fs::remove(file));
    EXPECT_THROW((void)Canon::ComputeContentBuildDirectFingerprint(entry, root / "source"),
                 std::runtime_error);
}

TEST_F(ContentGeneratedOutputTest, ManifestRefusesSupportOutsideInputAndOutputRoots)
{
    const auto file = root / "outside" / "report.txt";
    Write(file, "generated report");
    const auto result = ResultWithSupportFile(file, false);
    EXPECT_THROW((void)ManifestEntry(result), std::runtime_error);
}

TEST_F(ContentGeneratedOutputTest, ManifestRefusesDeploymentOverAuthoredSource)
{
    const auto file = root / "source" / "report.txt";
    Write(file, "generated report");
    auto result = ResultWithSupportFile(file, true);
    result.deploymentFiles[0].outputPath = "source/input.txt";
    EXPECT_THROW((void)Canon::MakeContentBuildManifestEntry(
                     result, root / "source", root, root / "asset.cnb"),
                 std::invalid_argument);
    std::ifstream stream(root / "source" / "input.txt");
    const std::string original{std::istreambuf_iterator<char>(stream), {}};
    EXPECT_EQ(original, "authored input");
}
