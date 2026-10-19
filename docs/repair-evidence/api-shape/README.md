# Passing-demo counterexamples: current API shape and asset roots (009, 020, 029, 150)

A passing associated demo is not evidence that these findings are fixed. The
haptic demos use optional construction, the sound-effect demo uses std::ifstream,
and the graphics-profile demo does not invoke the standalone Runtime facade.
Fresh probes against current CNA `1ad0a3916` confirm all three remain open.

- `HapticDefaultConstructionProbe.cpp` fails in the public inline default
  constructor's unique_ptr destruction path: IPlatformHapticDevice is incomplete.
  Report: `repair-haptic-default-before-build.log`. No haptic hardware is required
  to reproduce this compile-time defect.
- `SoundEffectStreamProbe.cpp` fails to convert System::IO::Stream to std::istream&.
  The only current public FromStream overload still has the latter signature.
  Report: `repair-sound-effect-stream-before-build.log`. No playback is claimed.
- Existing `tools/repro_runtime_facade.cpp` compiles but cannot link any of its
  five Runtime methods against current libcna.so. Current modules contain no
  definitions. Report: `repair-runtime-facade-before-build.log`.

Compilation uses C++23/Wall/Wextra/Werror and the actual native configuration's
include/definition paths; the first two probes use syntax-only compilation, the
facade probe performs the real link. No code fix or final disposition is claimed.

`CompiledSongRootProbe.cpp` additionally confirms020 against the same current
native product and existing rebuilt demo assets: ContentManager rooted at
assets/media successfully loads music/theme with a positive duration, but a
manager rooted at assets fails to load media/music/theme. The diagnostic says
music/theme.ogg was not found beside it, even though the same file was just loaded
successfully by the narrower-root control. The loader still calls
BuildAssetPath(reference) without the compiled asset's location. Strict build
passes; run fails exactly at the parent-root case. Reports:
`repair-compiled-song-root-before{,-build}.log`. No assets were changed and no
playback or graphics device was needed. XNA-reference/format migration decisions
must still be verified before the content repair.
