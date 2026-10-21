# ISSUE-101 URI dependency and CNA candidate

## Latest outcome (supersedes running states below)

**Runtime reference confirmed:** VideoFileFailureOracle.cs compiled with genuine
.NET4 csc /platform:x86 /warnaserror+ in3,018ms and passed32/32checks in620ms.
It calls the genuine XNA pre-decoder guard for null/empty/relative-missing/
absolute-missing/directory/embedded-NUL inputs, each independently verified
not to be a file before invocation. All return E_INVALIDARG; the actual public
Play HRESULT translator maps/throws exact ArgumentException, null ParamName.
Two success controls pass. No decoder/player/texture was created, no existing
file was submitted, and no native playback or arbitrary codec error is claimed.
Framework MVID and all pinned reference/source hashes unchanged. Peak278,560,768
bytes/events0 under1GiB/no swap. Reports: repair-video-file-failure-xna-*.
Thus the earlier FNA-based FileNotFound expectation and proposed C IO result
are not genuine-XNA-compatible for this guard. Preserve their history and
correct them explicitly when this video batch resumes; do not silently weaken
exception assertions or conflate the CNA URI factory extension with XNA Play.
No production/video/demo assertion edits during the frozen shadow gates.

**Reference caution discovered during read-only follow-up,15:40 CEST:** the
genuine Framework IL's WmpInterface.VideoDecoder_Play checks File.Exists before
native handle lookup and returns E_INVALIDARG (0x80070057) for a missing/non-file
path. Helpers.GetExceptionFromResult maps that code to exact ArgumentException,
not FNA's FileNotFoundException. VideoPlayer.Play calls this error translator
after VideoDecoder_Play. The current installed Framework module MVID matches
the inspected IL:34d977ae-c3ec-4d07-aa6d-6fed6d8e3864. This is source/IL evidence,
**not yet a runtime reference result**. VideoFileFailureOracle.cs and its driver
are prepared/unrun; execute serially after active shadow native45760. It invokes
only the pre-decoder guard on verified non-files and the actual HRESULT mapper,
never a valid file/decoder/texture/GPU. If confirmed, correct the exact missing-
file exception expectation according to XNA precedence, preserving the original
FNA finding/history. Do not treat the existing green FileNotFound assertion as
proof of XNA compatibility. The earlier101 owner question's proposed IO mapping
must be revisited with that evidence, not silently assumed. No expectation or
production video change was made during frozen shadow validation.

External-v2 session68581 completed with exit1: **gate not accepted**.
Existing linux-full build passed in2,576,494ms with zero compiler diagnostics;
43 original witnesses passed against its actual libcna.so (compile13,362ms,
run938ms, XML audit23ms). Each renderer, SOFTWARE and OPENGLES3, completed56
original scenarios:50PASS/5FAIL/1manual NOT RUN. Original099 C++27,100 pixel9
and101 URI9 now pass on both. Four unrelated failing scenarios and every
unrelated assertion outcome are unchanged. C VideoPlayer still fails the same
five assertions on each renderer: fresh GetTexture, fresh frame descriptor,
Volume clamping, Stop Video identity and stopped frame absence. Those failures
are **not** converted to knownIssue or expected failures. Pure-C VideoSmoke
compiles348ms but exits2 in237ms; its log does not identify the failing assertion,
so do not infer a precise failure line from that exit alone.

All frozen sources/artifacts unchanged. Scope peak4,302,946,304 bytes;
memory.high3924 (reclaim/throttling), max0/oom0/oom_kill0. One compiler worker,
4GiB high/6GiB hard cap/no swap, private GPU. Original reports retained under
`build/linux-full/reports/repair-video-uri-external-v2-*`.

099/100/101 remain open; C contract choices are still unanswered and actual
decoder-open failure is a101 follow-up. No C source/assertion/marker edit,
production CNA commit, issue closure, push or merge. Continue independent102
while those decisions remain pending; the campaign is not blocked as a whole.

2026-10-21. Starting revisions: CNA ba56ddf5a, demos ca6310b, Sharp Runtime
97713eb292093b4d81f7e29c1965d5a317a57680. No issue is closed by this checkpoint.
The earlier VIDEO_REPAIR_VALIDATION_CHECKPOINT.md remains the historical099/100
gate with rejected C regressions. Those C assertions and public contracts are
unchanged; the owner's099 choice remains unanswered. No new push or merge.

## Reproduction and reference

Original URI9:5FAIL/4PASS against native-v2. New file-boundary5: allFAIL against
the same old native library and its actual Sharp Runtime IO archive; strict build
2025ms, run240ms, audit22ms. The initial standalone link attempt failed because
the old shared library did not export unused File::Exists; it was not a runtime
test. The corrected attempt links the actual existing archive, without changing
any assertion. Logs preserve both attempts.

`UriLocalPathOracle.cs` was built by genuine Framework4 csc and run on that CLR
under Wine and .NET8.0.31/Linux:34 cases each; build2883ms, runs640/157ms,
unchanged source/compiler hashes, peak308244480 bytes and no memory events.
It classifies inputs only, with no network/file access. Ordinary escaped file
paths, NUL preservation, DOS/UNC and relative guards agree. Actual differences:
Framework4 decodes malformed UTF-8 differently, retains a file query in LocalPath
and normalizes some backslashes that modern Linux .NET retains. New runtime
getters follow measured.NET8/Linux plus local.NET11-preview source. This is not
an all-versions or native-Windows parity claim.

FNA b35512475ed7980169574d2c40927381c1764d5a provides FromUriEXT's file/relative/
non-file behavior. The factory is an FNA extension, not an XNA Video API.
The candidate adds no CNA URI parser: it uses System::Uri, File::Exists and the
existing TitleLocation bridge. Native absolute strings keep literal percent signs.
Trusted metadata construction still performs no IO; missing-file Play detaches
the failed association and throws FileNotFoundException. Existing but undecodable
files still take the pre-existing silent decoder-failure path: **not** a claim of
complete FNA codec-failure parity. The C Play contract needs separate review.

The101 C contract conflict is concrete, not inferred from099: pure_c/VideoSmoke.c
173–190 explicitly requires `http://example.com/v.mp4` to return CNA_RESULT_IO,
and docs/c-api/MEDIA.md describes the factory as deliberately forwarding URI text
without parsing. video.h's Play comment also promises success/stopped for a file
the platform cannot decode. The owner has now been asked separately whether to
align URI resolution/refusal and missing-file Play with corrected FNA/C++, or
preserve these C differences in an adapter. Neither099 nor101 has an explicit
answer yet. No C source, header, specification, smoke or demo assertion was edited.
An existing-but-undecodable file remains distinct from an absent file in the
candidate; do not silently expand this fix to every codec-open failure.

Root-cause follow-up before101 closure: the File.Exists precheck does not itself
propagate an actual decoder-open failure (including a file disappearing between
check and open). FNA VideoPlayerTheora.cs304–307 and VideoPlayerAV1.cs225–228 throw FileNotFoundException when
their decoder open fails. Review/minimize that broader path and the documented C
success/stopped difference; the green URI9 must not be mistaken for proof that
every deferred-open failure is repaired. No additional codec behavior changed
during the frozen external run.

## Completed runtime gates

| Gate | Result |
| --- | --- |
| Baseline before-v2 |18,158/18,158 PASS across41 executables; URI311; build101ms/full50743ms |
| URI candidate after |18,192/18,192 PASS; URI345; build143545ms/full49168ms |
| URI + File.Exists after-v2 |18,193/18,193 PASS; URI345; build27807ms/full49314ms |
| Module graph |44 physical modules,109 dependency edges; valid |

Zero build diagnostics, test failures or skips in all accepted gates. The first
baseline attempt had five fixture-path skips because its default samples path was
stale. Accepted reruns use XNA_SAMPLES_ROOT with a symlink view of the unchanged
original ShipGame/Spacewar samples. No skip allowance or source-fixture edit.
Existing SOAP harness remains private loopback. Latest runtime peak830410752
bytes, all memory events0, frozen input hashes unchanged. One worker/shared cache,
existing build/, no temporary build tree. Migration/source/version boundaries and
downstream spelling inventory are in runtime docs/Migration-UriLocalPathAndFileExists.md.

## CNA validation sequence (historical running states retained)

**Completed sanitizer:**97309 terminated0: libraries57,823ms, strict fixture
compile22,209ms,43PASS501ms, XML inspection27ms. Actual Video/VideoPlayer,
Sharp Runtime and canonical TitleLocation object instrumented; no ASan/UBSan/
float-cast/leak findings or suppressions. Third-party libraries are not claimed
instrumented. Frozen hashes unchanged,peak920498176 bytes/all memory events0.
External43227 stopped before build because native audit retained old ASan
archives; v2 explicitly takes the successful sanitizer's updated ASan hashes
and rejects any changed native source/artifact.68581 external-v2 now builds
and runs43 original witnesses/112 scenarios/pure-C smoke; gate remains pending.

**Completed native-v3:**59093 terminated0. Build18,503ms, focused43PASS1,037ms,
media345PASS12,715ms, full10,948=10,847PASS/101unchangedSKIP1,341,053ms.
Comparison136ms confirms all prior10,934 outcomes unchanged and exactly14newPASS.
No build diagnostics; frozen hashes unchanged; peak2,626,609,152 bytes and all
memory events0. Native Linux/OPENGLES3, private Wayland and network, no hardware
or other-platform claim. Sanitizer97309 now runs the43 witnesses with the scoped
TitleLocation object approach described below. External gate still pending.

Latest: native-v3 job59093 completed build18,503ms/zero diagnostics, focused43/43
PASS1,037ms and media345/345PASS12,715ms. Full10948 is RUNNING, not a completed
result. Previous14068 stopped before build: its source guard found libcna.so
rebuilt at step52 of18544. The driver now records that intentional build-artifact
replacement and rehashes it after rebuilding; unrelated source hashes are still
required to match. No assertion change or retry of a test failure is involved.

Update: runtime repair is local commit c26614e85dffff4cadc5c4803b00d78953706b21,
clean runtime worktree, no push. Native18544 failed linking CnaMediaTests after
324516ms: the new public TitleLocation witness belongs to cross-module tests,
not the isolated media link closure. Moved only its native copy to
tests/MediaVideoUriContractTests.cpp, preserving all assertions/public calls;
no production dependency expansion. The standalone original remains in this
evidence directory. Native-v2 job14068 now runs43focus through CnaTests,
345 media (original340 plus5boundary), and10948 full. Full inventory unchanged
by the relocation. Prepared sanitizer uses the canonical CnaTests link closure
for the cross-module fixture. Original failure preserved, not treated as a pass.

Native job18544, cna-repair-video-uri-native.scope, is the only heavy job. It
builds CnaTests/CnaMediaTests in cmake-build-debug, then requires43 focused tests,
354 media tests and full10948 exact outcome parity against the prior10934.
Private GPU/network/owner UID/no capabilities;4GiB high/6GiB max/no swap; one worker.
These counts are planned, **not yet a runtime result**. Sources are frozen.

The original URI9 witness now uses PathToGenericUtf8 instead of generic_string
for filesystem→API text, avoiding Windows ANSI narrowing. Linux input bytes and
all assertions are unchanged; native and evidence copies match. Existing
FromUriEXT local-file tests now provide an explicit absolute fixture path; they
still test the same actual file, independent of global TitleLocation left by
earlier tests. New relative-URI witness protects title-vs-current-directory behavior.

Prepared, not run: repair-video-uri-sanitizer.mjs (43 original witnesses, actual
instrumented Video/runtime); repair-video-uri-external.mjs (actual external
library,43 witnesses,112 unchanged scenarios, pure-C smoke). C regressions still
reject the external gate; no new expected failure is introduced.

Sanitizer preparation refinement: the existing ASan tree has no umbrella
libcna.so or runtime archive. Rather than build unrelated modules just for this
fixture, the driver retains the canonical Media static link closure and requests
the real CMake object target for modules/runtime/src/TitleLocation.cpp. Its
instrumentation flags/source/header/object hashes are checked, and that object
is linked into the43-case witness. No stub, copied implementation, new build tree
or production dependency change. This supersedes the initially prepared whole
CnaTests link-closure approach above; still unrun while native59093 is active.

Reports: build/linux-full/reports/repair-uri-local-path-oracle-*,
repair-uri-runtime-{before,before-v2,after,after-v2}-*, repair-video-uri-before-*,
repair-video-file-path-before-{*,v2-*}, repair-video-uri-native-*.

## Accepted gates and current continuation

Native-v3 completed: 10,948 cases = 10,847 PASS / 101 unchanged SKIP.
All previous 10,934 outcomes are unchanged and exactly 14 new cases pass.
Build 18,503 ms; focused 43/43 PASS in 1,037 ms; media 345/345 PASS in
12,715 ms; full suite 1,341,053 ms; parity comparison 136 ms. Frozen hashes
unchanged, peak 2,626,609,152 bytes, all memory events zero.

Sanitizer gate completed: all 43 cases PASS with actual Video/VideoPlayer and
runtime ASan/UBSan/float-cast/leak instrumentation, no findings or suppressions.
Libraries 57,823 ms, strict witnesses 22,209 ms, run 501 ms, audit 27 ms;
peak 920,498,176 bytes, all memory events zero, frozen hashes unchanged.
This does not claim third-party codec instrumentation.

Sharp Runtime dependency repair is committed as
`c26614e85dffff4cadc5c4803b00d78953706b21`; its worktree was clean at the
checkpoint. No push. External-v2 session 68581 is still running, rebuilding
the existing linux-full tree with one compiler worker before 43 witnesses,
112 original renderer scenarios and unchanged pure-C VideoSmoke. The first
external attempt stopped before building on superseded sanitizer artifact
hashes; v2 merges only accepted ASan artifacts, preserving all native input
checks. No source/expectation changes during the frozen run.

Root-cause follow-up before ISSUE-101 closure: the File.Exists precheck does
not itself propagate an actual decoder-open failure, including a file
disappearing between check and open. FNA VideoPlayerTheora.cs lines 304–307
and VideoPlayerAV1.cs lines 225–228 throw FileNotFoundException when decoder
open fails. Minimize that
broader path and review the documented C success/stopped difference; green
URI checks do not prove every deferred-open failure repaired. No additional
codec behavior changed during the frozen external run.

Next: inspect the external gate, preserve exact C regressions, then continue
the independent ISSUE-102 safe setter-boundary reproduction while the two
explicit C video contract choices remain unanswered. Preserve inherited 011
and every historical ID. 099/100/101 remain open; matrix 155 IDs / 97 final /
58 pending. No campaign completion, new closure, push or merge.
