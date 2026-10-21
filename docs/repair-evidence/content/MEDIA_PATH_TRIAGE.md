# Media path reference triage

2026-10-20. Read-only follow-up while the separate root-batch builds/tests run.
Neither015 nor028 is closed or repaired by this entry.

Follow-up: genuine WaveBank path oracle now14/14PASS; current native streaming
probe6/6PASS, strict warning-free build and four native root-artifact hashes exact.
The initial reference compile refused direct access to internal TitleLocation;
the next refused GameTime without the Game assembly. Both setup logs remain. The
final fixture uses reflection through the already referenced Framework Curve type,
without changing any path/exception expectation. Reports
`repair-wave-bank-path-reflection-framework-xna-*` and `repair-wave-bank-path-native-*`.
No native audio engine is created in the genuine probe: inspected AudioEngine's
static constructor only initializes its managed dictionary/lock. The native probe
uses dummy audio and valid bank metadata without playing a cue. These are limited
path witnesses, not full audio or physical-hardware validation. Earlier preparation
notes below are historical; the unmeasured lazy-path concern still requires triage.

## Reference provenance

The installed genuine XNA4 Framework assembly has SHA-256
`8b7aa4c4939f243b21432747281cc8aacdcda56191a16d9eaa036b4136cf0da4`,
MVID `34D977AE-C3EC-4D07-AA6D-6FED6D8E3864`. Xact has SHA-256
`6824bb0b7b42626d1ed5b7ab7e4dab4a380fa010175d4de0fadb1c3904e491d1`,
MVID `8D274BE4-2C96-4001-8E0A-77D4B131C309`. Both are the4.0 GAC assemblies
in the private `.wine-cna-xna40` reference environment.

An older `/tmp/xna-framework-4.il` has a different MVID and was not silently treated
as this installed revision. Fresh dumps are `build-probe/repair-media-reference/`.
Framework monodis exits134 on a later mixed-mode data shape; that failure and its
partial output are retained. The complete targeted MediaSong_FromUri method and
matching module identity are present before the failure. Xact disassembly exits0.
Reports `repair-media-reference-{framework,xact}-disassembly.log`.

## ISSUE-015

### Accepted repair, 2026-10-21

CNA `9639707991b589914d8db8224b6d79f3867d60d0` contains only the015 path fix,
public comments and native regressions. Pending011 ownership changes in the same
Song files were excluded by selective staging and remain uncommitted/unaccepted.
Validation used that explicitly dirty shared worktree; it is not a clean global
campaign acceptance or a closure of011.

Native31828:10882=10781PASS/101sameSKIP, every10874 prior outcome preserved plus
eight newPASS; media311PASS, focused247 cases without failures, fresh-process
string/typed cold-title probes PASS. Native build147276ms, full1228182ms, compare124ms;
source hashes unchanged;peak4297228288/high943/max/OOM0.

External23738 hit its driver's30-minute build limit after683/1001 steps (1800106ms,
SIGTERM/ETIMEDOUT, no compiler diagnostics). No runtime cases or final audit were
emitted. Logs retained, process tree confirmed exited.79454 resumed incrementally
without changing sources or expectations and with a90-minute build allowance:
build917304ms/zero diagnostics, strict original probe2231ms and unchanged8/8PASS41ms;
cold probe build1388ms/string39ms/typed38ms PASS. All112 audio/media/C comparisons
match058 exactly except the one added play_song identity check (now6PASS on both
renderers). Each renderer still has46PASS/9historicalFAIL/1manual NOT RUN. C labels,
failed assertions and knownIssue values unchanged. Frozen319 artifacts and sources
unchanged;peak2254340096/all memory events0. Reports `repair-song-uri-path-external-v2-*`.

Registered49056: native9PASS3621ms and demos16PASS11881ms, exact JUnit audits,
unchanged hashes;peak211771392/all events0. Reports `repair-song-uri-path-ctest-v2-*`.
All heavy runs serial, shared ccache, existing trees, private GPU,6GiB max/no swap.
No new sanitizer claim for this path-only change, no physical audio/other-platform
claim, no new full719-demo/visual/global CTest claim. Historical notes below remain.

Native31828 completed0: full10882=10781PASS/101unchangedSKIP; every10874 previous
outcome retained, exactly eight newPASS. Full1228182ms, compare124ms, frozen inputs
unchanged;peak4297228288/high943/max/OOM0. Media311PASS; focused247 cases have no
failures. External23738 is now RUNNING. The real play_song demo now supplies a
relative URI and adds one title-file identity check; all five old playback checks
remain. Full external112 renderer parity and registered CTests are still pending.

Native gate31828 update, 2026-10-21 06:29 UTC: build147276ms PASS, strict
cold-title probe build1643ms and fresh-process string44ms/typed43ms both PASS.
Media311PASS11821ms; broader focused gate PASS2064ms. Full native regression is
still running; no full-suite pass or final memory/hash audit claimed yet.
The default-title probe is committed in dc12e06. Older preparation/run-state
notes below are chronological evidence, not the current state.

Focused candidate23666 completed0: existing cna_shared rebuild103408ms, then the
unchanged eight-case baseline fixture8/8PASS (build2858ms/run43ms/XML22ms), zero
diagnostics. Frozen sources intact; expected library rebuild deltas recorded;
peak1353715712/all memory events0. Wider native31828 is now running with an added
fresh-process cold-title witness before full regression. No closure yet.

Current native baseline (2026-10-21):11469 completed the strict eight-case probe
against CNA a9d74c023 plus the separately uncommitted011 candidate. Six relative
title/cwd/decoy cases fail and both absolute controls pass; all inputs unchanged.
Build2196ms/no diagnostics, run35ms, XML audit20ms;peak867409920/all events0.
Reports `repair-song-uri-path-before-*`. This now confirms OPEN015 at runtime.

A local repair now uses the existing lower-layer title bridge and platform base
path for relative URI input, preserving absolute paths and the existing empty/
invalid-path refusal. It adds no runtime dependency to media. The exact eight-case
fixture is copied to top-level cross-module tests. The existing typed-relative
case keeps its name and assertions but moves there to configure public TitleLocation;
its expected handle becomes title-rooted, as the measured contract requires.
The two plain-path factory controls now use their existing asset's absolute path,
so they do not accidentally depend on a cwd-resolution bug. No tests are dropped.
Candidate build/unchanged-fixture validation23666 is RUNNING; no pass/closure yet.
Song.cpp/Song.hpp also contain pending011 changes: stage only015 hunks later.

2026-10-21 follow-up: `../media/SongFromUriPathTests.cpp` is a prepared,
NOT-YET-COMPILED eight-case fixture (four cases for each string/typed overload).
It separates title-only, cwd-only, duplicate-name decoy and absolute-path cases,
without playback. The current genuine assembly's complete MediaSong_FromUri IL
at RVA0x206a0 explicitly calls TitleLocation.Path and Path.Combine for a relative
URI; local FNA has the same combination. Do not confuse this with028's cwd contract.

Implementation constraints: media is below runtime. The existing core
`CNA/Internal/TitlePath.hpp` bridge and platform filesystem base path are how
ContentManager obtains this information without a runtime dependency. Preserve
that layering if repairing Song. Public TitleLocation-based regression is a
cross-module integration test, not a reason to add runtime to cna_media's link
closure. Existing Song tests that compare a relative raw handle need reference-
justified adjustment rather than retaining the wrong cwd resolution; preserve
their actual file/name/exception assertions. No such adjustment is made yet.

The installed framework's MediaSong_FromUri checks Uri.IsAbsoluteUri. For a relative
URI, its complete managed method obtains TitleLocation.Path and combines it with the
URI text before passing the path to native media creation. Current CNA Song.cpp's
scheme-less path branch still passes the path directly to the constructor and has
a stale comment denying a title-root concept for Song. This supports the historical
finding; the dedicated current native decoy reproduction and actual media reference
execution remain to be performed. No audio/playback/hardware pass is claimed.

## ISSUE-028: historical expectation needs correction evidence

Prepared during37318, NOT compiled/run: `WaveBankLargeOffsetProbe.cpp` is a
Linux sparse-file safeguard against narrowing streaming offsets to signed32 IO.
It moves only the existing small fixture's wave-data segment beyond2GiB, requires
less than1MiB physically allocated, and checks lazy metadata residency plus equal
decoded duration. No2GiB buffer is allocated. This must be measured before choosing
an IO-layer migration; Sharp Runtime Stream currently exposes intcs positions and
length, whereas the existing native XWB path uses64-bit stream offsets. Do not
introduce a large-bank regression while repairing file identity. No claim about
other platforms or reference-XNA large-bank execution is made by preparation.
Its prepared driver is `build-probe/repair-wave-bank-large-offset-before.mjs`;
run only after37318 and its final external/CTest gates finish, with the same
one-worker/private-display/6GiB limit. Native integration must reuse the existing
WaveBankTestAccess definition in WaveBankTests.cpp (or extract one shared header),
not link token-different friend-seam definitions from these standalone probes.

55931 completed both follow-up native baselines on2026-10-21, serially AFTER the
accepted038/073 native-v4 gate. Retained-file probe: strict compile1449ms/run113ms,
4PASS/2FAIL exactly; both renamed and renamed-plus-replaced absolute paths fail
their first lazy read, while initial preparation and unchanged-path controls pass.
Peak365621248/events0, hashes unchanged. Reports `repair-wave-bank-retained-file-before-*`.
This proves that resolving an absolute filename alone cannot fix the observed
streaming identity defect; the opened file itself must survive until disposal.
Linux rename evidence does not assert Windows sharing/deletion behavior.

Constructor contract fixture: strict compile2704ms/run45ms/XML21ms,14cases=
5PASS/9FAIL exactly, no skips;peak476516352/events0, hashes unchanged. Reports
`repair-wave-bank-constructor-before-*`. Both overloads fail the four relative/
invalid-header contracts; streaming also fails missing-absolute-file refusal.
Both null-engine/empty-name controls and the nonstreaming absolute-missing control
pass. These are product defect reproductions, NOT successful product tests.
The genuine fourteen-case constructor oracle remains unchanged. No028 repair yet.

2026-10-21 preparation during the unrelated native audio guard regression:
`WaveBankConstructorContractTests.cpp` mirrors the fourteen genuine constructor
path/refusal controls with a real minimal native AudioEngine. It is NOT compiled
or run. Both constructors must resolve cwd rather than substitute a title file,
preserve the absolute missing filename, reject a bad header with ArgumentException,
and validate null engine/empty name before file lookup. The dedicated prepared
`repair-wave-bank-constructor-before.mjs` will require the completed native-v4
gate and audit all fourteen cases; its hypothesized nine failures are not evidence
until execution. This is distinct from the retained-file lazy-read probe below.
Do not change the shared AudioEngine/SoundBank title helper merely to repair
WaveBank; their contracts require separate evidence. No production changes yet.

Prepared `WaveBankRetainedFileProbe.cpp`, NOT compiled/run: absolute-path control
plus rename and rename-with-invalid-replacement cases, six checks, existing read
seam and real valid XWB. This separates stream identity from cwd resolution.
Genuine WaveBank keeps `pFileStream = File.OpenRead(fullPath)` until Dispose,
whereas current CNA closes the metadata stream and reopens sourcePath later.
An absolute-path-only repair might therefore leave the same root lifetime issue.
No conclusion from the unrun probe yet; rename semantics are platform-specific
and this witness does not claim Windows runtime compatibility.

8353 completed the lazy-path witness on2026-10-21: strict compile1680ms/run114ms,
exactly6PASS/2FAIL, unchanged source/artifact hashes,peak357634048/all events0.
All four banks are initially prepared. The relative changed-cwd and relative
decoy-cwd lazy reads fail; unchanged-cwd and absolute-path reads pass. Reports
`repair-wave-bank-lazy-path-before-*`. This confirms a real retained-path defect
separate from the disproved title-root expectation. No production028 repair yet;
retain the original oracle and original historical description.

Prepared follow-up `WaveBankLazyPathProbe.cpp` (2026-10-21), NOT compiled/run:
four valid streaming-bank constructions, then first access to an entry through
the existing native WaveBankTestAccess seam. It separates unchanged cwd, absolute
path plus cwd change, relative path plus empty cwd, and relative path plus an
invalid same-name decoy. Every construction and lazy decode is asserted (eight
checks); no playback. Driver `repair-wave-bank-lazy-path-before.mjs` requires the
completed015 native gate and records the suspected two failures explicitly.
This is preparation, not evidence that the lazy-path defect is reproduced yet.

Both genuine Xact WaveBank constructors first call Path.GetFullPath on the supplied
filename, then the same managed CheckWaveBankHeader. The streaming overload then
opens that full path with File.OpenRead before native handle creation. There is no
TitleLocation combination in either constructor. Thus the historical assertion
that XNA requires a streaming bank under the title directory is unsupported; source
evidence instead points to cwd resolution, unlike Song.FromUri.

`WaveBankPathOracle.cs` is prepared to measure this at the genuine constructors'
managed pre-native boundary. It uses a non-null uninitialized engine token with
finalization suppressed and only missing/invalid-header files, so no native engine
or playback is needed. It tests relative cwd/title decoys, absolute paths, exact
exceptions/filenames and argument ordering for both overloads. Not run yet. Do not
claim healthy audio-engine initialization, valid bank parsing, streaming or offset
behavior from this deliberately limited reference seam.

`WaveBankPathProbe.cpp` is also prepared, not run: six current-CNA streaming
controls stage an existing valid bank under separate title/cwd roots, inspect
prepared state and exercise absolute paths/title overrides without playback.
It must use dummy audio and the existing XACT assets. Missing-file exception timing
is deliberately not claimed equal: CNA currently logs an unavailable streaming
bank while genuine constructors throw before native resource creation.

One additional source-only concern needs separate minimization before any conclusion:
CNA's case-insensitive path resolver preserves an already-existing relative spelling,
and the streaming decoder reopens its saved source path on lazy wave-data access.
Genuine XNA obtains an absolute path and retains an opened FileStream. A later cwd
change may therefore affect CNA's lazy read even though initial cwd resolution is
correct. Do not confuse this unmeasured lifetime/path concern with the incorrect
historical title-root expectation, and do not close028 from static evidence alone.
