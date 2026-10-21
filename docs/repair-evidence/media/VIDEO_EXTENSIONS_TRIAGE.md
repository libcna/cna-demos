# ISSUE-100/101: Video extension follow-up

Latest:92340 after-v2 all29 focusedPASS (including all four unchanged100 cases),
4221 sanitizer-v2 all29PASS without findings; 86932 native-v2 media340/340PASS
and full10934 ongoing. Frozen hashes unchanged on completed gates. See the099
triage for exact timing/memory. The external public-pixel reproduction is still
required;100 remains CONFIRMED_OPEN, not closed on decoder-rate tests alone.

2026-10-21, read-only source/contract review while099 native regression15439 runs.
No100/101 production repair or current focused baseline is claimed yet.

**Follow-up result:**63091 now reproduced the four prepared100 cases exactly:
threeFAIL/onePASS, plus099's separate destruction regression. Combined strict
build3302ms/run342ms/XML20ms; hashes unchanged,peak459255808/events0. This follows
15439's full10929 checkpoint, whose old outcomes and101skips all match. Reports
`repair-video-followup-before-*`. The local100 candidate selects each nonnegative
Video preference ahead of the player's fallback, before output setup, without
overwriting player defaults.29 focused cases are now running in92340, including
the original24, destruction1 and tracks4. No100 closure/commit yet; the original
public-pixel external oracle and full regression remain required.101 is untouched.
The no-FFmpeg registration requirement below has been implemented for the four
real-decoder fixtures; pure guard tests remain included. The OFF test now requires
the measured fresh InvalidOperationException before and after refused Play.
No actual OFF-profile runtime pass is claimed.

## Track preferences (100)

Local FNA `b35512475ed7980169574d2c40927381c1764d5a`, Video.cs151-195 and
VideoPlayerTheora.cs277-362, stores per-Video preferences and explicitly applies
them ahead of playback, giving nonnegative per-Video settings priority. The local
checkout has only unrelated untracked .idea/.junie directories. Current official
[extension documentation](https://fna-xna.github.io/docs/5%3A-FNA-Extensions/#setaudiotrackext-setvideotrackext),
checked2026-10-21, recommends selection before Play. Audio wave-format differences
are permitted before playback but not during it; video streams must retain equal
dimensions and YUV format. This is FNA extension evidence, not genuine XNA API.

CNA stores Video preferences but OpenDecoder only reads the separate player fields.
The last accepted external checkpoint has exactly the known detached-video pixel
failure on both renderers; the eight other controls remain PASS. Reproduce against
the current099 candidate before changing that path. Preserve existing player-level
preselection; a Video override must not accidentally replace the player's default
for later unrelated Videos.

Prepared `VideoTrackPreferenceTests.cpp`, NOT compiled/run: four native audio
selection tests use CNA's existing multi_track_audio.mkv and existing read-only
VideoPlayerTestAccess. ffprobe confirms its tracks are mono48000 and mono44100;
all format changes are selected before Play. Three failure families/one passing
control are a source-derived hypothesis, not a result. No physical/audio-output
claim: these measure the decoder's actual selected rate. The original external
track_matrix demo remains the public-pixel video oracle (equal160x90/YUV420).

`repair-video-followup-before.mjs` is prepared to run these four and the separate
099 destruction-contract witness serially AFTER15439 finishes, before any further
Video production edit. It requires the full audit and freezes sources/fixtures.

## URI/deferred errors (101)

Local FNA Video.FromUriEXT resolves file URIs through LocalPath, relative URIs
through TitleLocation, and refuses non-file absolute URIs. VideoPlayerTheora.Play
throws FileNotFoundException if the native decoder fails to open the path.
CNA FromUriEXT forwards the spelling as a raw path and OpenDecoder silently
returns after failed open. The accepted external checkpoint still shows four
named101 failures and five controls on each renderer. A focused current baseline,
URI decoding/path controls and failure ownership review are still needed.

Do not silently change the public C Play contract: video.h explicitly documents
undecodable input as a successful stopped result. That is distinct from the099
owner question; decide whether missing-file refusal can be corrected while retaining
the documented existing-but-undecodable extension. Do not conflate absence with
unsupported codecs or widen the repair before checking evidence.

## Cross-profile test integration requirement

New native real-decoder099 fixture files are not yet listed in UnitTests.cmake's
existing no-FFmpeg exclusion block. Add only real-decoder fixtures to that block;
keep pure guard tests enabled. VideoBackendAvailabilityTests' OFF branch also
contains the old fresh GetTexture==null expectation; correct it from the same XNA
evidence rather than silently leaving an untested configuration inconsistent.
No decoder-free runtime validation has been performed in this continuation.
