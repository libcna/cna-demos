// SPDX-License-Identifier: MS-PL
// ISSUE-047: output-root authorization must not grant arbitrary input access.
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>
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
