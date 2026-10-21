# ISSUE-099: current reference preparation

## Candidate validation, 2026-10-21

**Latest:** all native/external jobs finished. The original099 C++27 checks pass
on both renderers; five unchanged C contract assertions regress on both, and
pure-C VideoSmoke fails. Candidate remains unaccepted pending the owner's C choice.
See [final candidate checkpoint](VIDEO_REPAIR_VALIDATION_CHECKPOINT.md), including
the separately retained log-parser correction; no product assertion was relaxed.

Native-v2 session86932 has now finished: full10,934 =10,833PASS/101unchangedSKIP,
1,328,138ms; comparison121ms. Exact original10,905 outcome parity, with the same
reference-corrected Volume rename and29 newPASS. Media340/340PASS. Sources stayed
unchanged; peak3,127,988,224 bytes/all memory events0. External session37654 is
running the strict original29/112/pure-C gate. No C contract decision or final
acceptance yet; all original C expectations and knownIssue markers remain.

92340 after-v2 completed0: original guards9/lifetime6/association9 plus destroyed1/
tracks4 allPASS. Build61433ms/zero diagnostics, unchanged hashes,
peak1226051584/events0.4221 sanitizer-v2 completed0:29/29PASS, libraries12500ms/
strict fixtures14043ms/run399ms/XML23ms, unchanged hashes,peak629182464/events0.
Actual Video/VideoPlayer/runtime ASan/UBSan/float-cast/leak instrumentation remains;
no suppressions/findings, third-party instrumentation still not claimed.
86932 native-v2 build45964ms/zero diagnostics, media340/340PASS11990ms; full10934
RUNNING. External112 and the unresolved C contract remain; no closure.
Copy audit: native Validation/Lifetime fixtures differ from their preserved demo
sources only by one extra trailing LF (5574/5573 and4168/4167 bytes). Their complete
content before trailing whitespace is identical; no input/assertion changed.
Destroyed/TrackPreference copies are byte-identical (2272 and2663 bytes). The
standalone gates compile the preserved demo originals, not the copies.

15439 full completed0:10929=10828PASS/101unchangedSKIP,1416421ms, compare123ms.
Every old10905 outcome is retained with the one documented Volume test rename;
exactly24 newPASS. Hashes unchanged;peak4295897088/high61/max0/OOM0. This is the
pre-follow-up candidate checkpoint, not final099 acceptance or C API validation.

63091 follow-up then reproduced the missed original C++ destruction contract:
one test FAIL because GetTexture throws after its Video is destroyed. The same
strict fixture batch reproduced ISSUE-100's three failures/one passing control.
Build3302ms/run342ms/XML20ms, hashes unchanged,peak459255808/events0. Reports
`repair-video-followup-before-*`; these expected defect results are not product
passes. No original demo assertion was changed. The local candidate now records
independent C++ Video destruction separately from fresh/failed Play, preserving
the existing null-frame result and resetting that state when a new Play begins.
Its new exact witness also protects replay, failed replacement and disposed refusal.
92340 `repair-video-player-after-v2` is running29 strict cases after this adjustment
and the related100 selection repair. Source inputs frozen; no final acceptance.

41878 integrated sanitizer completed0: all24 cases PASS, library build17306ms/
strict fixtures12219ms/run472ms/XML30ms. Actual Video/VideoPlayer translation units
and Sharp Runtime are instrumented with ASan/UBSan/float-cast-overflow/leaks;
third-party libraries are not. No suppressions/findings, hashes unchanged,
peak1118015488/all memory events0. Reports `repair-video-player-sanitizer-*`.
15439 native build79272ms/zero diagnostics and media335/335PASS14598ms; full
regression remains RUNNING, not accepted yet.

Read-only oracle review additionally found the pre-existing pause_and_loop check
requiring GetTexture==null after a C++ owner destroys its Video. Genuine XNA GC
cannot expose that situation; this is a previously passing C++ lifetime extension,
not permission to weaken a test. `VideoDestroyedFrameTests.cpp` preserves that
conjunction and adds initial fresh refusal/real-play controls; it is UNRUN until
the serial full native job finishes. The current guard appears to reject this
state as well, so candidate acceptance remains pending a measured reproduction.

The owner question is now expanded to the complete C contract: video.h also
documents clamped Volume, and standalone/c_api.media.video_player expects Stop
to clear Video and the frame. No C code/docs/test expectations have been changed.
Whether to align these routes with XNA/C++ or preserve explicit C adapter
differences is awaiting the owner's response, separately from011's handle choice.

25907 focused candidate completed0: build80477ms/zero diagnostics; original nine
guards PASS (compile2606/run54/XML30ms), original six lifetime cases PASS
(3016/428/31ms), nine new native copy/multiple-player/resource association cases
PASS (3533/460/30ms). Frozen source hashes unchanged; peak1191071744 bytes,
all memory events0. Reports `repair-video-player-after-*`. No closure yet.
The candidate separates Stop from decoder destruction, retains observed Video
identity after Stop/Dispose, detaches replaced/destroyed players safely, and adds
the genuine-XNA guards without changing public method signatures. Private Video
association copies are deliberately not copied; assignment preserves destination
observers. Sanitizer and broader regression remain required.

Current installed Video assembly disassembly `video-current-v2.il` confirms Stop
RVA0x2644 retains decoder/activeVideo and GetTexture RVA0x26a8 refuses absent video
or invalid decoder. Dispose(bool) RVA0x2250 releases native/texture resources while
retaining activeVideo. First monodis attempt lacked Graphics assembly resolution
and exited139; the successful repeat supplied the pinned dependencies via MONO_PATH.
This is static reference evidence, not execution of genuine native GetTexture
after Stop or genuine pixels. Local FNA's null-implementation NRE is not XNA's
InvalidOperationException and must not be described as such.

**Owner choice pending:** `modules/c-api/include/CNA/C/video.h` explicitly promises
SUCCESS/no frame before Play and `VideoSmoke.c` asserts it. The C++ repair changes
what the existing wrapper observes. Asked whether to align C with XNA/C++ using
INVALID_STATE and explicit docs/tests corrections, or retain the documented C-only
extension. No reply yet, no C API edit or claim of C compatibility. Earlier039
RootDirectory approval does not authorize this separate contract decision.

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
