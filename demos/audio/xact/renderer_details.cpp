// SPDX-License-Identifier: MS-PL
// AudioEngine::RendererDetails: the audio outputs XACT can play through, and choosing one.
#include <any>
#include <chrono>
#include <string>
#include <thread>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioCategory.hpp"
#include "Microsoft/Xna/Framework/Audio/AudioEngine.hpp"
#include "Microsoft/Xna/Framework/Audio/RendererDetail.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundBank.hpp"
#include "Microsoft/Xna/Framework/Audio/WaveBank.hpp"
#include "System/ArgumentNullException.hpp"
#include "System/InvalidOperationException.hpp"
#include "System/IO/FileNotFoundException.hpp"
#include "System/TimeSpan.hpp"

using namespace Microsoft::Xna::Framework::Audio;

namespace
{
    void RendererDetails(CnaDemos::DemoContext& ctx)
    {
        bool emptySettingsRefused = false;
        bool missingSettingsRefused = false;
        bool corruptSettingsRefused = false;
        try { AudioEngine unexpected(""); }
        catch (const System::ArgumentNullException&) { emptySettingsRefused = true; }
        try { AudioEngine unexpected("assets/xact/missing.xgs"); }
        catch (const System::IO::FileNotFoundException&) { missingSettingsRefused = true; }
        try { AudioEngine unexpected("assets/xact/tones.xwb"); }
        catch (const System::InvalidOperationException&) { corruptSettingsRefused = true; }
        ctx.Check(emptySettingsRefused, "an empty settings path is an ArgumentNullException");
        ctx.Check(missingSettingsRefused, "a missing settings file is a FileNotFoundException");
        ctx.Check(corruptSettingsRefused, "an existing non-XGS file is an InvalidOperationException");

        // An engine lists the outputs; a game shows the friendly names and remembers the id.
        AudioEngine probe("assets/xact/demo.xgs");
        const auto& details = probe.getRendererDetailsProperty();
        for (const RendererDetail& detail : details)
        {
            ctx.Print("renderer \"{}\" (id \"{}\")", detail.getFriendlyNameProperty(), detail.getRendererIdProperty());
        }
        ctx.Check(!details.empty(), "at least one audio renderer is listed");
        if (details.empty())
        {
            return;
        }
        ctx.Check(!details[0].getFriendlyNameProperty().empty() && !details[0].getRendererIdProperty().empty(),
                  "a renderer has a friendly name and an id");
        const RendererDetail selected = details[0];
        const std::string chosen = selected.getRendererIdProperty();
        const std::string friendly = selected.getFriendlyNameProperty();
        ctx.Check(selected.Equals(details[0]) && selected.Equals(std::any(details[0])) &&
                      selected == details[0] && !(selected != details[0]) &&
                      selected.GetHashCode() == details[0].GetHashCode() && selected.ToString() == friendly,
                  "a copied renderer has typed and boxed identity, equal hash and friendly diagnostic text");
        ctx.Check(!selected.Equals(std::any{}) && !selected.Equals(std::any(chosen)),
                  "renderer equality rejects empty and differently typed boxes");
        probe.Dispose();
        ctx.Check(selected.getFriendlyNameProperty() == friendly && selected.getRendererIdProperty() == chosen,
                  "the copied renderer record remains valid after its discovery engine is disposed");

        AudioEngine fallback("assets/xact/demo.xgs", System::TimeSpan::FromMilliseconds(500),
                             "renderer-that-does-not-exist");
        ctx.Check(!fallback.getIsDisposedProperty() && fallback.GetCategory("SFX").getNameProperty() == "SFX",
                  "CNA accepts an arbitrary renderer id and look-ahead through its single-backend fallback");

        // The engine for the game, created for the chosen renderer (look-ahead 250 ms).
        AudioEngine engine("assets/xact/demo.xgs", System::TimeSpan::FromMilliseconds(250), chosen);
        WaveBank waves(&engine, "assets/xact/tones.xwb");
        SoundBank sounds(&engine, "assets/xact/tones.xsb");
        sounds.PlayCue("Beep");
        engine.Update();
        ctx.Check(sounds.getIsInUseProperty(), "a cue plays on the engine created for that renderer");
        for (int i = 0; i < 20; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            engine.Update();
        }
    }
}

CNA_DEMOS_CONSOLE(RendererDetails, {
    .id = "audio.xact.renderer_details",
    .title = "Audio renderers",
    .description = "Checks AudioEngine settings-file failures, lists RendererDetails (friendly name and id of each "
                   "output), exercises CNA's arbitrary-renderer fallback, and plays through the selected renderer.",
    .apis = {"AudioEngine::RendererDetails", "RendererDetail::FriendlyName", "RendererDetail::RendererId",
             "RendererDetail::Equals", "RendererDetail::GetHashCode", "RendererDetail::op_Equality",
             "RendererDetail::op_Inequality", "RendererDetail::ToString", "RendererDetail",
             "AudioEngine::AudioEngine", "AudioEngine::Dispose", "SoundBank::PlayCue"},
    .tags = {"audio", "xact", "device", "output"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"xact/demo.xgs", "xact/tones.xwb", "xact/tones.xsb"},
    .restrictions = "CNA has one audio backend (SDL3_mixer): it accepts the renderer id and the look-ahead time "
                    "but plays through the default output either way.",
})
