// SPDX-License-Identifier: MS-PL
// XML external references carry relocatable filenames, not embedded instances of their targets.
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Microsoft/Xna/Framework/Content/Pipeline/ExternalReference.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/InvalidContentException.hpp"
#include "Microsoft/Xna/Framework/Content/Pipeline/Serialization/Intermediate/IntermediateSerializer.hpp"
#include "System/Object.hpp"
#include "System/Xml/XmlReader.hpp"
#include "System/Xml/XmlWriter.hpp"
#include "System/Xml/XmlWriterSettings.hpp"

namespace Xna = Microsoft::Xna::Framework::Content::Pipeline;
using namespace Xna::Serialization::Intermediate;
namespace
{
    struct Profile : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.External.Profile";
        const std::string& GetTypeName() const override { static const std::string name(XnaTypeName); return name; }
        int health = 0;
        static void DescribeContent(ContentTypeDescriptor<Profile>& type) { type.Field("Health", &Profile::health); }
    };
    using Reference = Xna::ExternalReference<Profile>;
    struct Scene : System::Object
    {
        static constexpr std::string_view XnaTypeName = "CnaDemos.External.Scene";
        const std::string& GetTypeName() const override { static const std::string name(XnaTypeName); return name; }
        std::shared_ptr<Profile> preview, shared;
        std::shared_ptr<Reference> primary, again, other, empty, absent;
        static void DescribeContent(ContentTypeDescriptor<Scene>& type)
        {
            type.Field("Preview", &Scene::preview);
            type.Field("Shared", &Scene::shared).SharedResource();
            type.Field("Primary", &Scene::primary);
            type.Field("Again", &Scene::again);
            type.Field("Other", &Scene::other);
            type.Field("Empty", &Scene::empty);
            type.Field("Absent", &Scene::absent);
        }
    };
}
int main()
{
    try
    {
        bool passed = true;
        const auto check = [&](bool condition, std::string_view message) {
            std::cout << (condition ? "  check: " : "  CHECK FAILED: ") << message << '\n';
            passed = condition && passed;
        };
        const auto sourceRoot = std::filesystem::temp_directory_path() / "cna-demos-authored-reference-layout";
        const auto movedRoot = std::filesystem::temp_directory_path() / "cna-demos-relocated-reference-layout";
        const auto sourceFile = sourceRoot / "scenes" / "room.xml";
        const auto movedFile = movedRoot / "scenes" / "room.xml";
        // These are authored file locations. Serialization records names; it does not import them.
        const auto guardFile = (sourceRoot / "profiles" / "guard.xml").generic_string();
        const auto bossFile = (sourceRoot / "profiles" / "boss.xml").generic_string();
        auto authored = std::make_shared<Scene>();
        authored->preview = std::make_shared<Profile>(); authored->preview->health = 90;
        authored->shared = std::make_shared<Profile>(); authored->shared->health = 75;
        authored->primary = std::make_shared<Reference>(guardFile);
        authored->again = std::make_shared<Reference>(guardFile);
        authored->other = std::make_shared<Reference>(bossFile);
        authored->empty = std::make_shared<Reference>();
        System::Xml::XmlWriterSettings settings; settings.Indent = true; settings.NewLineChars = "\n";
        std::unique_ptr<System::Xml::XmlWriter> writer(System::Xml::XmlWriter::CreateToString(settings));
        IntermediateSerializer::Serialize<Scene>(*writer, authored, sourceFile.generic_string());
        const auto xml = writer->ToString(); std::cout << xml << '\n';
        std::size_t entries = 0;
        for (auto at = xml.find("<ExternalReference "); at != std::string::npos; at = xml.find("<ExternalReference ", at + 1)) ++entries;
        check(entries == 2, "two wrappers naming the same target share one external table entry while a different filename gets its own");
        check(xml.find("..\\profiles\\guard.xml") != std::string::npos && xml.find("..\\profiles\\boss.xml") != std::string::npos &&
              xml.find(sourceRoot.generic_string()) == std::string::npos,
              "the source document location makes absolute targets relative in the emitted XML");
        const auto health = xml.find("<Health>90</Health>");
        check(health != std::string::npos && xml.find("<Health>90</Health>", health + 1) == std::string::npos,
              "the preview is embedded once; external references contain filenames rather than copies of its fields");
        const auto resources = xml.find("  <Resources>");
        const auto externals = xml.find("  <ExternalReferences>");
        check(resources != std::string::npos && externals != std::string::npos && resources < externals &&
              xml.find("<Health>75</Health>", resources) < externals,
              "a combined document emits shared Resources before ExternalReferences");
        check(xml.find("<Empty />") != std::string::npos,
              "an external wrapper with no filename is an empty member and adds no table entry");
        bool roundTrip = false;
        try
        {
            std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(xml));
            const auto loaded = IntermediateSerializer::Deserialize<Scene>(*reader, sourceFile.generic_string());
            roundTrip = loaded && loaded->primary && loaded->primary->getFilenameProperty() == guardFile;
        }
        catch (const Xna::InvalidContentException& error)
        {
            // Preserve only the diagnosed alias failure; unrelated failures must stop the tool.
            if (std::string(error.what()).find("Cannot find type \"External:Profile\".") == std::string::npos) throw;
            std::cout << "Native emitted XML refusal: " << error.what() << '\n';
        }
        check(roundTrip, "emitted external TargetType declarations round-trip in the same namespace as the scene (CNA-DEMOS-ISSUE-046)");

        // An independent canonical-name control keeps relocation/identity checks observable even
        // if the emitted namespace alias fails. The original assertion and XML above are retained.
        std::string canonical = xml;
        std::size_t changed = 0;
        for (auto at = canonical.find("TargetType=\""); at != std::string::npos; at = canonical.find("TargetType=\"", at))
        {
            at += std::string_view("TargetType=\"").size();
            const auto end = canonical.find('"', at);
            if (end == std::string::npos) throw std::runtime_error("Incomplete TargetType attribute");
            canonical.replace(at, end - at, Profile::XnaTypeName);
            at += Profile::XnaTypeName.size(); ++changed;
        }
        if (changed != 2) throw std::runtime_error("Expected two external target declarations");
        std::unique_ptr<System::Xml::XmlReader> reader(System::Xml::XmlReader::CreateFromString(canonical));
        const auto moved = IntermediateSerializer::Deserialize<Scene>(*reader, movedFile.generic_string());
        if (!moved || !moved->shared || !moved->primary || !moved->again || !moved->other || !moved->empty)
            throw std::runtime_error("Missing relocated references");
        check(moved->primary->getFilenameProperty() == (movedRoot / "profiles" / "guard.xml").generic_string() &&
              moved->other->getFilenameProperty() == (movedRoot / "profiles" / "boss.xml").generic_string(),
              "deserialization resolves relative targets against the new document location");
        check(moved->primary != moved->again && moved->primary->getFilenameProperty() == moved->again->getFilenameProperty(),
              "equal external filenames preserve separate reference wrappers rather than shared object identity");
        check(!moved->absent && moved->preview && moved->preview != authored->preview && moved->preview->health == 90,
              "the null reference remains absent and the inline preview restores its own data");
        check(moved->empty->getFilenameProperty().empty(),
              "an empty external wrapper restores as an object with no filename rather than a null reference");
        check(moved->shared != authored->shared && moved->shared->health == 75,
              "the shared resource is restored as a fresh object alongside relocated external wrappers");
        const auto refuse = [&](const std::string& input, const std::string& relocation, const char* diagnostic, const char* message) {
            bool refused = false;
            try
            {
                std::unique_ptr<System::Xml::XmlReader> bad(System::Xml::XmlReader::CreateFromString(input));
                (void)IntermediateSerializer::Deserialize<Scene>(*bad, relocation);
            }
            catch (const Xna::InvalidContentException& error)
            {
                refused = std::string(error.what()).find(diagnostic) != std::string::npos;
                std::cout << "Refused reference: " << error.what() << '\n';
            }
            check(refused, message);
        };
        refuse(canonical, "", "Invalid filesystem location", "relative references require a document relocation location");
        std::string missing = canonical;
        const auto ref = missing.find("<Reference>");
        if (ref == std::string::npos) throw std::runtime_error("Missing emitted reference");
        const auto end = missing.find("</Reference>", ref);
        missing.replace(ref + 11, end - ref - 11, "#Missing");
        refuse(missing, movedFile.generic_string(), "Missing external reference", "an unresolved external table ID is refused");
        std::string wrong = canonical;
        const auto type = wrong.find("TargetType=\"") + std::string_view("TargetType=\"").size();
        wrong.replace(type, Profile::XnaTypeName.size(), "System.String");
        refuse(wrong, movedFile.generic_string(), "wrong type for external reference", "a known but incompatible target type is refused");

        std::string missingTargetType = canonical;
        const auto targetAttribute = missingTargetType.find(" TargetType=\"");
        const auto targetAttributeEnd = targetAttribute == std::string::npos
            ? std::string::npos
            : missingTargetType.find('"', targetAttribute + std::string_view(" TargetType=\"").size());
        if (targetAttribute == std::string::npos || targetAttributeEnd == std::string::npos)
            throw std::runtime_error("Missing emitted TargetType attribute");
        missingTargetType.erase(targetAttribute, targetAttributeEnd - targetAttribute + 1);
        refuse(missingTargetType, movedFile.generic_string(), "TargetType",
               "every external table entry requires a TargetType attribute");

        std::string unknownTargetType = canonical;
        const auto unknownType = unknownTargetType.find("TargetType=\"") + std::string_view("TargetType=\"").size();
        unknownTargetType.replace(unknownType, Profile::XnaTypeName.size(), "No.Such.Type");
        refuse(unknownTargetType, movedFile.generic_string(), "No.Such.Type",
               "an unregistered external TargetType is refused before fixups are applied");

        std::string emptyFilename = canonical;
        const auto externalStart = emptyFilename.find("<ExternalReference ");
        const auto filenameStart = emptyFilename.find('>', externalStart);
        const auto filenameEnd = emptyFilename.find("</ExternalReference>", filenameStart);
        if (externalStart == std::string::npos || filenameStart == std::string::npos || filenameEnd == std::string::npos)
            throw std::runtime_error("Missing external filename entry");
        emptyFilename.erase(filenameStart + 1, filenameEnd - filenameStart - 1);
        refuse(emptyFilename, movedFile.generic_string(), "cannot be null",
               "a referenced external table entry cannot have an empty filename");

        std::string inlineFilename = canonical;
        const auto primaryStart = inlineFilename.find("<Primary>");
        const auto primaryContent = primaryStart == std::string::npos
            ? std::string::npos
            : primaryStart + std::string_view("<Primary>").size();
        const auto primaryEnd = inlineFilename.find("</Primary>", primaryContent);
        if (primaryStart == std::string::npos || primaryEnd == std::string::npos)
            throw std::runtime_error("Missing Primary wrapper");
        inlineFilename.replace(primaryContent, primaryEnd - primaryContent, "..\\profiles\\guard.xml");
        refuse(inlineFilename, movedFile.generic_string(), "invalid XmlNodeType",
               "an external filename cannot be written inline in its member wrapper");

        std::string unusedEntry = canonical;
        const auto externalsEndForUnused = unusedEntry.find("  </ExternalReferences>");
        if (externalsEndForUnused == std::string::npos) throw std::runtime_error("Missing external table end");
        unusedEntry.insert(externalsEndForUnused,
            "    <ExternalReference ID=\"#Unused\" TargetType=\"CnaDemos.External.Profile\">"
            "..\\profiles\\unused.xml</ExternalReference>\n");
        std::unique_ptr<System::Xml::XmlReader> unusedReader(System::Xml::XmlReader::CreateFromString(unusedEntry));
        const auto unusedLoaded = IntermediateSerializer::Deserialize<Scene>(*unusedReader, movedFile.generic_string());
        check(unusedLoaded && unusedLoaded->primary && unusedLoaded->other && unusedLoaded->empty &&
              unusedLoaded->primary->getFilenameProperty() ==
                  (movedRoot / "profiles" / "guard.xml").generic_string(),
              "an unreferenced valid external table entry is ignored without changing resolved wrappers");

        std::string reversed = canonical;
        const auto resourcesStart = reversed.find("  <Resources>");
        const auto resourcesEndMarker = reversed.find("  </Resources>\n", resourcesStart);
        const auto externalsStart = reversed.find("  <ExternalReferences>", resourcesStart);
        const auto externalsEndMarker = reversed.find("  </ExternalReferences>\n", externalsStart);
        if (resourcesStart == std::string::npos || resourcesEndMarker == std::string::npos ||
            externalsStart == std::string::npos || externalsEndMarker == std::string::npos)
            throw std::runtime_error("Missing combined document sections");
        const auto resourcesEnd = resourcesEndMarker + std::string_view("  </Resources>\n").size();
        const auto externalsEnd = externalsEndMarker + std::string_view("  </ExternalReferences>\n").size();
        const std::string resourcesBlock = reversed.substr(resourcesStart, resourcesEnd - resourcesStart);
        const std::string externalsBlock = reversed.substr(externalsStart, externalsEnd - externalsStart);
        reversed.replace(resourcesStart, externalsEnd - resourcesStart, externalsBlock + resourcesBlock);
        refuse(reversed, movedFile.generic_string(), "Resources",
               "ExternalReferences cannot precede Resources in a combined document");
        return passed ? 0 : 1;
    }
    catch (const std::exception& error)
    {
        std::cerr << "External XML reference workflow failed: " << error.what() << '\n';
        return 1;
    }
}
