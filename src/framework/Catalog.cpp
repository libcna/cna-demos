// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Catalog.hpp"

#include <cstdio>
#include <ostream>

#include "CNA/GraphicsRendererSelection.hpp"
#include "CNA/GraphicsRendererType.hpp"

#ifndef CNA_DEMOS_VERSION
#  define CNA_DEMOS_VERSION "unknown"
#endif
#ifndef CNA_DEMOS_CNA_VERSION
#  define CNA_DEMOS_CNA_VERSION "unknown"
#endif

namespace CnaDemos
{
    std::string_view CnaDemosVersion()
    {
        return CNA_DEMOS_VERSION;
    }

    std::string_view CnaVersion()
    {
        return CNA_DEMOS_CNA_VERSION;
    }

    std::vector<std::string> CompiledRenderers()
    {
        std::vector<std::string> names;
        for (const CNA::GraphicsRendererType type : CNA::GraphicsRendererSelection::GetAvailable())
        {
            names.emplace_back(CNA::getGraphicsRendererName(type));
        }
        return names;
    }

    std::string JsonEscape(std::string_view text)
    {
        std::string escaped;
        escaped.reserve(text.size());
        for (const char c : text)
        {
            switch (c)
            {
                case '"': escaped += "\\\""; break;
                case '\\': escaped += "\\\\"; break;
                case '\n': escaped += "\\n"; break;
                case '\r': escaped += "\\r"; break;
                case '\t': escaped += "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                    {
                        char buffer[8];
                        std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(c));
                        escaped += buffer;
                    }
                    else
                    {
                        escaped += c;
                    }
            }
        }
        return escaped;
    }

    namespace
    {
        void WriteStringArray(std::ostream& out, const std::vector<std::string_view>& values)
        {
            out << '[';
            for (std::size_t i = 0; i < values.size(); ++i)
            {
                out << (i == 0 ? "" : ", ") << '"' << JsonEscape(values[i]) << '"';
            }
            out << ']';
        }

        std::string Join(const std::vector<std::string_view>& values, std::string_view separator)
        {
            std::string joined;
            for (std::size_t i = 0; i < values.size(); ++i)
            {
                if (i > 0)
                {
                    joined += separator;
                }
                joined += values[i];
            }
            return joined;
        }
    }

    void WriteCatalogJson(std::ostream& out)
    {
        Registry& registry = Registry::Instance();
        out << "{\n";
        out << "  \"schema\": \"cna-demos-catalog/1\",\n";
        out << "  \"cnaDemosVersion\": \"" << JsonEscape(CnaDemosVersion()) << "\",\n";
        out << "  \"cnaVersion\": \"" << JsonEscape(CnaVersion()) << "\",\n";
        out << "  \"renderers\": [";
        const auto renderers = CompiledRenderers();
        for (std::size_t i = 0; i < renderers.size(); ++i)
        {
            out << (i == 0 ? "" : ", ") << '"' << JsonEscape(renderers[i]) << '"';
        }
        out << "],\n";
        out << "  \"demoCount\": " << registry.All().size() << ",\n";
        out << "  \"demos\": [";
        bool first = true;
        for (const RegisteredDemo& demo : registry.All())
        {
            const DemoEntry& entry = demo.entry;
            const DemoInfo& info = entry.info;
            out << (first ? "\n" : ",\n");
            first = false;
            out << "    {\n";
            out << "      \"id\": \"" << JsonEscape(info.id) << "\",\n";
            out << "      \"title\": \"" << JsonEscape(info.title) << "\",\n";
            out << "      \"description\": \"" << JsonEscape(info.description) << "\",\n";
            out << "      \"category\": \"" << JsonEscape(entry.Category()) << "\",\n";
            out << "      \"subcategory\": \"" << JsonEscape(entry.Subcategory()) << "\",\n";
            out << "      \"kind\": \"" << ToString(entry.kind) << "\",\n";
            out << "      \"automation\": \"" << ToString(info.automation) << "\",\n";
            out << "      \"needs\": ";
            WriteStringArray(out, NeedNames(info.needs));
            out << ",\n      \"apis\": ";
            WriteStringArray(out, info.apis);
            out << ",\n      \"tags\": ";
            WriteStringArray(out, info.tags);
            out << ",\n      \"assets\": ";
            WriteStringArray(out, info.assets);
            out << ",\n      \"frames\": " << (entry.kind == DemoKind::Console ? 0
                                               : info.frames > 0 ? info.frames : DefaultAutomatedFrames);
            out << ",\n      \"deterministic\": " << (info.deterministic ? "true" : "false");
            out << ",\n      \"restrictions\": \"" << JsonEscape(info.restrictions) << "\"";
            out << ",\n      \"knownIssue\": \"" << JsonEscape(info.knownIssue) << "\"";
            out << ",\n      \"ownProcess\": " << (info.ownProcess ? "true" : "false");
            out << ",\n      \"source\": \"" << JsonEscape(entry.source) << "\"";
            if (entry.kind == DemoKind::Standalone)
            {
                out << ",\n      \"executable\": \"" << JsonEscape(entry.Executable()) << "\"";
            }
            out << "\n    }";
        }
        out << "\n  ]\n}\n";
    }

    void WriteInfo(const RegisteredDemo& demo, std::ostream& out)
    {
        const DemoEntry& entry = demo.entry;
        const DemoInfo& info = entry.info;
        out << info.title << '\n';
        out << std::string(info.title.size(), '=') << '\n';
        out << info.description << "\n\n";
        out << "  ID          : " << info.id << '\n';
        out << "  Category    : " << entry.Category();
        if (!entry.Subcategory().empty())
        {
            out << " / " << entry.Subcategory();
        }
        out << '\n';
        out << "  Kind        : " << ToString(entry.kind);
        if (entry.kind == DemoKind::Standalone)
        {
            out << " (executable " << entry.Executable() << ")";
        }
        out << '\n';
        out << "  Automation  : " << ToString(info.automation) << '\n';
        if (entry.kind != DemoKind::Console)
        {
            out << "  Auto frames : " << (info.frames > 0 ? info.frames : DefaultAutomatedFrames)
                << (info.deterministic ? " (deterministic output)" : "") << '\n';
        }
        out << "  CNA APIs    : " << Join(info.apis, ", ") << '\n';
        if (info.needs != Need::None)
        {
            out << "  Needs       : " << Join(NeedNames(info.needs), ", ") << '\n';
        }
        if (!info.tags.empty())
        {
            out << "  Tags        : " << Join(info.tags, ", ") << '\n';
        }
        if (!info.assets.empty())
        {
            out << "  Assets      : " << Join(info.assets, ", ") << '\n';
        }
        if (!info.restrictions.empty())
        {
            out << "  Restrictions: " << info.restrictions << '\n';
        }
        if (!info.knownIssue.empty())
        {
            out << "  Known issue : " << info.knownIssue << " (docs/CNA_ISSUES.md) -- expected to fail\n";
        }
        if (info.ownProcess)
        {
            out << "  Process     : its own (once-per-process CNA state); batch runs start a new cna-demos\n";
        }
        out << "  Source      : " << entry.source << '\n';
        out << "  Run         : cna-demos " << info.id << '\n';
    }
}
