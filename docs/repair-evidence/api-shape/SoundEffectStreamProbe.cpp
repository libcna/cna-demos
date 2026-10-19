// SPDX-License-Identifier: MS-PL
// ISSUE-029: XNA's SoundEffect stream and TitleContainer stream have the same type.
#include "Microsoft/Xna/Framework/Audio/SoundEffect.hpp"
#include "System/IO/Stream.hpp"
#include <memory>
void ProbeSoundEffectStream(System::IO::Stream& stream)
{
    const std::unique_ptr<Microsoft::Xna::Framework::Audio::SoundEffect> sound(
        Microsoft::Xna::Framework::Audio::SoundEffect::FromStream(stream));
}
