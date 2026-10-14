// SPDX-License-Identifier: MS-PL
// CNA C++ audio wrappers: scope destruction, moving a voice and explicit disposal.
#include <string>
#include <utility>

#include "CnaDemos/Demo.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundEffectInstance.hpp"
#include "Microsoft/Xna/Framework/Audio/SoundState.hpp"
#include "Microsoft/Xna/Framework/TitleLocation.hpp"
#include "System/InvalidOperationException.hpp"
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
        bool volumeRefused = false;
        bool pitchRefused = false;
        bool panRefused = false;
        try { moved.setVolumeProperty(0.8f); }
        catch (const System::ObjectDisposedException&) { volumeRefused = true; }
        try { moved.setPitchProperty(0.5f); }
        catch (const System::ObjectDisposedException&) { pitchRefused = true; }
        try { moved.setPanProperty(0.5f); }
        catch (const System::ObjectDisposedException&) { panRefused = true; }
        ctx.Check(volumeRefused && pitchRefused && panRefused && moved.getVolumeProperty() == 0.2f &&
                      moved.getPitchProperty() == 0.0f && moved.getPanProperty() == 0.0f,
                  "disposed Volume, Pitch and Pan setters refuse writes and preserve their values");
        bool loopMutationRefused = false;
        try { moved.setIsLoopedProperty(false); }
        catch (const System::InvalidOperationException&) { loopMutationRefused = true; }
        ctx.Check(loopMutationRefused && moved.getIsLoopedProperty(),
                  "a played-and-disposed instance retains the one-way IsLooped mutation lock");

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

        SoundEffect parentMoveSource(path);
        SoundEffectInstance parentMoveChild = parentMoveSource.CreateInstance();
        parentMoveChild.setIsLoopedProperty(true);
        parentMoveChild.Play();
        SoundEffect parentMoveTarget(std::move(parentMoveSource));
        ctx.Check(!parentMoveTarget.getIsDisposedProperty() && !parentMoveChild.getIsDisposedProperty() &&
                      parentMoveChild.getStateProperty() == SoundState::Playing,
                  "move construction preserves a SoundEffect resource and its active child");
        parentMoveTarget.Dispose();
        ctx.Check(parentMoveChild.getIsDisposedProperty() && parentMoveChild.getStateProperty() == SoundState::Stopped,
                  "disposing the moved-to SoundEffect still cascades to its original child");

        SoundEffect replacedParent(path);
        SoundEffect incomingParent(path);
        SoundEffectInstance replacedChild = replacedParent.CreateInstance();
        SoundEffectInstance incomingChild = incomingParent.CreateInstance();
        replacedChild.setIsLoopedProperty(true);
        incomingChild.setIsLoopedProperty(true);
        replacedChild.Play();
        incomingChild.Play();
        replacedParent = std::move(incomingParent);
        ctx.Check(replacedChild.getStateProperty() == SoundState::Playing &&
                      incomingChild.getStateProperty() == SoundState::Playing &&
                      !replacedParent.getIsDisposedProperty(),
                  "move assignment transfers the incoming parent without explicitly disposing the replaced wrapper");
        replacedParent.Dispose();
        ctx.Check(incomingChild.getIsDisposedProperty() && !replacedChild.getIsDisposedProperty() &&
                      replacedChild.getStateProperty() == SoundState::Playing,
                  "the assigned parent cascades only to the incoming resource's registered child");
        replacedChild.Dispose();

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
    .description = "Keeps voices across C++ wrapper and parent/child moves, verifies disposal cascades, and checks "
                   "idempotent post-dispose transport and property behavior.",
    .apis = {"SoundEffect::SoundEffect", "SoundEffect::CreateInstance", "SoundEffect::Dispose", "SoundEffect::IsDisposed",
             "SoundEffectInstance::SoundEffectInstance", "SoundEffectInstance::Play", "SoundEffectInstance::State",
             "SoundEffectInstance::Pause", "SoundEffectInstance::Resume", "SoundEffectInstance::Stop",
             "SoundEffectInstance::Dispose", "SoundEffectInstance::IsDisposed", "SoundEffectInstance::IsLooped",
             "SoundEffectInstance::Volume", "SoundEffectInstance::Pitch", "SoundEffectInstance::Pan"},
    .tags = {"audio", "instance", "lifetime", "ownership", "cnaext"},
    .needs = CnaDemos::Need::Audio,
    .assets = {"sounds/tone_660.wav"},
})
