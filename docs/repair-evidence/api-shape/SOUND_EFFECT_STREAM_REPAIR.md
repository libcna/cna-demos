# ISSUE-029: SoundEffect.FromStream(Stream) repair

2026-10-23. Baseline CNA `a24c75b2e`; repair CNA `5d66bb795`. The contract is the one measured before any repair and
recorded in [SOUND_EFFECT_STREAM_TRIAGE.md](SOUND_EFFECT_STREAM_TRIAGE.md): 25/25 genuine managed
stream controls and 71/71 format/chunk/loop/duration controls, plus the decompiled
`Microsoft.Xna.Framework.Audio.WavFile` and `AudioFormat.DurationFromSize`.

## Before

`SoundEffectStreamProbe.cpp` did not compile: no conversion from `System::IO::Stream` to
`std::istream&` (`repair-sound-effect-stream-before-build.log`). The 28 prepared native cases
(`SoundEffectXnaStreamTests.cpp`, `SoundEffectXnaWaveFormatTests.cpp`) could not be compiled for
the same reason; they are installed unchanged in CNA's audio tests (only their header comment
drops "prepared").

## Repair

`SoundEffect::FromStream(System::IO::Stream&)` ports WavFile: empty source is
`ArgumentNullException("source")`; an unreadable stream is refused by `BinaryReader`; the RIFF
header and length are checked exactly; chunks are read until the first that fails, whatever the
failure; a missing or unsupported format, or data before format, is `ArgumentException`; the
`wsmp` loop wins over `smpl`, an invalid `wsmp` loop prevents the `smpl` fallback, and `smpl`'s
end is inclusive; a partial final frame is dropped. The source is closed only after a successful
parse (the reader is opened with `leaveOpen`, as XNA's is only closed on success), so every
refusal leaves it open. Duration is `AudioFormat.DurationFromSize`: a float millisecond count
rounded to whole milliseconds the way .NET Framework's `TimeSpan.FromMilliseconds` does (modern
.NET no longer rounds). 8-bit samples are widened to signed 16-bit for the mixer.

The existing `std::istream&` overload is unchanged and now marked as the CNA extension it is
(any audio file the mixer decodes; used by WaveBank and the C `create_from_encoded_ext` route).

## Validation

- Native, `cmake-build-multi`, dummy audio: the 28 cases pass 28/28 on the first build; the whole
  audio suite 803 PASS / 4 unchanged SKIP (775 + 28), no failure; warning-free build.
- `SoundEffectStreamProbe.cpp` now compiles with the native configuration's flags, `-Wall
  -Wextra -Werror`, syntax only (`repair-029-probe-after-build.log`).
- Demo `audio.sound_effect.from_stream` keeps every extension check and adds one: the stream
  `TitleContainer::OpenStream` returns loads through the new overload (0.5 s). It passes with 9
  checks on SOFTWARE and OPENGLES3; the rest of the `audio` category matches the ISSUE-010
  closure sweep per ID.
- Complete checkpoint with the demos tree rebuilt from the tree committed as CNA `8842b9a50`:
  719-ID sweeps SOFTWARE 654/14/48/3 and OPENGLES3 696/13/7/3 against the ISSUE-010 closure change
  only `sensor_lifecycle` FAIL-to-PASS and the intended check counts; serial CTest 684 PASS/
  48 SKIP/0 FAIL/3 disabled (`repair-batch3-closure-{full,ctest}*`). Closed with CNA `da7388fa9`.

No physical audio output is claimed; the dummy SDL audio driver is used, as before.
