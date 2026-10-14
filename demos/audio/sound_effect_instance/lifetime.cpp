// SPDX-License-Identifier: MS-PL
// CNA C++ audio wrappers: scope destruction, moving a voice and explicit disposal.
#include <string>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/ObjectDisposedException.hpp"

using namespace Microsoft::Xna::Framework::Audio;
using Microsoft::Xna::Framework::TitleLocation;

namespace
{
    void InstanceLifetime(CnaDemos::DemoContext& ctx)
    {
        const std::string path = TitleLocation::Path() + "/assets/sounds/tone_660.wav";
        // The temporary C++ SoundEffect wrapper is destroyed at this semicolon. The instance
        // retains the audio data: releasing a wrapper differs from explicitly calling Dispose.
        SoundEffectInstance survivor = SoundEffect(path).CreateInstance();
        survivor.setIsLoopedProperty(true);
        survivor.setVolumeProperty(0.2f);
        survivor.Play();
        ctx.Check(!survivor.getIsDisposedProperty() && survivor.getStateProperty() == SoundState::Playing,
                  "an instance retains playable data after its temporary SoundEffect wrapper is destroyed");

        // Moving a live voice transfers its track, not a second independently playing voice.
        SoundEffectInstance moved(std::move(survivor));
        ctx.Check(survivor.getIsDisposedProperty() && !moved.getIsDisposedProperty() &&
                  moved.getStateProperty() == SoundState::Playing,
                  "moving a playing instance transfers ownership and leaves the source disposed");
        moved.Dispose();
        moved.Dispose();
        ctx.Check(moved.getIsDisposedProperty(), "explicit instance disposal is idempotent");
        bool refused = false;
        try { moved.Play(); }
        catch (const System::ObjectDisposedException& error)
        {
            refused = true;
            ctx.Print("Disposed voice refused: {}", error.what());
        }
        ctx.Check(refused, "a disposed instance cannot be played again");
        bool resumeRefused = false;
        bool passiveCallsSafe = true;
        try { moved.Resume(); }
        catch (const System::ObjectDisposedException&) { resumeRefused = true; }
        try
        {
            moved.Pause();
            moved.Stop();
        }
        catch (...) { passiveCallsSafe = false; }
        ctx.Check(resumeRefused && passiveCallsSafe && moved.getIsDisposedProperty() &&
                      moved.getStateProperty() == SoundState::Stopped,
                  "disposed Resume refuses resurrection while Pause and Stop remain safe cleanup calls");

        SoundEffect oldAssignmentOwner(path);
        SoundEffect newAssignmentOwner(path);
        SoundEffectInstance assigned = oldAssignmentOwner.CreateInstance();
        SoundEffectInstance assignmentSource = newAssignmentOwner.CreateInstance();
        assignmentSource.setIsLoopedProperty(true);
        assignmentSource.Play();
        assigned = std::move(assignmentSource);
        ctx.Check(assignmentSource.getIsDisposedProperty() && !assigned.getIsDisposedProperty() &&
                      assigned.getStateProperty() == SoundState::Playing,
                  "move assignment replaces the destination with the source's live voice");
        oldAssignmentOwner.Dispose();
        ctx.Check(!assigned.getIsDisposedProperty() && assigned.getStateProperty() == SoundState::Playing,
                  "disposing the destination's former parent does not reach the assigned voice");
        newAssignmentOwner.Dispose();
        ctx.Check(assigned.getIsDisposedProperty() && assigned.getStateProperty() == SoundState::Stopped,
                  "disposing the source parent follows move assignment and disposes the transferred voice");

        SoundEffect effect(path);
        SoundEffectInstance first = effect.CreateInstance();
        SoundEffectInstance idle = effect.CreateInstance();
        first.setIsLoopedProperty(true);
        first.setVolumeProperty(0.2f);
        first.Play();
        SoundEffectInstance active(std::move(first));
        ctx.Check(active.getStateProperty() == SoundState::Playing &&
                  idle.getStateProperty() == SoundState::Stopped,
                  "one effect has both a moved playing instance and an unplayed instance");
        // Dispose is an explicit cascade, including an instance whose C++ address changed.
        effect.Dispose();
        ctx.Check(effect.getIsDisposedProperty() && active.getIsDisposedProperty() && idle.getIsDisposedProperty(),
                  "SoundEffect.Dispose disposes both its moved active voice and its idle voice");
        effect.Dispose();
        ctx.Check(effect.getIsDisposedProperty(), "explicit effect disposal is idempotent too");
        ctx.Print("Ownership and playback states checked; audio quality requires listening.");
    }
}

CNA_DEMOS_CONSOLE(InstanceLifetime, {
    .id = "audio.sound_effect_instance.lifetime",
    .title = "Sound data ownership and explicit disposal",
    .description = "Keeps a voice after its temporary C++ sound wrapper is destroyed, exercises move construction "
                   "and assignment across parent cascades, and checks idempotent and post-dispose transport behavior.",
    .apis = {"SoundEffect::SoundEffect", "SoundEffect::CreateInstance", "SoundEffect::Dispose", "SoundEffect::IsDisposed",
             "SoundEffectInstance::SoundEffectInstance", "SoundEffectInstance::Play", "SoundEffectInstance::State",
             "SoundEffectInstance::Pause", "SoundEffectInstance::Resume", "SoundEffectInstance::Stop",
             "SoundEffectInstance::Dispose", "SoundEffectInstance::IsDisposed", "SoundEffectInstance::IsLooped"},
    .tags = {"audio", "instance", "lifetime", "ownership", "cnaext"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_660.wav"},
})
