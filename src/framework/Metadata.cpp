// SPDX-License-Identifier: MS-PL
#include "CnaDemos/Metadata.hpp"

namespace CnaDemos
{
    const std::vector<NeedName>& AllNeeds()
    {
        static const std::vector<NeedName> needs = {
            {Need::ThreeD, "3d"},
            {Need::CustomEffects, "custom-effects"},
            {Need::Audio, "audio"},
            {Need::Keyboard, "keyboard"},
            {Need::Mouse, "mouse"},
            {Need::GamePad, "gamepad"},
            {Need::Touch, "touch"},
            {Need::Microphone, "microphone"},
            {Need::Network, "network"},
            {Need::Window, "window"},
            {Need::Video, "video"},
            {Need::Instancing, "instancing"},
            {Need::MultipleRenderTargets, "multiple-render-targets"},
            {Need::Texture3D, "texture3d"},
            {Need::OcclusionQuery, "occlusion-query"},
            {Need::StencilBuffer, "stencil"},
            {Need::WireFrame, "wireframe"},
            {Need::MultiSampling, "multisampling"},
            {Need::CompiledEffects, "compiled-effects"},
            {Need::FloatRenderTargets, "float-render-targets"},
            {Need::AnisotropicFiltering, "anisotropic-filtering"},
            {Need::IndirectDraw, "indirect-draw"},
            {Need::ComputeShaders, "compute-shaders"},
        };
        return needs;
    }

    std::vector<std::string_view> NeedNames(Need needs)
    {
        std::vector<std::string_view> names;
        for (const auto& [need, name] : AllNeeds())
        {
            if (HasNeed(needs, need))
            {
                names.push_back(name);
            }
        }
        return names;
    }

    std::string_view ToString(DemoKind kind)
    {
        switch (kind)
        {
            case DemoKind::Console: return "console";
            case DemoKind::Game: return "game";
            case DemoKind::Standalone: return "standalone";
        }
        return "unknown";
    }

    std::string_view ToString(Automation automation)
    {
        switch (automation)
        {
            case Automation::Automated: return "automated";
            case Automation::SmokeOnly: return "smoke-only";
            case Automation::Manual: return "manual";
        }
        return "unknown";
    }

    std::string_view DemoEntry::Category() const
    {
        const std::string_view id = info.id;
        return id.substr(0, id.find('.'));
    }

    std::string_view DemoEntry::Subcategory() const
    {
        const std::string_view id = info.id;
        const auto first = id.find('.');
        const auto last = id.rfind('.');
        if (first == std::string_view::npos || first == last)
        {
            return {};
        }
        return id.substr(first + 1, last - first - 1);
    }

    std::string DemoEntry::Executable() const
    {
        return kind == DemoKind::Standalone ? StandaloneExecutableName(info.id) : std::string();
    }

    std::string StandaloneExecutableName(std::string_view id)
    {
        std::string name = "cna-demo-";
        for (const char c : id)
        {
            name.push_back(c == '.' ? '-' : c);
        }
        return name;
    }
}
