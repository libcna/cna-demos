# ISSUE-099: current reference preparation

## Current native baselines, 2026-10-21

95880 ran serially after the accepted038/073 gates: guard fixture9=5FAIL/4PASS,
exactly the predicted named failures, no skips; strict build2231ms/run47ms/XML25ms,
hashes unchanged,peak420233216/events0. The preceding028 large-offset witness passed.
This confirms current CNA guards, not a fix. Reports `repair-video-player-validation-before-*`.

The first lifetime fixture failed strict compilation on an unbraced GoogleTest
macro (dangling-else/Werror), build1798ms, no runtime/no final audit. Original source
is retained as `build/linux-full/reports/repair-video-player-lifetime-before-source.cpp`,
SHA256 `76b1088d81d5325b6a83678c469978b3530f9efae4a8671efd14c148a9f2d515`,
with its original compile log. Braces-only correction69241 (`...lifetime-before-v2-*`)
then reproduced6 cases=4FAIL/2PASS exactly: Stop retention, Dispose retention,
retained-player-first destruction's identity assertion, and destruction of the
replaced video. Strict build2180ms/run302ms/XML23ms; hashes unchanged,
peak445419520/events0. Both active-video destruction and replay controls pass.
Every case proves actual CNA playback/160x90 texture creation first. This is not
a sanitizer, pixel-exact visual or genuine-XNA native playback result.

The replacement ordering concern below is now confirmed: destroying the first
Video after Play(second) stops the second player's current playback. Combined
with retained identity, this needs correct observer detachment rather than simply
removing `video_ = nullptr`. A single active `parent_` cannot protect several
retained players or safely copy association state into a different C++ Video.
Keep public API unchanged if a local private relationship repair suffices; test
copy and multi-player lifetime safety as well as the original six cases.
No099 production repair yet. Earlier preparation statements below are history.

Prepared during37318 (NOT compiled/run): `VideoPlayerLifetimeTests.cpp` adds six
real-decoder cases using CNA's existing synthetic `chroma_420.mkv`, with initial
Playing/identity/160x90 texture checks. They cover retained identity after Stop
and Dispose, both destruction orders, replacement of a playing video, active
video destruction, and replay. Four failures/two controls are a source-derived
hypothesis, not runtime evidence. The replacement case additionally checks the
old video's back-reference: `Play` currently assigns `video_` before `OpenDecoder`
closes the old decoder, so closing detaches the incoming rather than old video.
Do not treat this as confirmed until the reproduction runs. The prepared driver
`build-probe/repair-video-player-lifetime-before.mjs` pins the four source files
and fixture bytes. Run serially after the existing external gates, before any
099 production edit. These cases also need later instrumented lifetime validation;
a plain native run alone is not an ASan result. No lifetime architecture chosen.

Prepared during37318, NOT compiled/run: `VideoPlayerValidationTests.cpp` covers
nine native guard scenarios with real default-constructed players and no decoder,
device or playback. It checks exact exceptions/parameter/object names, refusal
before mutation, NaN/boundary controls, readable disposed configuration getters,
and existing transport/idempotence guards. The expected pre-fix split is five
failures/four controls; this is only a hypothesis until execution. It deliberately
does not substitute a fake Video token for actual native lifetime validation.
The existing real-video demo and future sanitizer lifetime cases remain required.
Prepared driver `build-probe/repair-video-player-validation-before.mjs` requires
the completed native-v4 gate and pins the four Video/VideoPlayer source files to
the validated622d0bd1a code. Run serially after the active038/073 external/final/
CTest gates, before any099 source change; do not treat the hypothesized split as
already reproduced.

## Genuine execution, 2026-10-21

56438 completed0 after the native full suite ended: all41 managed guard/retention
checks PASS on the exact pinned current Video/Graphics/Framework assemblies.
Strict C# build2246ms/run445ms, source hashes unchanged;peak295788544/all memory
events0 under1GiB/no swap. Reports `repair-video-player-guards-xna-*`.
This confirms the managed ordering, exact exception/object/parameter names,
retained identity and NaN boundary for the initialized-field/invalid-decoder seam.
It does NOT validate actual decoder creation/destruction, playback, texture pixels
or audio output. No099 production fix or closure is claimed. Earlier notes below
are preparation history, superseded only by this explicitly limited execution.

2026-10-21, read-only investigation while038/073 native full regression runs.
`VideoPlayerGuardsOracle.cs` is prepared, NOT compiled/run. Its41 managed checks
use explicitly populated uninitialized players with invalid decoder handles and
finalization suppressed; it never creates or plays a native decoder. Stop/Dispose
only traverse their no-decoder paths, so this does not validate real decode,
rendered textures, hardware, sound, or native resource destruction.

The current installed Video assembly is in GAC_MSIL, not GAC_32. SHA-256
`63c7673c936747abd9ebe779e8837c8b8add2c078a31216684fbf8c6bcab2745`,
MVID `6bc5f328-0528-401f-828a-1d9c05e99bfd`. This differs from the historical
ISSUE-099 recorded SHA; do not silently call it that older reference revision.
The Graphics reference assembly SHA is
`39aed7fd8e308ccace5ce9390256f1c829ca72c2eccd97d4a0f629f24015d3a5`.
The prepared fixture must pin these and the existing Framework identity when run.

Local decompiled VideoPlayer shows a narrower NaN rule than audio Pan: Volume
checks `value < 0 || value > 1`, so NaN reaches native assignment. The prepared
invalid-decoder control measures this managed boundary only, not what the actual
native decoder accepts. Do not generalize audio's explicit NaN rejection to video.

The original demo already covers real CNA playback/retained identity. A future
repair must inspect Video/VideoPlayer's C++ back-reference lifetime as well as
exception guards: keeping a raw Video pointer after Stop/Dispose must not create
a dangling pointer if its C++ owner later destroys it. No production099 change
or new current reproduction is claimed here. Do not run a parallel Wine oracle
while the ongoing full native suite owns its own reference Wine subprocesses.
