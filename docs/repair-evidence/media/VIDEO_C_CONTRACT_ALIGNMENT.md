# ISSUE-099/101 — owner-approved C contract alignment

2026-10-21. Owner approved099 XNA/C++ alignment and101 URI alignment, then
explicitly approved the genuine-XNA ArgumentException / INVALID_ARGUMENT
correction, superseding the earlier FNA FileNotFoundException / IO proposal.
This document records intentional oracle corrections and the accepted local repair;
native Windows codec/error timing remains explicitly unverified.

## Current validation (2026-10-21)

CNA repair commit: `abbd8a4ea7ae3f511e0f3aac3faaba411f6de0a4`.
ISSUE-099 and100 are FIXED_VALIDATED. ISSUE-101 is
FIXED_PLATFORM_VALIDATION_PENDING: its URI/missing-file fixes pass here, while
the genuine native Windows codec reference still requires the environment
described in VIDEO_CODEC_REFERENCE_LIMITATION.md. Provisional
VideoDeferredOpenFailureTests.cpp remains UNRUN and unregistered, not an oracle.

Accepted checkpoint:

- Native62083:53focused/355mediaPASS; full10,885PASS/101sameSKIP, exact old10,982
  outcomes plus4PASS. Full1,869,465ms, compare159ms; no frozen input drift/OOM.
- Sanitizer63740:53PASS, no ASan/UBSan/float-cast/leak findings; actual production
  Video/VideoPlayer/TitleLocation/Sharp Runtime instrumented, not third-party code.
- External62468: build147,275ms/zero diagnostics; actual53PASS; all112 exact
  comparisons accepted. C player33, C++ guards27, tracks9(public pixels), URI9
  pass on SOFTWARE and OPENGLES3. Pure-C smoke uses a real decoded frame.
- Inventory22151: seven original gatesPASS; no approval/criterion expansion.
  ABI91302: original C17/C++23, error and bool smokePASS against external C ABI.
- Final48985: exact three knownIssue removals, two documented metadata updates;
  every assertion/unrelated catalog entry unchanged. Ten renderer cases and
  eight CTestsPASS. Rebuild26,591ms, CTest19,490ms; peak3,157,680,128/events0.

Commit review removed exactly one extra final LF from each of two native test
files (Lifetime/Validation). The staged original bytes matched the validated
hashes and the resulting files matched those bytes minus only that final LF.
No assertion/token/product change or new runtime pass is inferred. The original
final audit is retained; `repair-video-commit-whitespace.json` records the exact
two before/after hashes and carries the current frozen checkpoint forward.

The code/test commit excludes inherited011 ownership edits. Its mapping change
contains only the two video descriptions; the obsolete trait correction, two
SoundEffect pins and three dependent generated summaries remain separately dirty.
Nothing was pushed or merged. Historical failed gates below remain evidence.

## Earlier checkpoints (historical)

After native acceptance, inventory-v3 passes all seven original C API metadata
gates; see VIDEO_C_INVENTORY_REVIEW.md for retained failures and generated-file
commit boundaries. Sanitizer63740 passes all53 under ASan/UBSan/float-cast/leaks,
no findings/suppressions, unchanged hashes. Libraries17,506ms, strict fixture
39,002ms,run962ms,inspection35ms;peak1,726,898,176 bytes/all memory events0.
The initial sanitizer command failed before building due to an escaped wrapper
replacement; correcting only that driver enabled the actual run. External62468
now checks the same112 oracles, actual53 witnesses and real-frame pure-C smoke.
No further predicate/label changes or video closure are claimed.

Final metadata review still required after runtime acceptance: URI demo restriction
must distinguish FNA's URI extension from genuine XNA's deferred missing-file
exception. The C video_asset metadata must describe the newly tested failed Play
attempt (no successfully decoded frame/audio claim). Capture catalog/source before
these documentation changes and the three knownIssue removals; preserve every
runtime assertion and verify the exact catalog delta afterward.

Native62083 is now complete and accepted:53focusedPASS,355mediaPASS,
full10,885PASS/101unchangedSKIP in1,869,465ms. The159ms exact comparison retains
all10,982 previous outcomes and adds precisely the four unchanged loop witnesses
as PASS. No source/artifact drift; peak4,299,919,360 bytes,high5228/max0/OOM0.
Inventory29275 is running separately. Sanitizer53/external112 and extra C ABI
are still pending; this does not close the batch or replace historical failures.

### Loop-boundary follow-up — candidate, not accepted

External84138 finished with failure, not acceptance. Build157,279ms/zero
diagnostics; actual external49PASS; all112 original outcomes/authorized labels
unchanged except the remaining C player assertion on SOFTWARE and OPENGLES3:
`Stop retains the frame without advancing or restarting its generation`.
The other four corrected C player assertions pass. Pure-C VideoSmoke now passes
with actual decoded-frame coverage (build649ms/run366ms). Peak4,301,926,400 bytes,
memory.high6577/max0/OOM0; frozen hashes unchanged. Failed report/logs retained.

The failure exposes a genuine frame-identity defect, not an incorrect Stop
expectation: GetTexture seeks to the beginning at loop EOF and resets
`lastFramePts_` to-1 as a decode-scheduling sentinel, but still holds a texture
and its nonzero generation. GetFramePresentationTimeEXT returns that cursor,
contradicting its promise to describe the retained frame. A Stop exactly here
keeps the negative sentinel until another Play. Neither C oracle is weakened.

VideoLoopFrameIdentityTests.cpp uses the existing native test friend to set
elapsed playback beyond the authored two-second fixture; real decoding and EOF
still execute, without sleeps or retries. Baseline74000 reproduces2FAIL/2PASS:
the loop timestamp and Stop-at-loop cases fail, ordinary Stop and Dispose
controls pass. Build5562ms/run545ms/XML39ms; frozen hashes unchanged,
peak3,626,180,608/events0. This is before any product change.

The candidate separates held-frame presentation time from the scheduler cursor,
updates it only after successful frame upload, clears it when closing the
decoder and retains it when looping seeks. The four original witnesses are
registered unchanged, with a decoder-off selector. Native62083 now runs53focus,
355media and exact full10982+4 parity. Sanitizer53 and external112/pure-C must
be rerun; the earlier49-case successes do not validate this new change. The
independent C inventory failures below remain unresolved. No closure/commit.

62083 progress: build71,620ms/zero diagnostics,53focusedPASS1,350ms and
355mediaPASS12,721ms. The new loop-boundary tests now pass without changing
their baseline assertions. Full native10986 is still running; no final parity
or new sanitizer/external result is claimed yet.

Sanitizer15652 completed49/49PASS under actual ASan/UBSan/float-cast-overflow/
leak instrumentation of Video/VideoPlayer, TitleLocation and Sharp Runtime;
third-party libraries are not claimed instrumented. No findings/suppressions.
Library build15,639ms, strict fixture37,028ms, execution756ms, XML inspection36ms.
Frozen hashes unchanged; peak685,629,440 bytes, all memory events0. External84138
now runs existing linux-full incrementally, then49 witnesses linked to the actual
external libcna.so,112 SOFTWARE/OPENGLES3 scenarios and pure-C decoded-frame smoke.
It has not completed; the separate C inventory failure below remains unresolved.

Native run90148 completed:49focusedPASS,351mediaPASS, full10881PASS/101sameSKIP.
Every old10976 outcome is unchanged; exactly six new XNA missing-file/control
cases pass. Build40,919ms, focus1,324ms, media13,223ms, full1,653,247ms,
comparison260ms. Source/artifact hashes stayed fixed. Peak4,297,699,328 bytes;
memory.high5540, memory.max/OOM/OOM-kill0. This is native Linux validation,
not native Windows codec validation.

Post-review20536 verifies the real decoder-off CMake selector in both modes and
builds incrementally in23,490ms with zero diagnostics. CnaTests, CnaMediaTests
and libcna.so remain byte-identical to the accepted full run. The C header
review only relocates GetTexture documentation to its declaration and documents
handle clearing; stripped declarations are identical.

**C API metadata gate FAILED:3PASS/2FAIL.** CApiCoverageMatrix and CApiReleaseGate
reject `content-reader-detail-traits`, whose two declarations were removed by
committed3d254dfa7. A read-only actual Doxygen diagnostic additionally identifies
two stale SoundEffect move pins belonging to inherited uncommitted ISSUE-011.
No content/audio source, approval pins, release criterion or test is changed.
Keep the raw failed `repair-video-xna-postreview-ctest.log`/XML. The independent
`repair-video-xna-runtime-checkpoint.json` sets `contractAccepted=false` and
`closureBlocked=true`; it permits further sanitizer/external diagnostics, not
video closure or a passing release-gate claim. Settle inventory/ownership before
final acceptance. Earlier baseline and owner-approved corrections follow.

Baseline: CNA7b028ebe8 / demos788bf0e, Runtime c26614e85. Final070 audit pins
all prior sources/artifacts. The prior C video player has five unchanged failures
per renderer and the original pure-C smoke exits2; they have never been accepted
as expected failures. New VideoMissingFileXnaTests.cpp reproduces5FAIL/1PASS:
empty, relative-missing, absolute-missing, directory, embedded-NUL paths all
throw the wrong exact exception, while real decoding passes. Baseline85836:
build7,651ms, run554ms, inspect55ms,peak3,427,909,632/events0; hashes unchanged.
The original32 genuine-XNA guard/translator checks establish the correct exact
ArgumentException with absent ParamName. C++ represents absent ParamName as
empty, per Sharp Runtime. This does not alter raw Video/FromUri missing-file IO.

Explicit oracle changes, preserving history and all unrelated assertions:

| Surface | Previous assertion | Corrected assertion |
| --- | --- | --- |
| C fresh GetTexture | SUCCESS / no frame | INVALID_STATE; cleared handle, scalar unpublished |
| C fresh frame descriptor | SUCCESS / absent | INVALID_STATE; deterministic cleared descriptor |
| C Volume outside0..1 | clamp | INVALID_ARGUMENT; prior interior value unchanged |
| C Stop Video | cleared | same Video retained; stopped/zero position unchanged |
| C Stop frame | absent | last frame retained, generation exactly unchanged |
| C++ deferred missing Play | exact FileNotFoundException | exact ArgumentException, closed state retained |
| C non-file URI | IO literal path | INVALID_STATE scheme refusal without network |
| C absolute/relative file URI | literal filename | file decoding / TitleLocation resolution |

The original C player demo still has the same assertion count: five labels and
their corresponding predicates change above. The C asset demo retains its
missing-file factory check (renamed to describe resolved-file refusal) and adds
real absolute/relative URI and deferred-Play checks. Pure-C VideoSmoke keeps all
raw undecodable-file checks; its URI path uses the existing absolute compiled
fixture because relative URIs intentionally no longer resolve against cwd.
It additionally checks atomic range refusal and deferred missing-file mapping.
The public C routes already delegate to the corrected C++ layer; no adapter
special case or result-code remapping is introduced.

Existing invalid-codec behavior is not changed. The genuine native-decoder
observer under Wine failed its positive WMV control at MFCreateVideoMediaType;
do not claim native Windows asynchronous error/timing parity. See
VIDEO_CODEC_REFERENCE_LIMITATION.md. No video marker removed or issue closed yet.

Next: run49 focused /351 media /full10976+6 native parity, instrumented lifetime
regression and original external112 with precisely the authorized assertion
changes. Pure-C smoke must become a real pass with decoded-frame coverage.
Then review coherent product commit and marker removal, retaining inherited011.
