# Checkpoint history and earlier source research

Archived verbatim from HANDOFF.md on 2026-10-12 after checkpoint 451.
The current baseline, running work and next actions are in [HANDOFF.md](../HANDOFF.md).
Historical next-task statements and `/tmp` draft names below are superseded unless the current
handoff explicitly selects them. Preserve the evidence and source observations; do not restart
completed phases from these old notes. Search here for earlier checkpoint details and subsystem
research that remains useful when returning to that area.

Earlier COMPLETE catalog checkpoint: 446-palette at 55ebc57 (closure 2c3f095), all 459 enabled
CTests (143.36 s), SW 395/23/25/3, GL 416/24/3/3, exact prior 32/33 failed assertions and all
old C/custom-content child lists/host trace retained. two_bones passes thirteen checks; all other
old counts unchanged. Selected hashes and normalized stderr match. Original evidence/helpers:
reports/checkpoint-446-palette-*. The newer reentry checkpoint is now the baseline. Previously applied palette/clone/C drafts
are stale; the owner-dispatch draft is now applied. The newer 446-reentry runtime checkpoint is complete as described above.

A later seven-target rebuild against clean CNA 4ee1e0b553667b6e999248fcefdea729654ee497 passed;
the diagnostic compiles with GCC 14/C11/-Wall/-Wextra/-Werror. All 32 selected catalog binaries
still match 446-palette after this rebuild and probe compilation; metadata remains 446/0. The
only new native executable is the separate defect reproducer. Its final four cases each exit 1
for the explicitly confirmed defect. Renderer initialization/selection INFO stderr is preserved,
not empty; the first evidence-check script wrongly assumed silence and its correction is recorded.

Focused palette evidence: 27-target build without diagnostics, metadata 446/0, all 17 targeted
CTests (2.30 s), thirteen checks on both selections, empty stderr. Both inspected 800x480 images
are RGBA-identical to their original 422 references. Original/focused images, logs and observed
source hashes: checkpoint-446-palette-focused-* and -original-*. No new CNA issue from this demo.

Full 446 at 62b5cb8 (closure 420a099) is the completed baseline: 459 enabled CTests (153.20 s),
SW 395/23/25/3, GL 416/24/3/3, exact 32/33 failed assertions and all prior child lists retained.
Selected binary/asset hashes and normalized stderr unchanged. Evidence: checkpoint-446-*.

Clone focused evidence: build passed (51 targets; known vendored Draco CMP0148 configure
warning, no compiler diagnostics), metadata 446/0, all 42 targeted CTests passed (40 passed
including three WILL_FAIL, two capability skips; 10.46 s). Both inspected 800x480 captures are
RGBA-identical; focused stderr empty. Evidence: checkpoint-446-focused-*. Inventory 293/329
types, 1356/2326 XNA members and 158/3210 C routes; all runtime columns now include 446 IDs.
The /tmp clone draft is APPLIED AND STALE. No new CNA issue.

Focused C evidence: build passed (87 targets, no compiler diagnostics); metadata 445/0, all 31
C/CLI CTests passed (6.80 s), both renderer selections pass, all six stderr streams empty.
Host passes 28 child checks with captures (26 without); both inspected 800x480 images are
RGBA-identical to original 444 host captures. Evidence: checkpoint-445-callbacks-focused-*.
External CNA C API sources continue changing; no atomic dependency snapshot claim.

Complete 445-profile is closed at e903a2e (closure 7eea0a3): 458 enabled CTests, SW 394/23/25/3,
GL 415/24/3/3, exact original 32/33 failed assertions and all old content/host child lists retained.
The resize extension passes seven checks. All 32 selected binaries and four explicit assets
remained unchanged through runtime; normalized stderr identical. Evidence: checkpoint-445-profile-*.

The original profile-focused build passed (81 steps, no compiler diagnostics), metadata 445/0,
33 targeted CTests (11.56 s), and both seven-check runs with empty stderr. Four actually inspected
800x480 before/after screenshots are RGBA-identical. The raw source PNG is byte-identical to its
existing content fixture. XNB calls stop at ImportAndProcess; only native CNB bytes are written
and decoded, with no XNB output/Reach-hardware claim. Documentation-only ISSUE-049 adds no
expected-failure marker. The /tmp resize source/body drafts are APPLIED AND STALE.

The first full attempt (session 13537, exit 1) failed in external CNA C API ContentReaders/Cnb:
31 unused-helper diagnostics under -Werror during uncommitted generated-route work. Runtime phases
never started. Original evidence: reports/checkpoint-445-profile-initial-{build.log,validation.log,
cna-state.json}; post-failure source hashes are not an atomic build-input snapshot. Current generated
files contain direct calls to those helpers; the full retry rebuild passes without suppressing
warnings or modifying CNA. This is not a reproduced clean-HEAD or cna-demos source defect.

The C batch now verifies the three previously prepared behaviors. Invalid MasterVolume reaches
the native setter after handle dispatch; zero TargetElapsedTime is rejected locally and cannot
prove error forwarding. Canonical message/type/parameter buffers are bounded and copied on the
worker. Original C11 acquire/release and bounded timeout cleanup are unchanged. Hook-layout
negative controls use fully allocated C objects, and actual next-frame traces verify preservation.
Host run_frame_ext failure completion/documentation ambiguity remains untested.

The new graphics demo uses native Clone/SetOwnedTexture. Two independent one-bone palettes
share a non-white tint texture after the original effect/caller reference are released; exact
rendered colors/positions and weak_ptr ownership checks cover the complete intended lifetime.
An expired-texture guard prevents a dangling texture draw if ownership breaks. No explicit
Texture2D.Dispose, raw setter ownership, multi-weight blend or all-material clone claim.
The two_bones extension now covers this authored equal-weight joint's silhouette, not every
possible weight or a four-influence mesh. Palette count failures and maximum-size readback use
actual SkinnedEffect APIs; the final slot is distinct but no vertex indexes it during rendering.
Next meaningful gaps: four-influence geometry, skinned material/fog rendering or the separately
uncovered C host failure completion/queued dispatch cancellation. Avoid inferring a queued call
from only a worker's pre-call readiness flag. Existing completed phases remain the baseline.

Confirmed CNA-DEMOS-ISSUE-050: clean CNA 4ee1e0b553667b6e999248fcefdea729654ee497 was observed
before rebuild and after reproduction. generated Runtime.inc uses CallWithExceptionBarrier on
owner-only driving/destruction routes, despite runtime.h's explicit exception to dispatch.
GetCallableGame adds only a callback-reentrancy guard; ordinary handle lookup signals a retryable
foreign-thread failure. Initial and final-source probes reproduce both RunFrameEXT and Destroy
on both selections. Cancelled INVALID_STATE proves real queuing; readiness/timing alone would
not. The probe distinguishes scheduling-inconclusive cases with exit 2 and never services the
forbidden body. Other generated driving routes share the source shape but were not executed.

Reproduction commands and source analysis: docs/CNA_ISSUES.md ISSUE-050. All original/final
stdout, stderr, exits, initial source, clean-state observations and selected source/binary hashes:
reports/cna-issue-050-*. The initial final-source verification stopped because native INFO stderr
was incorrectly assumed empty, and a premature archive then lacked the remaining case logs.
All remaining cases were completed; each final stderr equals its original stream exactly. The
note preserves that tooling error. No extra CNA defect, renderer-frame validation or full atomic
transitive input snapshot is claimed. No CNA file changed or upstream message sent.

445 evidence: /tmp/cna-demos-445-validation.log exit 0; all 458 enabled CTests (206.40 s), SW
394/23/25/3, GL 415/24/3/3, exact 32/33 failed assertions unchanged, every older count/status/
knownIssue retained. Ten new native source-root checks pass; all older content tool/24 host child
lists unchanged. Normalized stderr unchanged and all 32 selected hashes match. Full evidence and
helpers: reports/checkpoint-445-*. Original focused runs/builds/source hashes/29 targeted CTests
(11.25 s): checkpoint-445-focused-*. Native diagnostics still has nine passes and now displays
its three captured messages. Private trees independently gone; focused stderr empty.
First source-root SOFTWARE run had one wrong diagnostic expectation: leading ../source/image.png
survives lexically_normal, then CNA correctly rejects its '..' segment. The corrected test still
checks Import, CNA.CnjImporter, exact source/asset and the observed refusal reason. Original failure
is checkpoint-445-first-roots-SOFTWARE.*; no CNA patch/issue. Initial build had the known Draco
CMP0148 configure warning; final/full builds had no compiler diagnostics. External C API changes
were built without claiming an atomic dependency snapshot. Coverage has complete runtime columns:
293/329 types, 1354/2326 named members, 158/3210 C routes. Semantic coverage remains incomplete.

The /tmp source-root draft is APPLIED AND STALE, including its wrong traversal diagnostic; use
repository sources. Source-root nested propagation, CLI fingerprints, symlinks and platform path
semantics remain separate. Ordinary root-map getters/alias spelling need no independent demos.

444 evidence: /tmp/cna-demos-444-validation.log exit 0; 457 enabled CTests, SOFTWARE 393/23/25/3,
GL 414/24/3/3, exact 32/33 failures retained. All older statuses/known issues and content child lists
match. Nine native diagnostic checks pass and C host frames retain seventeen + seven new checks.
All 32 selected runtime hashes match; normalized stderr unchanged (SW 25 ERROR/16 DEBUG/3 INFO;
GL 3 ERROR/16 DEBUG/3 INFO plus the same 84 X protocol lines). Full evidence/review helpers are
reports/checkpoint-444-*; original focus/build/28-targeted-CTest evidence is checkpoint-444-focused-*.
Both last-frame host images were inspected, exactly 800x480 solid (32,128,192,255), RGBA-identical
to prior captures; exact callback trace ILBUbUbDeUDUrDsXEZ. With captures the tool has 26 checks.
Native opacity source cleanup independently confirmed and both focused stderr streams empty.
Initial build had the known vendored Draco CMP0148 configure warning; final build and full build
had no compiler diagnostics. More external C API changes were built; no atomic CNA snapshot claim.

The initial SOFTWARE archive helper stopped at a repeated C child ID and reported zero child
checks. Raw logs confirmed 17/24 and the corrected parser skips identical child headers; strict
reviews now pass for both renderers. Initial helper failure is preserved in
reports/checkpoint-444-initial-archive-review.log. This was not a demo or CNA failure.
The /tmp opacity and hook drafts are APPLIED AND STALE. Repository host code captures the last
replacement-hook frame; do not reapply its draft's earlier first-draw capture. Next other gaps:
C ABI callback-failure/foreign-thread behavior. Source-only note: runtime.h's run_frame_ext prose
names exiting/end_run after failure, but CGame::Invoke suppresses later callbacks; UnloadContent
opts into runAfterFailure. No runtime reproduction or issue has been recorded for that wording.
Coverage is now complete for all 444 IDs in both supplied full runs: 293/329 types, 1354/2326
named XNA members, 158/3210 C routes. This is not comprehensive semantic coverage.

443 evidence: /tmp/cna-demos-443-validation.log exit 0; 456 enabled CTests, SOFTWARE 392/23/25/3,
GL 413/24/3/3. Both original comparisons preserve exact 32/33 failures. Graph sixteen child checks
match focused; patrol 14 -> 15. All 32 selected runtime hashes unchanged. Normalized stderr adds
exactly the thirteen focused graph lines, no removals; GL keeps 84 X protocol lines, no GTK.
Original reports/checkpoint-443-* include both streams, reviews, build/CTest logs and binary hashes.
Focused graph final /tmp/cna-443-graph-final-* has sixteen passes; initial thirteen retained;
patrol /tmp/cna-443-absence-* has fifteen. All 26 targeted CTests passed (6.10 s). Coverage now has
complete runtime columns: 293/329 types, 1354/2326 named members, 158/3210 C routes. Semantic
coverage remains incomplete; external source-root capabilities are now explicitly listed pending.
All /tmp graph/absence/publication drafts are APPLIED AND STALE. Never reapply them over the final
sixteen-check graph. Use ORIGINAL checkpoint-443 as the next full baseline.

Original /tmp/cna-443-build.log completed 110 steps including external C API changes; its automatic
configure includes vendored Draco's known CMP0148 developer warning, no compiler diagnostics.
Initial thirteen graph checks remain /tmp/cna-443-graph-* (before the -final extension). Existing
patrol's intentional XNB route absence is registered before freeze and reports its reason through
the real Selection exception; direct Cnb/Xnb reason lookups distinguish the working route.

442 evidence: /tmp/cna-demos-442-validation.log exit 0; 455 enabled CTests, SOFTWARE 391/23/25/3,
GL 412/24/3/3, exact 32/33 failures retained. Both archive wrappers /tmp/cna-archive-442.py and
/tmp/cna-442-finish.py pass; 31 selected hashes unchanged; stderr adds precisely the six focused
Publish/Graph lines. Original full evidence is reports/checkpoint-442-*; focused evidence is
reports/checkpoint-442-focused-* (14 passes both selections, 25 targeted CTests in 6.30 s).

Build evidence: /tmp/cna-442-build.log completed 108 steps; automatic configure included vendored
Draco CMP0148 developer warning, not a compiler warning/error. Final build /tmp/cna-442-final-build.log
completed 86 steps after external C API changes. /tmp/cna-442-target-build.log then confirmed the
strengthened final source up to date. Do not conflate those dependency rebuilds with demo defects.
All /tmp/cna-multi-artifact-publication* drafts are APPLIED AND STALE; use repository sources.

Previous complete checkpoint: 441-contracts at 2d28a13 (closure 89510ca); 454 enabled CTests pass,
SOFTWARE 390/23/25/3, GL 411/24/3/3, exact 32/33 failed assertions, same normalized stderr and
30 unchanged selected hashes. Original evidence under reports/checkpoint-441-contracts-*.
Contract focused evidence remains /tmp/cna-441-values-* (14 native checks) and
/tmp/cna-441-strictness-final-* (17 passes plus ISSUE-047/048); 24 targeted CTests passed.

Both /tmp/cna-pipeline-value-contracts.cpp and /tmp/cna-xna-strictness.cpp drafts/generators are
APPLIED AND STALE: repository contains the final labeled reproduction and extra returned-key
output. Do not reapply drafts. Native example distinguishes importer/processor empty/wrong stable
identities from wrong C++ payloads accepted by identity selection but rejected by the consumer Get.
The existing XNA adapter checks actual default CNB bytes, two contextual warnings, caller request
retention, valid override preservation and refusal of missing processor input. ISSUE-048 concerns
the documented effective result metadata only: both dropped keys remain. MakeContentBuildManifestEntry
copies result.parameters; cache effects are source observations only, not reproduced failures.
No CNA source edit. ISSUE-047 was accidentally below Resolved in the issue document; it is now
correctly under Open along with new ISSUE-048.

Previous complete checkpoint: 441-config at e3b8244 (closure e245a87); all 454 enabled CTests,
SOFTWARE 390/23/25/3, OPENGLES3 411/24/3/3, exact 31/32 failures and unchanged normalized stderr.
Archives reports/checkpoint-441-config-* include original streams, exact reviews and 30 hashes.

Multi-artifact publication now demonstrates native AddDeploymentFile with a source-contained
briefing and separate output root, distinct from generated-output XNA ISSUE-047. After obstruction,
actual waves=4/rewards=100 CNBs were already published while the successful manifest stayed old;
removing the final-path directory lets retry complete the set. The ownership conflict edits arena
4 -> 6 and adds bonus 5; both claim the fixed briefing destination and both get Graph refusals
without changing prior artifacts. Only after observing that does the demo remove bonus and restore
arena 4 for the unchanged-skip control. Manifest-less clean is checked before restoring its exact
saved ownership record and exercising normal clean. No whole-node rollback/crash/concurrency claim.
Next deeper gaps: actual content-build dependency scheduling/effective fingerprints/cycles, or
remaining C ABI callback lifecycle contracts. Prefer a coherent extension where it fits; no per-
getter or per-error demo proliferation. Source AddContentBuildDependency exists, but neither this
publication example nor manually supplied ContentBuildSibling controls prove CLI graph scheduling.
Completed graph work at the focused level now closes the previously pending strong-edge/effective-
fingerprint/cycle behavior gap and demonstrates two-worker scheduling with a finite overlap probe.
Only the graph's serial and two-worker invocations are covered; multiple independent compiler
processes competing for one output root, broader graph shapes and crash recovery remain separate.
Next useful member work: remaining C ABI callback lifecycle contracts, or actual content logger
forwarding/declared-loss diagnostics in an existing coherent component example. Ordinary registry
inventory/accessor views and repeated Freeze need no new demo IDs. Native absent-writer routing
now has direct evidence in the existing patrol; the related /tmp draft is stale.
Archive helpers for 442 are /tmp/cna-archive-442.py and
/tmp/cna-442-finish.py; the latter checks that added stderr equals precisely the six focused
Publish/Graph diagnostic lines after private-path normalization.

Initial config build /tmp/cna-441-config-build.log stopped in concurrently modified C API:
CnaCApiDisplay.cpp:439 lacked the qualification/using for ToNativePresentationParameters. At later
inspection, the external worktree already had the alias on line 48. No CNA edit from this task,
no claim that clean CNA 161d43b00 fails. Initial log and post-failure source/status hashes remain
reports/checkpoint-441-config-initial-{build.log,cna-state.json}; the latter explicitly is not an
atomic failed-source snapshot. Independent compiler target built and passed focused 20 checks
before full retry /tmp/cna-441-config-retry-build.log succeeded. This transient external transition
is not recorded as a new stable CNA issue. Preserve the initial failure, do not hide it.

441 focused evidence: /tmp/cna-441-xml-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}, ten child checks
per run and empty stderr. /tmp/cna-441-build.log clean, metadata 441/0 and all 22 targeted CTests pass
(/tmp/cna-441-targeted-ctest.log, 11.16 s), including all 16 C ABI demos. Full evidence is archived
as reports/checkpoint-441-*; both exact original comparisons and selected hash verification pass.

XML integration source 9995869 is built and fully validated. Focused evidence:
/tmp/cna-440-xml-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}, ten child checks each, private paths
independently absent. Clean /tmp/cna-440-build.log; metadata 440/0 and all 22 targeted CTests pass.
Originals and 29 selected hashes archived in reports/checkpoint-440-*; both exact reviews pass.
The XML-to-XNB /tmp drafts are APPLIED AND STALE; use repository sources.

441 raw/existing-instance evidence: the custom Slots serializer uses public ReadObject and
ReadRawObject with preallocated references. Default descriptors preserve their pointer identity,
authored Health and nonserialized owner tokens. Ordinary Null returns empty without filling the
supplied slot, unrelated known string Type is refused, and raw Type/Null attributes are bypassed.
An explicit field-specific compact serializer writes integer text and fills its own supplied
instance without changing the ordinary global descriptor. The same authoring reference is encoded
twice inline; no shared-resource alias preservation is claimed. All ten checks pass, including
following-field alignment after ordinary Null. No new CNA issue, file fixture, GPU, genuine-XNA or
runtime-XNB raw-reading claim. All /tmp/cna-xml-existing-* drafts are APPLIED AND STALE; use sources.

Display isolation investigation: inherited WAYLAND_DISPLAY=wayland-0, XDG_SESSION_TYPE=wayland
and DISPLAY=:0 were present; xvfb-run changes DISPLAY but does not remove Wayland selection.
With unchanged native_handle and OPENGLES3, the inherited-Xvfb probe prints **Wayland** and the GTK
warning; SDL_VIDEO_DRIVER=x11 plus env -u WAYLAND_DISPLAY prints **X11**, passes, and emits no GTK
warning. Evidence reports/checkpoint-440-display-{inherited,x11}.{json,log,stderr.log} and
checkpoint-440-display-review.json. The last invocations omitted AUTOMATION.md's documented WAYLAND_DISPLAY clearing; correct that
invocation and also make the script's own Xvfb leg enforce its promised backend. This does not prove
the lower-level source of GTK's init-order warning. Do not label inherited 439-integrity/current 440
as proven private-X11 execution. Their assertions/results are valid on the selected environment.
Original streams remain intact. The GL leg is now fixed, full 440-x11 is complete, and inherited
and isolated checkpoints are archived separately. No CNA code edit or issue ID.

Earlier GTK investigation evidence remains reports/checkpoint-439-integrity-gtk-* and its
-stderr-review.json: 55 GTK warnings per full renderer, one per isolated inherited title probe,
none for displayless SOFTWARE. Selected libcna was unchanged; 29 future selected hashes now include
resolved SDL3/mixer libraries. No full dependency closure is claimed.

439 targeted evidence:
- nested_builds passes 20 child checks and incremental_compiler ten, under both fresh displayless
  renderer selections: /tmp/cna-439-{nested_builds,incremental_compiler}-{SOFTWARE,OPENGLES3}.{json,log}.
  All old nested checks are retained. Every printed private path is independently verified gone.
- The new compiler actually uses RunContentCompiler's registry factory, parses its manifest and
  checks output digests. Component counters prove unchanged skips, one-primary and shared-input
  invalidation. Invalid score import preserves old output/manifest; repair and missing-output
  rebuild pass. Real clean preserves an unowned note/source; final private cleanup is separate.
  The intentional rejected import prints a contextual diagnostic; it is not a failed demo check.
- Initial /tmp/cna-439-build.log stopped because external C API work included generated/Avatars.inc
  before the file existed. It was present/tracked at subsequent CNA HEAD 57ddacfa2ea7ad89a1bbe2ae8672a9741f4ca63c.
  No CNA edit, no claim clean 57ddacfa fails. Same-tree /tmp/cna-439-retry-build.log is clean.
  Metadata 439/0 and all 21 targeted CTests pass (/tmp/cna-439-targeted-ctest.log), including all
  16 C ABI demos and both changed tools. Source assets are unchanged; score fixtures are original
  small runtime text files, not a native build tree or copied CNA corpus.
- All sibling/compiler /tmp drafts are now APPLIED AND STALE. Use repository sources. No GPU,
  genuine-XNA, crash-consistency, multi-output atomic-failure or concurrent-publisher claim.

438 audio correction evidence: initial full attempt at f899d61 stopped in CTest on from_stream's
fixed 600 ms completion assumption; no smoke began. Six unchanged isolated runs passed, with
identical original/copied WAV SHA b7cb8c864d076444d0de8842373ac137c7691c0a3974f7d0ff8380fec56cf5c1.
The actual SDL3 mixer is asynchronous; an exact scheduling cause was not proven. Both demos now
poll State to natural completion with a steady-clock 3 s deadline, without Stop to satisfy it.
Six checks each remain, including the original failed assertion text. Focused evidence is
/tmp/cna-438-audio-final-{from_stream,playback_state}-{SOFTWARE,OPENGLES3}.{json,log}; all 23 targeted
CTests and five repetitions per audio test passed before the successful complete retry.
No audible-quality/latency claim and no persistent CNA defect established.

433 targeted evidence:
- reader_versions passes 17 console checks per renderer selection WITHOUT DISPLAY/WAYLAND;
  `/tmp/cna-433-reader_versions-{SOFTWARE,OPENGLES3}.{json,log}`. Actual v0/v1 payloads,
  pre-read refusal of unsupported versions, owned immutable factory policies, duplicate capture
  release, scoped withdrawal, surviving created readers and cached values all verified.
- existing_objects has 13 checks: eleven pass, two reproduce ISSUE-043. Automatic raw lookup
  leaves the stream position unchanged before explicit-reader recovery; original failure retained.
  `/tmp/cna-433-existing_objects-{SOFTWARE,OPENGLES3}.{json,log}`. No visual/GPU claim.
- Three new original assets regenerate byte-identically and independently decode to exact EOF.
  Clean build `/tmp/cna-433-build.log`, metadata 433/0, all 20 focused CTests pass including
  all 16 C ABI demos (`/tmp/cna-433-targeted-ctest.log`). Coverage 293/329 types,
  1354/2326 named members, 158/3210 C routes; runtime columns now include all 433 demos.
- External CNA HEAD advanced to 4d774868e97c481fbb64e52a6286ed8c0c65873f with further generated
  C API test/tool edits uncommitted. Build includes these external changes; do not edit/revert CNA
  or claim an atomic source snapshot. Preserve binary hashes after the full checkpoint.
- Full 433 completed with all 446 enabled CTests and both renderer smoke runs. Both exact
  ISSUE-043 reviews are archived; original logs are untouched. See the current checkpoint above.
- All /tmp reader-version and raw-extension drafts are now applied and stale; use repo sources.

434 batch (built and targeted validation complete):
- Full 434 passed all 447 enabled CTests and both complete renderer runs. ORIGINAL comparisons
  to 433 require review only for new ISSUE-045 and passing cnj_loader 7 -> 11 / multiple_batches
  0 -> 6 checks. No older failed assertion removed or changed (27/28 -> 28/29); original logs,
  exact review/hashes and all 19 binary hashes archived. SOFTWARE stderr byte-identical;
  GL remains 3 ERROR/3 INFO/84 X protocol lines with volatile serials.

- asset_precedence: original same-name XNB/CNB/CNJ/native values 400/300/200/100, private files,
  cache/fresh-manager/Unload behavior, invalid compiled-file refusal without fallback, repair
  and retry, explicit native filename. Both fresh displayless renderer selections have 14
  checks: thirteen pass, exactly one confirms unwrapped EndOfStreamException through public
  Load (ISSUE-045). Initial unlabeled evidence `/tmp/cna-434-asset_precedence-{SOFTWARE,OPENGLES3}.{json,log}`.
  Final labeled evidence `/tmp/cna-434-asset-final-{SOFTWARE,OPENGLES3}.{json,log}` retains
  exactly the same failure with ISSUE-045. Both private temporary directories were verified gone.
- cnj_loader: 11 passing checks per selection WITHOUT DISPLAY/WAYLAND, including same-name,
  same CNJ type -> Enemy and Nameplate caches, unsupported requested type and both reloads.
  `/tmp/cna-434-cnj_loader-{SOFTWARE,OPENGLES3}.{json,log}`. Existing assets unchanged.
- multiple_batches: six checks pass per actual renderer, retained/deferred/default device states
  and actual transformed-arrow/fixed-HUD pixels. Both screenshots inspected; 3,108 pixels differ
  across renderers by at most one channel level, no exact-image equality claim.
  `/tmp/cna-434-multiple_batches-{SOFTWARE,OPENGLES3}.{json,log,png}`.
- New original 62-byte resolution/rules.xnb regenerates byte-identically, independently decoded
  through EOF; SHA-256 3c35a60861d39185dae914a2344c63fcc09e637367f50ef795c2fcc68a4f07df.
- Both `/tmp/cna-434-build.log` and `/tmp/cna-434-labeled-build.log` are clean. All 21 targeted
  CTests pass, including all 16 C ABI demos (`/tmp/cna-434-targeted-ctest.log`). Metadata 434/0;
  coverage 293/329 types, 1354/2326 named members, 158/3210 C routes. Runtime columns still
  now include all 434 IDs. External CNA source changed again during preparation, so the
  label build recompiled C API too. Observed HEAD 9e340c876f6426218e2f82651fcb0b086de25e15
  with additional externally edited C API implementation files. Do not edit/revert CNA or claim
  an atomic source snapshot. Post-build source patch/hashes `/tmp/cna-434-dependency{.patch,-files.json}`
  are read-only evidence, not an atomic record of every compiled input. Archive final binary
  hashes are archived with full 434; no rebuild occurred during its runtime phases.
- All /tmp precedence/CNJ/sprite drafts are now applied and stale; use repository sources.

XML extension batch (built and targeted validation complete, no new demo IDs):
- Complete tools/validate.sh passed: all 447 enabled CTests, SOFTWARE 386/20/25/3,
  OPENGLES3 407/21/3/3. Both original comparisons to 434 pass; all 28/29 failed assertions,
  statuses and JSON counts are unchanged. Child stdout proves 10 -> 16 and 9 -> 11 passes,
  retaining every earlier check. SOFTWARE stderr is byte-identical; GL retains 3 ERROR,
  3 INFO and 84 X protocol lines (42 request 18 / 42 request 25; volatile serials).

- intermediate_shared_resources: all 16 child checks pass under both selections WITHOUT
  DISPLAY/WAYLAND. Two actual resources close their forward/backward cycle, retain aliases
  and fresh values, share mutation, and expire after scoped unlinking of both authored and
  loaded C++ cycles. Existing ten acyclic/null/malformed-ID checks are retained. No automatic
  cycle collection, root-as-resource identity or malformed cyclic graph claim.
- intermediate_serializer: all 11 child checks pass similarly. Objective's optional C++ carrier
  is constrained by AllowNull(false); an authored value different from its constructor default
  round-trips and explicit null is refused on read AND write. Existing Hint absence/null/default
  behavior remains tested. Other collection/default policies remain separate gaps.
- Evidence `/tmp/cna-434-xml-{intermediate_shared_resources,intermediate_serializer}-{SOFTWARE,OPENGLES3}.{json,log}`.
  Parent standalone reports have checks=0; actual counts are in child stdout. No GPU/visual claim.
- Build `/tmp/cna-434-xml-build.log` clean, metadata 434/0, all 20 targeted CTests pass including
  all 16 C ABI demos (`/tmp/cna-434-xml-targeted-ctest.log`). Coverage stays 293/329 types,
  1354/2326 named members, 158/3210 C routes; full runtime report source is now checkpoint-434-xml.
- External CNA observed clean HEAD 419fb8d83 after committed C API handle/refusal/generation
  changes. Relevant objects rebuilt and C ABI tests passed. Do not infer an atomic source
  snapshot; archive the final binary hashes with the complete checkpoint.
- The /tmp cycle and null-policy drafts are applied and stale; use repository sources.

435 batch (built and targeted validation complete):
- content.custom.intermediate_polymorphism has eight passing child checks per fresh renderer
  selection WITHOUT DISPLAY/WAYLAND: actual concrete Type attributes, null, restored list order,
  fresh dynamic EnemySpawn/TreasureSpawn objects with inherited/derived fields, and refusal of
  unknown/unrelated/abstract types at their intended diagnostic gates. Public XmlReader inspects
  actual emitted prefixes. No GPU, XNB output or genuine-XNA execution is claimed.
- Evidence `/tmp/cna-435-polymorphism-{SOFTWARE,OPENGLES3}.{json,log}`. Standalone JSON has
  checks=0; all eight checks are in child stdout. No new CNA issue or asset.
- `/tmp/cna-435-build.log` clean; metadata 435/0; all 20 targeted CTests pass, including all 16
  C ABI demos (`/tmp/cna-435-targeted-ctest.log`). Coverage remains 293/329 types, 1354/2326
  named members, 158/3210 C routes. Full runtime columns now include all 435 IDs from the complete checkpoint. External CNA C API changes required rebuilding
  affected objects; no CNA edits here and no atomic dependency source snapshot claim.
- All /tmp polymorphism drafts are applied and stale; use repository sources.
437 batch (complete full validation included):
- Full retry passed all 450 enabled CTests and both complete renderer smoke runs. Exact original
  reviews confirm only new XML ISSUE-046 (28/29 -> 29/30 raw failures); all older states/counts
  and failed assertions retained. Ownership has 14 passing checks; XML has nine passes and one
  failure in child stdout. SOFTWARE stderr byte-identical; GL still 3 ERROR/3 INFO/84 X protocol
  lines (42 request 18 / 42 request 25; volatile serials). All 23 selected binary hashes archived.
- The first full attempt stopped before CTest because four completed content builds emitted
  one staging warning: cna_content_stage_v1_832841_18dd85b9e2bcf493_0 was not a validated
  current-user directory. It was already absent when inspected. The scavenger enumerates before
  checking directory status/name/ownership, so concurrent teardown is plausible, not proven.
  Original logs: /tmp/cna-437-full-first-build.log and /tmp/cna-demos-437-first-validation.log;
  also reports/checkpoint-437-first-{build,validation}.log. Nothing was deleted, no warning
  filter relaxed, no CNA edit made. The clean full retry is the completed checkpoint.

- read_asset_ownership passes all 14 checks under both selections WITHOUT DISPLAY/WAYLAND:
  callback identity for an actual nested shared_ptr<IDisposable> tile buffer, null/value exclusions,
  fresh independent repeated reads, short-read source stream destruction, lifetime across
  Unload/Dispose, early refusal, caller disposal and final weak-handle expiry. No default-manager,
  Load-cache interaction, raw/untyped/shared recording or callback-deduplication claim.
- intermediate_external_references has ten child checks: nine pass and one reproduces ISSUE-046
  under both displayless selections. Writer emits External:Profile; reader fails to resolve that
  target alias even though the same Profile serializer reads the inline preview. A separately
  labeled canonical-name control verifies relative relocation, wrapper/null identity and invalid
  location/ID/type refusal. Native XML and failed assertion remain; no hidden alias registration.
- Initial evidence `/tmp/cna-437-{ownership,xml-external}-{SOFTWARE,OPENGLES3}.{json,log}`;
  final evidence `/tmp/cna-437-{ownership-final,xml-external-final}-{SOFTWARE,OPENGLES3}.{json,log}`.
  Ownership JSON counts 14; the XML standalone JSON counts zero, so inspect its actual child
  stdout (nine passes, one failed assertion). No GPU/visual or genuine-XNA execution claim.
- Original assets/ownership/level.xnb: 117 bytes, deterministic regeneration and independent
  decode to exact EOF; SHA-256 850be84f95d6aee47a09ab51b02c5c3c871201887a1a89a377d9726ab927d8d6.
  Generator tools/xnb_ownership.py is integrated into generate_assets.py.
- Both `/tmp/cna-437-build.log` and `/tmp/cna-437-labeled-build.log` are clean. Metadata 437/0;
  all 21 targeted CTests pass including all 16 C ABI demos and the new WILL_FAIL XML tool
  (`/tmp/cna-437-targeted-ctest.log`). Coverage 293/329 types, 1354/2326 named members,
  158/3210 C routes. Runtime columns now include the complete 437 results.
- CNA observed e5014ffaaec2b7f2b018b4dad1f5e299899315f7; external C API changes again caused
  rebuilds. The inspected content-pipeline sources have no uncommitted changes. No CNA edits
  here, no atomic source snapshot claim; archive the actual binaries after full validation.
- All /tmp ownership/external-reference drafts are applied and stale; use repository sources.

438 initial batch (full validation complete; before the output extensions below):
- nested_builds passes eleven child checks under both selections WITHOUT DISPLAY/WAYLAND.
  BuildAndLoadAsset scales a real profile before computing a stage budget without compiling that
  child. BuildAsset emits one actual child CNB for duplicate requests, matching parent XREF and
  runtime name; dependencies, writer schemas, conflicting processing/name refusal and subsequent
  deterministic build are checked. Compiled bytes stay in memory; no publication, runtime load,
  incremental-cache or genuine-XNA execution claim.
- read_asset_ownership retains all fourteen prior controls, adds a successful absent-recorder
  read and one failed default-tracking/Unload assertion (ISSUE-011). Sixteen checks, fifteen pass,
  in each fresh displayless process. Its final explicit cleanup cannot turn that assertion into
  a pass. This extends the existing issue; the original texture unload demo is unchanged.
- Evidence `/tmp/cna-438-{nested,ownership}-{SOFTWARE,OPENGLES3}.{json,log}` and labeled
  `/tmp/cna-438-ownership-final-{SOFTWARE,OPENGLES3}.{json,log}`. Nested standalone JSON counts
  zero; its eleven checks are visible in child stdout. Ownership JSON counts sixteen.
- Initial `/tmp/cna-438-build.log` failed because the new standalone omitted cna_demos_framework
  for CnaDemos/Process.hpp/ExecutableDirectory. Corrected to the existing adapter-tool CMake
  pattern; this was a demo integration error, not CNA. `/tmp/cna-438-retry-build.log` and
  `/tmp/cna-438-labeled-build.log` are clean. Metadata 438/0; all 21 targeted CTests pass,
  including all 16 C ABI demos (`/tmp/cna-438-targeted-ctest.log`).
- Original nested_builds text sources regenerate identically: arena.stage SHA-256
  76d0b5bc2889502f3df7a36fb7c7be4ccd8829dcc998bca3a981bcf05384b1d1; guard.profile
  5fefa19a8106359d8329498a3f6db22bc8d15619ba8771284cf0ba8c3c2173da.
  tools/nested_build_assets.py is integrated into generate_assets.py. The ownership XNB is unchanged.
- Coverage 293/329 types, 1354/2326 named members, 158/3210 C routes. Runtime columns now use
  checkpoint-438, including the new nested-build PASS and the extended ownership ISSUE-011 failure.
- External CNA observed through ea41d758d9689cda348887a92a93bbe188864e8b, with C API work
  continuing; inspected content modules have no uncommitted changes. No CNA edit or atomic source
  snapshot claim. Archive actual binaries after validation. All /tmp drafts in this batch are
  applied and stale; use repository sources.

438-outputs batch (complete full checkpoint):
- nested_builds retains eleven checks and adds four passing checks in both fresh displayless
  selections. Actual parent-only CNB load fails for the missing child; caller publication of
  the returned additional output permits same-manager retry without Unload. The loaded profile
  is Guard/80 with stage budget 240; direct/cached parent loads share its pointer and do not
  re-enter the factories. The manager dies before private-tree removal, verified independently.
- The route adapter preserves all seven original checks. ExportSummary reads a processor label
  dependency and writes actual human-readable bounds. AddOutputFile then refuses a separate
  configured output root: reproduced and recorded as ISSUE-047. The explicitly separate
  source-contained control records the deployment file and dependencies; a changed-label build
  changes report bytes without changing compiled bounds. Missing input checks all four public
  error-context fields; cleanup passes. Final total 13 child checks, 12 pass/one known failure.
- Initial `/tmp/cna-438-outputs-xna_pipeline_components-{SOFTWARE,OPENGLES3}.{json,log}` also
  retains a demo diagnostic mismatch: the demo's missing-label exception omitted patrol.label.
  ResolveSourceDependency promises path resolution/recording, not existence checking. The demo
  now names the input in its own error; no second CNA defect or hidden skipped check.
- Clean builds `/tmp/cna-438-outputs-{build,final-build}.log`. Final fresh evidence for both tools:
  `/tmp/cna-438-outputs-final-{nested_builds,xna_pipeline_components}-{SOFTWARE,OPENGLES3}.{json,log}`.
  Private paths from all final logs are independently verified absent. All 22 targeted CTests
  pass (/tmp/cna-438-outputs-targeted-ctest.log); separate metadata validation is 438/0. Source assets remain unchanged.
- Both standalone JSON check counts are zero; review child stdout. Source drafts under /tmp are
  now APPLIED AND STALE, especially route-summary.cpp (unlabeled and old diagnostic). Never reapply.
- Runtime columns in COVERAGE.md now describe complete checkpoint-438-outputs. No CLI atomic publisher, incremental cache, GPU/visual
  or genuine-XNA execution claim. External CNA observed through d751cf46a with C API changes;
  no CNA edit or atomic source snapshot claim.

439 notification investigation (full retry complete):
- Unchanged fresh runs /tmp/cna-439-notification-original-{SOFTWARE,OPENGLES3}-{1,2}.{json,log}
  reproduce SOFTWARE failure once (7.024 s, both measured regions zero) and one pass (5.841 s,
  9882 top-right pixels); GL passes twice (~3.25 s, 9879 pixels). Four checks each.
- Source inspection shows a four-second real-time notification lifetime and 0.22 s entry
  animation. The demo sampled only at FramesDrawn()==10 after another 500 ms sleep. The exact
  failed frame's toast age was not instrumented; this fixed-frame timing assumption is invalid
  under variable load, and no persistent CNA positioning defect is established.
- The demo now observes every early EndDraw until either region exceeds the original visibility
  threshold or a steady-clock two-second bound is reached. Ten-ms cadence permits animation in
  an unthrottled run. Wrong-bottom placement is checked immediately, not retried until success.
  A never-visible toast still fails at deadline/end-run; all four checks and the original failed
  assertion text remain. No internal clock override or CNA source edit. Clean build /tmp/cna-439-notification-build.log. Two default-length runs plus a frame-eight capture
  per renderer pass all four checks: /tmp/cna-439-notification-final-{SOFTWARE,OPENGLES3}-{1,2,capture}.{json,log}
  and capture.png. Observation two finds 9882/9879 top-right pixels and zero bottom pixels; both
  screenshots inspected. All 26 guide/content/C ABI/catalog CTests pass (27.70 s), preserved in
  /tmp/cna-439-notification-targeted-ctest.log. Five consecutive notification CTests pass
  in /tmp/cna-439-notification-repeat-ctest.log (13.35 s); commit and retry the complete checkpoint.

Integrity extension (included in complete 439-integrity checkpoint):
The existing incremental_compiler ID now has fifteen passing checks. Accepted registry IsFrozen
and late RegisterWriter refusal pass. Writer version 1 -> 2 rebuilds both nodes despite identical
encoding; same-length output corruption with restored mtime rebuilds only that node; invalid
manifest text triggers two fresh builds and valid replacement. Real clean refuses changed owned
bytes without deleting the valid peer or manifest. Only then are exact owned bytes restored for
the original successful-clean control. All ten older checks remain. Evidence:
/tmp/cna-439-integrity-{SOFTWARE,OPENGLES3}.{json,log}. Expected corrupt-manifest warning and clean
refusal are preserved, not failed demo checks. No new CNA issue; no concurrency/crash-consistency
claim. Integrity drafts are APPLIED AND STALE; use repository sources.

The previously source-reviewed raw/existing-instance gap is now executed by the 441st demo above.
Explicit-hint ordinary combinations and boxed Core forwarding remain intentionally without isolated
duplicate demos; only add a meaningful dynamic-schema case. Public source review also confirms
concurrent Build requires reentrant components/log sinks; no parallel execution is claimed from
serial compiler counters or registry freezing.

Compiler configuration extension after complete 441 (full 441-config checkpoint passed):
/tmp/cna-compiler-configuration.cpp and /tmp/cna-compiler-configuration-metadata.cpp extend the
existing incremental_compiler from 15 to 20 checks. Source APIs verified: public
ContentBuildConfigurationFileName/Parse/Find, Context.Parameters and typed i64 JSON values stored
as strings; CLI uses --config or the conventional .cna-content.json, not an invented parameter flag.
Processor identity becomes 2 for the new bounded Bonus parameter (default 0 preserves all old cases).
Per-asset Bonus 5 should rebuild only arena to 32, whitespace-only rewriting should skip both,
Bonus 6 should rebuild arena to 33; manifest values/fingerprints and callback counters are inspected.
Removing menu.score should collect its trusted obsolete output while preserving the live node and
unowned note, and restoring it should publish menu again. Both owned outputs exist before the
original guarded/successful clean controls; exact latest arena bytes are captured and restored
only after observing modified-output refusal. All fifteen older assertion texts remain. All 20 checks and their counter/value expectations now pass under both displayless selections,
including actual obsolete-output collection and reintroduction. Drafts are applied and stale. /tmp/cna-draft-compiler-configuration.py
can regenerate these drafts from current sources; its C++ escape sequences were reviewed/corrected
before compilation. Prefer reading the drafts rather than overwriting later changes blindly.

Next meaningful compiler gaps: missing-manifest recovery, multi-output/deployment publication
failures, graph-level invalidation and concurrency; choose coherent scenarios rather than IDs per
cache variation. Typed parameter fingerprints and removed-source collection are now executed.
The member audit still identifies native ContentValue stable-type/C++-carrier and component-result
mismatches as meaningful contract gaps; inspect the existing pipeline_components demo before
choosing an extension. Registry freezing and late mutation rejection are already exercised.

XML aliases remain ISSUE-046; direct custom raw/existing-instance serialization now has focused
runtime evidence above, included in complete 441.
The XML-to-XNB integration now uses public RegisterXnaXmlSourceContentPipeline with a shared XNA
ContentCompiler after TypeSerializerFor<ProfileSource> and AddTypeWriter<ProfileWriter>. Public
factory options.xnbContainer supplies the container configuration. Actual original profile XML
builds to a 56-byte XNB through XmlImporter and pass-through, then public runtime reader/manager
restores Sentinel health 75. Unchanged input skips the writer; health 120 rebuilds. Cached data stays 75
until Unload/reload sees 120. Manifest importer and actual artifact digest are verified. A separate
CNB request reaches the expected Selection diagnostic for the unsupported arbitrary-object schema;
return 1 is explicitly asserted, not a failed demo. Two DEBUG runtime load lines plus two expected
CNB-refusal diagnostic lines are retained per run. No GPU, genuine-XNA, external-object-graph,
crash-consistency or multi-output publication claim. Do not repeat basic cache/sibling coverage.

**Latest complete checkpoint (2026-10-12):** 441 at 7361498, tools/validate.sh exit 0.
SOFTWARE 390/23/25/3, OPENGLES3 411/24/3/3; all 454 enabled CTests (191.33 s), metadata 441/0. Archive
prefix reports/checkpoint-441: original renderer JSON/stdout/stderr, both -review.json, build/ctest/
validation logs and -binaries.json (30 selected artifacts, including new raw XML tool and SDL3/mixer).
Exact ORIGINAL 440-x11 reviews retain all older IDs/statuses/known issues/counts and 31/32 raw failed
assertions. New raw XML 10 PASS; prior XML 10, compiler 15 and nested 20 lists retained. Normalized stderr
unchanged, including 84 GL X protocol diagnostics and no GTK warnings. Hashes match runtime capture.
Coverage 293/329 types, 1354/2326 members, 158/3210 C routes; both runtime columns complete.
Use ORIGINAL checkpoint-441 next. No native rebuild during runtime and no atomic dependency snapshot.

Historical 440-x11 at 8b370a0 remains archived: SOFTWARE 389/23/25/3, GL 410/24/3/3, all 453 enabled
CTests (162.54 s), 29 hashes. It corrected inherited Wayland selection with tested X11 isolation,
retained every assertion, and removed 55 GTK warnings per renderer without filtering stderr.

Historical 440 at 9995869 remains archived with both reviews and 29 hashes: same totals/child checks,
new XML integration 10 PASS, all 453 enabled CTests (332.25 s). That invocation inherited Wayland;
its valid results are not isolated-X11 evidence. Do not replace its raw streams with corrected data.

Historical 439-integrity at 83c919d is also archived: SOFTWARE 388/23/25/3, OPENGLES3 409/24/3/3,
all 452 enabled CTests, 26 hashes, compiler 10 -> 15 retaining all older checks. It shares the inherited
Wayland-selection limitation investigated above; failed assertions 31/32 unchanged.

Historical complete 439 at d6afc7b also remains archived with 26 selected hashes and both reviews:
same totals, compiler 10 checks and nested 20. It corrected the Guide observer timing defect after
preserving the failed first attempt; all 452 enabled CTests passed (164.62 s).

Historical 438-outputs checkpoint at e6327d2 remains archived with its original reviews, binary
hashes and logs: SOFTWARE 387/23/25/3, OPENGLES3 408/24/3/3; 451 enabled CTests. It introduced
ISSUE-047 and extended nested runtime publication; 31/32 raw failed assertions.

Historical 438 checkpoint:
**Completed checkpoint (2026-10-12):** 438 at 973a63a, full tools/validate.sh retry exit 0.
SOFTWARE 388/22/25/3, OPENGLES3 409/23/3/3; 451 enabled CTests, 438/0 metadata.
Archives: reports/checkpoint-438-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}, both -review.json,
checkpoint-438-{build,ctest,validation}.log and checkpoint-438-binaries.json (24 selected artifacts).
Only ownership's newly demonstrated default-recorder ISSUE-011 changes the original comparison;
all other older outcomes/counts and all older failed assertions retained. Raw failures 30/31.
Both stderr streams are byte-identical to 437. Selected binaries match hashes taken during runtime.
Coverage 293/329 types, 1354/2326 named members, 158/3210 C routes; runtime columns complete.
Use ORIGINAL checkpoint-438 next. First audio CTest failure and its logs are preserved separately.

Historical 437 checkpoint:
**Completed checkpoint (2026-10-12):** 437 at b697786, full retry tools/validate.sh exit 0.
SOFTWARE 388/21/25/3, OPENGLES3 409/22/3/3; 450 enabled CTests, 437/0 metadata.
Archives: `reports/checkpoint-437-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`,
`checkpoint-437-{build,ctest,validation}.log`, both renderer `-review.json` files, and
`checkpoint-437-binaries.json` (23 selected artifacts). Both exact reviews account for only
new XML ISSUE-046; all 435 older statuses/counts/failures retained. Raw failures 29/30.
SOFTWARE stderr byte-identical; GL same 3 ERROR/3 INFO/84 X protocol lines. First interrupted
attempt retained separately, with cause limits above. Coverage 293/329 types, 1354/2326 named
members, 158/3210 C routes; all runtime columns complete. Use ORIGINAL checkpoint-437 next.

Historical 435 checkpoint:
**Completed checkpoint (2026-10-12):** 435 at dec89ff, tools/validate.sh exit 0.
SOFTWARE 387/20/25/3, OPENGLES3 408/21/3/3; 448 enabled CTests, 435/0 metadata.
Archives: `reports/checkpoint-435-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`,
`checkpoint-435-{build,ctest,validation}.log`, both renderer-specific `-review.json` files,
and `checkpoint-435-binaries.json` (22 selected artifacts). Both strict original comparisons
pass: one new passing polymorphic XML tool with eight verified child checks; all older statuses,
check counts and failed assertions unchanged (28/29). SOFTWARE stderr byte-identical;
GL has the same 3 ERROR/3 INFO/84 X protocol lines, with volatile serials. Coverage remains
293/329 types, 1354/2326 named members, 158/3210 C routes; all runtime columns complete.
Use ORIGINAL checkpoint-435 as next baseline.

Historical 434-xml checkpoint:
**Completed checkpoint (2026-10-11):** 434-xml at c3368b0, tools/validate.sh exit 0.
SOFTWARE 386/20/25/3, OPENGLES3 407/21/3/3; 447 enabled CTests, 434/0 metadata.
Archives: `reports/checkpoint-434-xml-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`,
`checkpoint-434-xml-{build,ctest,validation}.log`, both `-review.json` files, and
`checkpoint-434-xml-binaries.json` (21 selected artifacts). Strict original comparisons pass,
including identical 28/29 failed-assertion multisets. Eight passing child checks are added;
all previous child checks retained. Coverage 293/329 types, 1354/2326 named members,
158/3210 C routes; all runtime columns complete. Use ORIGINAL checkpoint-434-xml next.

Historical 434 checkpoint:
**Completed checkpoint (2026-10-11):** 434 at 652398e, tools/validate.sh exit 0.
SOFTWARE 386/20/25/3, OPENGLES3 407/21/3/3; 447 enabled CTests, 434/0 metadata.
Archives: `reports/checkpoint-434-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`,
`checkpoint-434-{build,ctest,validation}.log`, both renderer-specific `-review.json` files,
and `checkpoint-434-binaries.json` (19 artifacts). Exact reviews cover new ISSUE-045 plus
cnj_loader 7 -> 11 and multiple_batches 0 -> 6 checks. All other older statuses/check counts
and failed assertions unchanged. Raw failures 28/29. SOFTWARE stderr byte-identical to 433;
GL remains 3 ERROR/3 INFO/84 X protocol lines (volatile serials). Coverage 293/329 types,
1354/2326 named members, 158/3210 C routes; all runtime columns complete.
Use ORIGINAL checkpoint-434 as next baseline.

Historical checkpoint notes below are evidence of earlier states, not current instructions.
Any old "pending", "next" or "running" statements are superseded by the immediate next action
and latest complete checkpoint above. Never reapply an old /tmp draft over repository sources.

Historical 433 checkpoint:
**Completed checkpoint (2026-10-11):** 433 at a01ab68, tools/validate.sh exit 0.
SOFTWARE 386/19/25/3, OPENGLES3 407/20/3/3; 446 enabled CTests, 433/0 metadata.
Archives: `reports/checkpoint-433-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`,
`checkpoint-433-{build,ctest,validation}.log`, both renderer-specific `-review.json` files,
and `checkpoint-433-binaries.json` (19 artifacts). Only the added implicit raw-read ISSUE-043
assertion requires review; all older failures/states unchanged. Totals 27/28 raw failures.
SOFTWARE stderr byte-identical to 432; GL has the same 3 ERROR/3 INFO/84 X protocol lines
(with volatile serials). Coverage 293/329 types, 1354/2326 named members, 158/3210 C routes;
all runtime columns complete. Use ORIGINAL checkpoint-433 as next baseline.

Historical 432 checkpoint:
**Completed checkpoint (2026-10-11):** 432 at 9cad3a1, tools/validate.sh exit 0.
SOFTWARE 385/19/25/3, OPENGLES3 406/20/3/3; 445 enabled CTests, 432/0 metadata.
Archives: `reports/checkpoint-432-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`,
`checkpoint-432-{build,ctest,validation}.log`, and `checkpoint-432-binaries.json` (19 artifacts).
Strict ORIGINAL comparisons to 430 both pass; only two new passing IDs. All older statuses,
check counts and 26/27 failed assertions unchanged. SOFTWARE stderr byte-identical; OPENGLES3
stderr remains separately preserved. Coverage 293/329 types, 1353/2326 named members,
158/3210 C routes, all runtime columns complete. Use ORIGINAL checkpoint-432 as next baseline.

Historical 430 checkpoint:
**Completed checkpoint (2026-10-11):** 430 at b113bdc, tools/validate.sh exit 0.
SOFTWARE 383/19/25/3, OPENGLES3 404/20/3/3; 443 enabled CTests, 430/0 metadata.
Archives: `reports/checkpoint-430-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}`,
`checkpoint-430-{build,ctest,validation}.log`, renderer-specific `-review.json` files and
`checkpoint-430-binaries.json`. Both ORIGINAL comparisons to 428 require review exactly for
new ISSUE-043; all older states/check counts/assertions are unchanged (25/26 -> 26/27).
External references pass 11 checks; existing objects has eleven passes plus the one known lookup
failure. SOFTWARE stderr is byte-identical; GL retains three ERROR/three INFO and 84 X protocol
lines (42 requests each 18.0/25.0; volatile IDs differ). All 16 C ABI demos also pass both full
smoke selections despite the externally evolving C API implementation (provenance below).
Coverage 293/329 types, 1352/2326 named members, 158/3210 C routes; all 430 runtime columns
are complete. Use ORIGINAL checkpoint-430 as the next baseline.

Historical 428 checkpoint:
**Completed checkpoint (2026-10-11):** 428 at 0c30ba0, tools/validate.sh exit 0.
SOFTWARE 382/18/25/3, OPENGLES3 403/19/3/3; 441 enabled CTests, 428/0 metadata.
Archives: `reports/checkpoint-428-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}` and
`checkpoint-428-{build,ctest,validation}.log`. Both reports match the fresh catalog; both
ORIGINAL stdout comparisons to 427 pass, with 25/26 failed assertions unchanged. Only the new
shared_resources ID is added (15 passing checks). SOFTWARE stderr is byte-identical; GL retains
three ERROR/three INFO and 84 X protocol lines (42 requests each 18.0/25.0, volatile IDs differ).
Coverage 293/329 types, 1346/2326 named members, 158/3210 C routes; all 428 runtime columns
are complete. Use ORIGINAL checkpoint-428 as the next baseline.

Historical 427 checkpoint:
**Completed checkpoint (2026-10-11):** 427 at 204d23c, tools/validate.sh exit 0.
SOFTWARE 381/18/25/3, OPENGLES3 402/19/3/3; 440 enabled CTests, 427/0 metadata.
Archives: `reports/checkpoint-427-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}` and
`checkpoint-427-{build,ctest,validation}.log`. Both reports cover the fresh catalog exactly;
both ORIGINAL stdout logs pass strict comparison to 425 (25/26 failed assertions unchanged).
Both new model IDs pass; manifest checks increase 20 -> 26 for six new built-in reader names.
SOFTWARE stderr is byte-identical to 425; GL ERROR/INFO counts remain three/three.
Coverage is 293/329 XNA types, 1344/2326 named members, 158/3210 C routes. All 427 IDs have
complete runtime columns. Use ORIGINAL checkpoint-427 as the next baseline.

Historical 425 checkpoint:
**Completed checkpoint (2026-10-11):** 425 at 7d25a1d, tools/validate.sh exit 0.
SOFTWARE 379/18/25/3, OPENGLES3 400/19/3/3; 438 enabled CTests, 425/0 metadata.
Archives: `reports/checkpoint-425-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}` and
`checkpoint-425-{build,ctest,validation}.log`. Both reports cover the fresh catalog exactly;
both ORIGINAL stdout logs pass strict comparison to 423 (25/26 failed assertions unchanged).
New palette_sampling/skinned_manifest pass; existing skinned_animation increases 3 -> 10 checks.
SOFTWARE stderr is byte-identical to 423; GL retains three ERROR/three INFO and 84 X protocol
lines (42 request 18.0 / 42 request 25.0; volatile serial/resource IDs differ).
Coverage remains 293/329 XNA types, 1342/2326 named members, 158/3210 C routes. All 425 IDs
now have complete runtime columns. Use ORIGINAL checkpoint-425 as the next baseline.

Historical 423 checkpoint:
**Completed checkpoint (2026-10-11):** 423 at 37ad437, tools/validate.sh exit 0.
SOFTWARE 377/18/25/3, OPENGLES3 398/19/3/3; 436 enabled CTests, 423/0 metadata.
Archives: `reports/checkpoint-423-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}` and
`checkpoint-423-{build,ctest,validation}.log`. Both original logs compare to 422 with exactly
one new failed ID/assertion: media.media_library.save_picture / ISSUE-041. The new short-read
reproduction intentionally adds this known failure; all older states/messages/multiplicities
are unchanged. Review/hashes: `checkpoint-423-{SOFTWARE,OPENGLES3}-review.json`.
Sampler checks increase 4 -> 10. Native stderr retains the old 25/3 ERROR lines and the old
84 GL X protocol diagnostics; exactly two INFO lines opening the new picture fixture are added.
Coverage is 293/329 XNA types, 1342/2326 named members, 158/3210 C routes. Complete runtime
columns now include all 423 IDs. Use ORIGINAL checkpoint-423 logs as the next baseline.

Historical 422 checkpoint:
**Current session checkpoint (2026-10-11):** full `tools/validate.sh` exited 0 for all **422 demos**
at `60e994b`. SOFTWARE 377/17/25/3, OPENGLES3 398/18/3/3, 435 enabled CTests, metadata 422/0.
Archives: `reports/checkpoint-422-{SOFTWARE,OPENGLES3}.{json,log,stderr.log}` and
`checkpoint-422-{build,ctest,validation}.log`. Both JSON reports match the fresh 422 catalog.
Strict comparisons to 421 (reviewed SOFTWARE baseline, original GL) pass directly on both
new ORIGINAL stdout logs, including exact messages/multiplicities 24/25. The only check-count
changes are the intended mesh_parts 8->10 and two_bones 3->8; new skinned_parts passes 11.
Native stderr classes/counts also match 421: SOFTWARE 25 renderer errors + one INFO; GL three
renderer errors + one INFO + 84 prior X protocol diagnostics (42 BadWindow ChangeProperty,
42 SendEvent; volatile serial/resource IDs differ). The process-output fix now has complete
end-to-end checkpoint evidence as well as its red/green regression fixture.
Coverage is 290/329 XNA types, 1328/2326 members, 158/3210 C ABI routes, not comprehensive coverage.
The 421 build began with CNA clean at e60fa35f7803a2109ae8baed06ac5827f8e59b7b. Its checkout
continued external documentation cleanup during validation; inspected modules/CMake source
paths had no difference from that revision. No CNA edits were made by this task.
Historical dependency provenance:
CNA was clean at `2b15506a48e1a6202c985823131daf5856826f71` when the 418 batch was built
(19:16 CEST); CI pins `10a4ef1...`. During the next batch, external CNA commits arrived
from 19:23 onward. At the 419 build completion (19:37), its latest committed revision was
`b9350df9a`; by 19:41 it had advanced to `c00b48c01` with a further documentation rename
in progress. We did not edit or revert those changes. The inspected cumulative diff from
2b15506 contains documentation/test-comment changes, a comment-only AudioMixer header edit,
and deletion of an unused root Hello World main.cpp; no library implementation or public
API behavior changed. The exact in-flight checkout was not atomically captured at build
start, so do not attribute the 419 binary to an exclusively pinned CNA commit. Checkpoint 419 completed against its already-built binary without a mid-run rebuild.

Historical 418 SOFTWARE raw comparison correctly requested review: five identical native logger stderr lines
were inserted between "no burst of catch-up updates" and " followed the load" in ISSUE-019's
stdout assertion (raw lines 746-751). Its JSON message independently contains the intact assertion.
Exactly that one splice was removed from a COMPARISON COPY only:
`checkpoint-418-SOFTWARE.reviewed.log`. `checkpoint-418-log-review.json` records both SHA-256
hashes, the exact removed logger line/count and corroborating JSON message. Both copies retain
24 CHECK FAILED markers; the reviewed copy matches checkpoint-417's exact failure multiset.
GL's raw 25 matches directly. No assertion was removed or baseline silently rewritten.
The 419 comparison used **checkpoint-418-SOFTWARE.reviewed.log**, retaining its raw
partner and review record. Future comparisons can use the separate-stream 419 logs directly. All original stdout/stderr bytes remain in the original archive.

`tools/validate.sh` now preserves stdout and stderr separately for future checkpoints, preventing
this top-level pipe splice. Archive BOTH `smoke-<renderer>.log` and `.stderr.log`; compare stdout
assertions and inspect stderr diagnostics. Four validator tests and four comparator tests pass;
the new fixture deliberately inserts stderr between two stdout writes and rejects the old script.
The parser remains strict and has no automatic error-stripping/reconstruction heuristic. This is
validation tooling, not a new CNA bug. Its /tmp draft files are now applied and stale.

Raw failed-assertion multisets are unchanged across 407/408/409 both complete 410 checkpoints and 411/412/413/415/416:
**24 SOFTWARE / 25 OPENGLES3**, including prefixed host/joiner lines. Earlier counts of 20/21
omitted those child-process assertions. The intermediate semantic-409 run exposed a separate
flaky demo expectation, investigated and corrected below. `tools/compare_checkpoints.py` now
checks IDs/statuses and assertion multiplicities; its tests and actual archived comparisons pass.


432 targeted evidence:
- graphics.model.imported_topology: 18 checks per renderer, exact loop-closing/strip/fan index
  ranges, public topology/counts, closed outline/unfilled interior and filled triangle regions.
- graphics.texture2d.dds_stream: 14 checks per renderer, packed offset and seven-byte short reads,
  complete five-level chain, four lower authored mips with exact DXT1 bytes, Color fit/crop,
  malformed-chain refusal and actual stripe/crop pixels. BOTH select public Dxt1 storage;
  Color fallback loading, other DXT alpha formats and physical GPU storage are not claimed.
- `/tmp/cna-432-{imported_topology,dds_stream}-{SOFTWARE,OPENGLES3}.{json,log,png}`. All four
  screenshots inspected; each renderer pair is RGBA-identical. Assets regenerate identically;
  glTF payloads independently decoded, DDS base independently decoded by Pillow and mip ranges
  checked through the exact end. Metadata 432/0; 20 targeted CTests pass including all 16 C ABI
  demos, both new demos and catalog checks (`/tmp/cna-432-targeted-ctest.log`).
- Initial build failed in external generated Cnb.inc: local result shadowed a same-named handle
  in its initializer (`/tmp/cna-432-build.log`). The external author changed it to routeResult;
  we did not edit CNA. Resumed build `/tmp/cna-432-retry-build.log` passes without diagnostics.
  The transient confirmed generator defect is documented as resolved ISSUE-044, no runtime marker.
- CNA HEAD was 4eed3baa7 with further external C API route generation in flight. Post-start hash
  inventory `/tmp/cna-432-dependency-files.json` is NOT an atomic dependency build snapshot.
  Preserve final binary hashes with the full checkpoint as with 430; do not edit/revert CNA.
Coverage is 293/329 types, 1353/2326 named members, 158/3210 C routes. Full runtime columns
now include all 432 demos. All /tmp graphics drafts are applied and stale.

430 targeted evidence:
- content.xnb.existing_objects: 12 console checks, eleven pass and one proves GetTypeReader's
  target-name/factory-name mismatch (CNA-DEMOS-ISSUE-043). Both final labeled runs WITHOUT
  DISPLAY/WAYLAND_DISPLAY: `/tmp/cna-430-existing-final-{SOFTWARE,OPENGLES3}.{json,log}`.
  The explicit public named-factory fallback allows indexed/raw existing-object state, null
  fallback, initialization order and type-erased tags to remain independently tested. No GPU claim.
- content.xnb.external_references: 11 passing checks per renderer; sibling PNG and compiled custom
  label, nulls, path normalization/refusal, owner caches and visible shared texture edit.
  `/tmp/cna-430-external-{SOFTWARE,OPENGLES3}.{json,log,png}` were inspected and RGBA-identical.
  Final stronger-boundary reports/captures use `/tmp/cna-430-external-final-{SOFTWARE,OPENGLES3}`;
  final captures match the inspected images byte-for-byte in RGBA.
- All eight new original assets regenerate identically; XNB tables/payload boundaries decoded
  independently. Metadata 430/0; coverage 293/329 types, 1352/2326 named members, 158/3210 C routes.
  Full 430 now includes all runtime columns; both original renderer comparisons were reviewed
  for exactly ISSUE-043 and archived without changing evidence.

IMPORTANT dependency change: CNA HEAD remains 2fda5389d2990b40146d06de0ae0531d05055f24,
but an external actor has edited roughly 60 C API implementation files and added marshal/output
headers. This is a real implementation refactor, not the preceding comment-only cleanup. Do not
edit/revert it or attribute this build to clean HEAD. Tracked patch and modified/untracked source
hashes: `/tmp/cna-430-dependency-tracked.patch`, `/tmp/cna-430-dependency-files.json` (captured
after build start, not an atomic build snapshot). Run the relevant C ABI demos/tests as well as
new readers; inspect any changed diagnostics/behavior. During full 430, CNA committed the helper refactor as 801ee62c5 (22:01) and math route generation
as 4eed3baa7 (22:13), with further audio/effect route edits uncommitted. The running checkpoint
continues using its already-built binaries; do not rebuild mid-run. Archive binary hashes at
completion to identify those artifacts. Full 428 remains the stable complete baseline built
before this refactor; its running binary was not rebuilt during that checkpoint.

427 targeted evidence:
- xnb_model: 11 checks per renderer. Original `.x` -> existing FORMAT xnb content build -> actual
  XNB signature, five named bones/two meshes, composed parent transforms, shared VB/IB fixups,
  independent part ranges/materials and actual red/green placements plus clear gap.
- multiple_skins: 21 checks per renderer. Two skins share authored mesh/material but have distinct
  data/effect palettes; non-joint ancestry persists, authored +/-4 mesh nodes remain in the graph
  while actual skinned mesh parents are the identity root. Prefixes are -0.5/+0.5. Animating one
  root and restoring the other bind pose preserves isolation. Live bounds are tested with a
  temporary pose chosen from the actual preceding bound so it is guaranteed outside it; the
  updated sphere includes it, restoring the palette excludes it, then actual visible pixels
  prove the final pose is restored. No claim about four-influence weighting or every bound case.
- Both initial screenshot pairs were inspected and RGBA-identical. Final 21-check multiple-skin
  captures are pixel-identical to the inspected 17-check scene after the temporary palette test.
Evidence: `/tmp/cna-427-xnb_model-{SOFTWARE,OPENGLES3}.{json,log,png}` and
`/tmp/cna-427-multiple-skins-final-{SOFTWARE,OPENGLES3}.{json,log,png}`. Earlier underscore-form
multiple_skins reports have 17 checks; preserve but do not confuse them with the final 21.
Final build `/tmp/cna-427-bounds-build.log` is clean; metadata 427/0. Focused final four CTests
are recorded in `/tmp/cna-427-model-final-ctest.log`. Asset generators reproduce both source
files byte-for-byte. XNB output is produced by CNA; no genuine-XNA-runtime interoperability run.
ISSUE-042 records stale public root-prefix comments only; no new known-failure marker or CNA edit.
CNA was 8b5564f1e81196f3aaee9e9a652b8030e21f4c6c at build start; since bf0cde63 only docs and
two doc/vendor test paths changed, not runtime implementation. Coverage is 293/329 XNA types,
1344/2326 named members and 158/3210 C routes. All /tmp model drafts are now applied and stale.

The existing_objects/external_references drafts prepared during full 428 are now APPLIED and
superseded by demos/content/xnb and tools/xnb_loadouts.py/xnb_external_assets.py. Do not copy
those older /tmp drafts again. The initially hidden type/factory-name assumption was corrected
before runtime; the distinct-name failure is confirmed and documented as ISSUE-043.

The runtime XNB graph drafts are now applied and superseded by tools/xnb_graphs.py and
content.xnb.shared_resources. It reads an ordinary inline Item alongside shared resources;
15 checks distinguish their identities, deferred callbacks, weak-edge lifetime and failure paths.
Continue remaining ContentReader behaviors in COVERAGE_AUDIT.md; built-in model fixups and
this custom graph do not establish every public reader overload or versioned schema.
Remaining model gaps include specular slots and broader skin/material variants; imported loop/strip/fan topology is now covered.
425 animation investigation: final/source-before-extension first-frame pixels were identical on
each renderer; cross-renderer lighting differences already existed (8,216 pixels, max delta 38),
with identical silhouettes. Original captures are `/tmp/cna-skinned-animation-before-*.png`.
The temporary old source was restored before the final 425 build/checkpoint; do not apply it again.

423 targeted evidence: part_sampler_state passes 10 checks on both renderers, including actual
glTF NEAREST/CLAMP metadata, owned texture and preserved out-of-range UVs; captures inspected,
RGBA-identical (`/tmp/cna-import-sampler-final-{SOFTWARE,OPENGLES3}.{json,log,png}`).
media.media_library.save_picture has 14 child checks and exactly one ISSUE-041 failure on both
selections (`/tmp/cna-save-picture-final-{SOFTWARE,OPENGLES3}.{json,log}`). Ordinary vector/stream
saves, verified private XDG roots, album views, exact original bytes, generated smaller thumbnail
and reindexing pass. No user media or visual/GPU thumbnail claim. ISSUE-040 is a separate stale
header comment. Final build `/tmp/cna-423-issue-build.log` is clean; four targeted CTests pass.
All these behaviors are now included in full checkpoint 423. No CNA code was edited.

Current targeted evidence (both renderer selections passed):
- mesh_parts: 10 checks, including mixed LineList/TriangleList in shared buffers; actual blue
  line verified in three narrow pixel bands. Screenshots inspected and RGBA-identical.
- two_bones: 8 checks, including exact palette readback, empty/oversized/invalid-weight refusal,
  preservation and the original rendered pose. Screenshots inspected; identical silhouettes,
  1,342 color pixels differ by at most one channel level (no exact-image equality claim).
- new graphics.model.skinned_parts: 11 checks, including owned-part transfer/replacement after
  donor destruction, different bone-count refusal before moves, removal, sampled inverse-bind
  palette and actual shifted surviving geometry/clear removed position. Screenshots inspected
  and RGBA-identical. No GPU leak/deallocation measurement, manifest-loading or equal-count
  skeleton-compatibility claim.
Evidence: `/tmp/cna-422-<demo-id>-{SOFTWARE,OPENGLES3}.{json,log,png}`. Build is clean
(`/tmp/cna-422-model-palette-build.log`); metadata 422/0; all six targeted CTests pass
(`/tmp/cna-422-model-palette-ctest.log`). CNA HEAD was e60fa35f7 with external documentation
cleanup in progress; modules/CMake source paths still matched that revision. Coverage now names
1328/2326 XNA members (290/329 types), 158/3210 C routes. Full runtime columns now include all 422 IDs and these changed-demo check counts. No new CNA issue.

The output-ordering fix is committed at 4b4e608. Its buffered-pipe fixture was verified red/green;
the real 30-demo content batch then matched all 26 passes/four expected failures and intact
failure lines from 419. Complete checkpoint 422 now also passes. Logs: `/tmp/cna-process-output-{red,green}-{build,test}.log` and
`/tmp/cna-process-output-content.{json,log,stderr.log}`. Its full smoke validation passed in 422.

Historical 421 review:
SOFTWARE 421 completed with expected demo statuses,
but strict assertion comparison requested review: ISSUE-011's "Unload disposed the loaded
texture" was split after "Unlo" by child stdout from later standalone XML/pipeline demos
(raw smoke-SOFTWARE.log lines 524-645). JSON independently retains the complete original
assertion. Original SOFTWARE report/stdout/stderr are already archived as checkpoint-421-SOFTWARE.*.
Manual review moved the existing suffix before the exact interposed 5,885-byte / 121-line block
in checkpoint-421-SOFTWARE.reviewed.log; no byte was added or removed, and all 24 failure
markers remain. checkpoint-421-log-review.json records raw/reviewed/block SHA-256 values and
corroborating JSON. Strict comparison to 419 now passes for the reviewed copy. Use this copy
for the historical 422 SOFTWARE comparison, retaining raw evidence. Later comparisons use 422 directly. Separate stderr capture works; this is a DIFFERENT buffered parent/child stdout
ordering defect in RunProcess (std::system starts the child before pending parent streams are
flushed). The raw log and the narrowly reviewed comparison copy with hashes are preserved.
OPENGLES3 subsequently completed and matched directly. The process regression
fixture `/tmp/cna-process-output-test.cpp` plus `/tmp/cna-check-process-output.cmake` was added to the
existing CMake test build and verified red/green as described above. The /tmp process drafts
are applied and stale. The repair is built and tested, including full checkpoint 422.
The fix drains C++ and C standard output streams before child execution; no parser normalization.
This is demo framework tooling, not a new CNA issue. Both new demos already pass targeted checks:

- graphics.model.part_sampler_state: 4 checks per renderer, stored clamp metadata versus
  application wrap and explicit selection, verified red/green pixels. Both screenshots inspected
  and exactly RGBA-identical (`/tmp/cna-part-sampler-{SOFTWARE,OPENGLES3}.{json,log,png}`).
- c_api.game.host_frames: 17 child checks without screenshot, 19 with it; copied callback tables,
  first draw suppressed, second drawn, exit/EndRun once, terminal false and unload on destruction.
  Exact trace ILBUbUbDeXEZ on both renderers; both constant-color captures inspected and identical
  (`/tmp/cna-host-frames-{SOFTWARE,OPENGLES3}.{json,log,png}`). Parent JSON checks=0 is normal for
  standalone demos; inspect child stdout and exit status. No browser-scheduling/error-path claim.

Build `/tmp/cna-421-model-host-build.log` is clean; metadata 421/0 and four targeted CTests pass
(`/tmp/cna-421-model-host-ctest.log`). CNA was clean at e60fa35f7803a2109ae8baed06ac5827f8e59b7b
before this build; subsequent changes are external. Coverage is 290/329 XNA types,
1327/2326 named members and 158/3210 C routes. Runtime columns now contain the complete 421 results. No new CNA issue. After the checkpoint, extend mesh_parts
with a meaningful mixed-topology part, or begin the separately audited SkinnedModelEXT workflow.

New `graphics.model.mesh_parts` passes all 8 checks on SOFTWARE/OPENGLES3:
shared buffers, distinct triangle/index/base-vertex ranges, effect collection deduplication,
retention/replacement, and two colored/two untouched pixel samples. Both screenshots inspected;
RGBA data is identical. Evidence: `/tmp/cna-mesh-parts-{SOFTWARE,OPENGLES3}.{json,log,png}`.
Build `/tmp/cna-419-mesh-build.log`, metadata 419/0 and its CTest plus two catalog checks pass
(`/tmp/cna-419-mesh-ctest.log`). Coverage: 290/329 XNA types, 1327/2326 members, 156/3210 C routes.
The complete 419 checkpoint now includes its runtime evidence in both coverage columns. No new CNA issue.
The /tmp mesh draft is applied and stale. Next productive Phase 7 work is per-part sampler/topology
behavior or the distinct SkinnedModelEXT carrier; the audit separates remaining members.

The /tmp sampler and C host-frame drafts have been applied and validated; they are stale copies.
Do not overwrite subsequent source changes with them.

The model topology, skinned_parts and two_bones palette drafts under /tmp are applied and
validated; they are stale copies. After completing 422, continue imported model/clip coverage
or the source-audited isolated MediaLibrary SavePicture workflow in ROADMAP. Do not restart
completed model scaffolding or treat type/member counts as comprehensive coverage.

Applied after full 422 (build/runtime validation pending):
`/tmp/cna-part-sampler-import.cpp` extends existing part_sampler_state with a third panel
loaded from an original loose glTF, checking unlit material/texture dimensions, imported
NEAREST/CLAMP_TO_EDGE metadata and actual out-of-range UV sampling. `/tmp/cna-gltf-models-sampler.py`
is the proposed generator update (adds sampler_quad and calls it from write_all, which is
already invoked by tools/generate_assets.py). The generated fixture is
`/tmp/cna-sampler-draft/models/loose_sampler_clamp.gltf`, 1,940 bytes, SHA-256
f43def05dedab6401eb6346dd5a07b484ab7c4ecd10e3c03ebdfdae0001b5e59. Two generations matched;
its embedded PNG decodes to the intended red/lime texels. No new demo ID or compiled-CNB
texture claim. 422 is complete and archived; these /tmp drafts are now applied and stale.

The second applied draft (build/runtime validation pending): `/tmp/cna-media-save-picture.cpp`
proposes media.media_library.save_picture as a standalone program (metadata draft at
`/tmp/cna-media-save-picture-metadata.cpp`, CMake draft `/tmp/cna-media-save-picture-CMakeLists.txt`).
It writes private XDG
configuration, verifies both resolved platform media roots before constructing MediaLibrary,
saves a generated panorama, checks picture/album views and exact GetImage bytes, saves the
GetThumbnail stream and reindexes both files. No HOME change or internal CNA test override. Source review found SDL_getenv uses a cached
environment: the standalone must set native XDG_CONFIG_HOME BEFORE its first CNA/SDL use,
not from an already-initialized browser. The resolved-root guard still runs before library access.
`/tmp/cna-generate-assets-picture.py` adds one framed_gradient(640,320) call to the existing
asset generator; `/tmp/cna-picture-draft/media/library_panorama.png` is its deterministic
1,666-byte output (SHA-256 95558108dc328537129e2b3b987dc9872f345c04f93dcc3b9dfbca76d5fc924a).
The public Picture.hpp GetThumbnail comment says no thumbnail is generated, but Picture.cpp
uses ThumbnailGenerator since MEDIA-210 (MaxEdge=128 internally). Verify the smaller returned
image at runtime before recording this documentation contradiction as a CNA issue; do not
assert 128 as a universal XNA contract. No new issue has been recorded yet. The draft expects
11 checks when Linux/XDG isolation succeeds and skips before library access otherwise.
It does not prove arbitrary short-read stream handling or other platforms.

Previous 418 batch (included in its complete checkpoint):

- New `c_api.game.foreign_thread_dispatch` passes 14 child checks under SOFTWARE/OPENGLES3:
  default THREAD refusal, opted-in explicit owner pumping of a worker's target-step read/write,
  and THREAD refusal after disabling dispatch. Evidence: `/tmp/cna-c-dispatch-{SOFTWARE,OPENGLES3}.{json,log}`.
  It creates a game but never starts its loop or draws. Automatic callback pumping, forwarded
  errors and actual pending-call cancellation are unclaimed; emergency cleanup was not triggered.
- Existing `c_api.math.vectors_and_matrices` passes 18 child checks on both renderer selections:
  linear direction transform, source/destination subranges, correct positions, guards and
  refused ranges preserving every output byte. Evidence: `/tmp/cna-c-math-ranges-{SOFTWARE,OPENGLES3}.{json,log}`.
  The backing arrays include extra guards beyond declared counts. No overlapping-array claim.

Build `/tmp/cna-418-c-abi-build.log`, metadata 418/0 and four targeted CTests pass
(`/tmp/cna-418-c-abi-ctest.log`). Actual compile commands use C11 for dispatch and C99 for math.
Coverage is regenerated (290/329 XNA types, 1321/2326 members, 156/3210 C ABI routes); the generated
runtime columns now include all 418 IDs after the complete checkpoint. No new CNA
issue. Parent standalone JSON checks=0; child counts are in stdout. Both C /tmp drafts are now
applied and stale; do not reapply them. C11 guidance now covers the two threading examples.

Previous 417 batch (included in its complete checkpoint):

- Existing cnb_loader passes 12 checks on both renderer selections; four new cases have valid
  containers but missing/duplicate RULE, a truncated lives integer or trailing schema byte.
  ParseFile succeeds before typed-load rejection, which names file/chunk and returns no value.
  Evidence: `/tmp/cna-cnb-boundaries-{SOFTWARE,OPENGLES3}.{json,log}`.
- Existing C lifecycle passes 18 child checks with screenshots, 16 without in CTest: valid
  LoadContent borrow, INVALID_HANDLE in the next Update, fresh usable borrow and INVALID_STATE
  outside callbacks after Run. Evidence: `/tmp/cna-c-borrowed-{SOFTWARE,OPENGLES3}.{json,log,png}`.
  Both screenshots inspected (uniform game-time-dependent clear color); no fixed-color equality
  claim. Parent JSON checks=0, child counts are in stdout.
- New `audio.sound_effect_instance.lifetime` passes 7 checks under both renderer selections
  with dummy audio: temporary wrapper survival, move of a playing voice, idempotent instance
  disposal/rejected Play, and effect disposal of moved/idle children. Evidence:
  `/tmp/cna-audio-lifetime-{SOFTWARE,OPENGLES3}.{json,log}`. No audible quality claim.

Build `/tmp/cna-417-boundaries-build.log`, metadata 417/0 and five focused CTests pass
(`/tmp/cna-417-boundaries-ctest.log`). No new CNA issue. Coverage now names 290/329 XNA types,
1321/2326 members and 150/3210 C ABI routes; the complete checkpoint-417 now includes the new audio ID and all extensions. The /tmp drafts
for these three changes are applied and stale; do not reapply them.


The ModelMesh/ModelMeshPart audit is in COVERAGE_AUDIT.md and now includes the validated
shared-buffer example above. Sampler source review now resolves the initial suspicion: CNA's easygl_gltf_sampler_wrap_test
explicitly documents GLTF-208, preserving application-owned device state and requiring explicit
selection of a part sampler. Do not report missing automatic assignment as a CNA bug. A small
procedural part-sampler metadata/explicit-selection pixel demo is a useful next step; importing
actual authored glTF sampler fixtures remains separate. SkinnedModelEXT is distinct from
the already demonstrated Model/SkinningData route. The C runtime audit also identifies host-driven
run_frame_ext and GameFrameHooks as unexecuted behavior; no draft exists for those yet.

Prior semantic-416 extensions (included in the completed checkpoint):

- C content root extension is applied and passes **41 child checks with screenshots** on both
  renderers (`/tmp/cna-c-content-root-{SOFTWARE,OPENGLES3}.{json,log,png}`), **39 without** in
  CTest's LastTest.log. It verifies setter/count/copy, caller-buffer reuse, retained checker
  cache hit, new sprite-strip resolution and IO for the old name after Unload. Existing owned
  checker/font handles still render. Screenshot RGBA pixels are identical to the preceding
  atlas-guard baseline on each renderer; expected checker colors and 1972 bright text pixels
  were verified directly in both PNGs. SOFTWARE visually inspected.
- SaveAs image extension is applied and passes **8 checks** on both renderers
  (`/tmp/cna-save-dimensions-{SOFTWARE,OPENGLES3}.{json,log,png}`). PNG/JPEG decode at the
  requested 64x32 size and the 128x128 source stays unchanged. The comparison loop now guards
  JPEG output shape. Labeled screenshots inspected, source/PNG panel pixels match exactly on
  each renderer, and between-renderer RGBA differences are at most 1 per RGB channel (alpha 0).

Build, metadata (416/0), both demo CTests and both catalog tests pass (4 tests). Coverage is
regenerated: 290/329 XNA types, 1318/2326 named members, **149/3210 C ABI routes**. No new CNA
issue. All /tmp drafts for these extensions are applied and stale; do not reapply them. The C
root setter's cache contract differs from the C++ loaded-root restriction (ISSUE-039).

Both semantic extensions are applied and pass: cnb_read_limits now has 11 checks with Zstd
available on both renderer selections (`/tmp/cna-compressed-limits-{SOFTWARE,OPENGLES3}.{json,log}`).
The 416-byte container shrinks to 328 bytes, restores identical logical chunks, keeps metadata
uncompressed, enforces the 207-byte decoded total against a 206-byte cap, and restores the original
image when compression is disabled. No timing/allocation measurement or unsupported-build run.
C error_handling passes 26 child checks, including short/version-incompatible diagnostic queries
preserving the original error (`/tmp/cna-diagnostic-layouts-{SOFTWARE,OPENGLES3}.{json,log}`).
Build, metadata (415/0), both demos' CTests and both catalog tests pass (4 tests). No new CNA issue.
Their /tmp drafts are stale; do not reapply. Coverage audit is updated; these extensions are
now included in the complete checkpoint-416. Checkpoint-415 remains the historical pre-extension report.

The two CNB drafts are applied and pass: `content.custom.cnb_external_references` has 9 checks
and `content.custom.cnb_read_limits` has 8 under both SOFTWARE/OPENGLES3 selections. Evidence:
`/tmp/cna-cnb-{references,limits}-{SOFTWARE,OPENGLES3}.{json,log}`. Build, metadata (415/0),
both demo CTests and both catalog tests pass (4 tests, `/tmp/cna-415-cnb-ctest.log`). These are
console examples without rendering. No new CNA issue. Coverage/status audit is updated; full
reports now contain all 415 IDs. Do not reapply the stale /tmp CNB drafts.

The initial run passed all checks then aborted at exit. GDB and nm identified duplicate
TouchPanel statics in the executable and libcna.so, extending existing ISSUE-016. The standalone
C++ helper now uses the browser's shared-library-first link workaround; duplicate definitions
are absent and clean exit is verified. Details and before/after evidence are in CNA_ISSUES.md.
The draft CNB API typo was a separate demo compilation mistake, corrected to RequireSingle/OpenChunk.
The /tmp draft is stale; do not reapply it. Next productive work is composite asset dependency/
reference behavior or remaining member gaps in COVERAGE_AUDIT.md; do not reset completed phases.

The composite example resolves root-relative XREF names through its owning ContentManager,
checks shared versus manager-local caches, invalid index/schema type, missing child/retry and
writer rejection of `..`. It does not establish cycles, automatic child compilation or XNB fixups.
The 416-byte tile layer checks independent writer/reader budgets plus schema limits after Parse
succeeds; its later compression extension is described above. Other limits remain explicitly
uncovered in the member audit.

The existing C audio ownership extension is applied and passes all 19 child checks on both
renderer selections (`/tmp/cna-c-audio-ownership-{SOFTWARE,OPENGLES3}.{json,log}`), including
four new checks for early parent release refusal, preserved duration and stale instance handles.
Build, metadata (415/0), its CTest and both catalog tests pass (3 tests). Dummy audio only;
no audible or visual validation. No new CNA issue. Its /tmp draft is now stale.


Next after this checkpoint: remaining CNB compression/limits behaviors or C ABI handle and
versioned-structure boundaries from COVERAGE_AUDIT.md. Keep the existing focused demos where
an extension fits; no per-enum or cosmetic variants. Composite nested build publication remains
separate from the now demonstrated runtime XREF loading.

The new `graphics.texture2d.copy_ownership` is applied and fully targeted-validated. All 6
checks pass on SOFTWARE/OPENGLES3, including actual backbuffer pixels after source disposal.
Both screenshots were inspected, with byte-for-byte identical PNG files (all RGBA channels).
Evidence: `/tmp/cna-texture-copy-{SOFTWARE,OPENGLES3}.{json,log,png}`. Build, metadata (416/0),
its CTest and both catalog checks pass (3 tests, `/tmp/cna-416-texture-ctest.log`). The example
checks copied-but-independent Name/Tag, partial shared edits, full-upload detachment and shared
sibling survival. It does not establish every mip/raw-data overload or render-target alias.
No new CNA issue. The /tmp draft is applied and stale; do not reapply it.

The XML property example is included in the complete 412 checkpoint: all 5 child checks pass
under SOFTWARE and OPENGLES3 selections (`/tmp/cna-intermediate-properties-{SOFTWARE,OPENGLES3}.{json,log}`).
It verifies setter dispatch/rejection, get-only collection defaults/appending, inherited-member
ordering and flattened spawn coordinates. No new CNA issue. These are console checks, not
renderer or visual validation. Runtime XNB graphs and base-typed polymorphic XML loading remain
unclaimed. Its old /tmp draft is applied; do not copy it over later edits.

The original 398/401 checkpoint debt is closed. Full 401 and two 404 checkpoints also passed
this session. A subsequent 405 attempt passed all CTests but lost the inherited X connection
`:0` during SOFTWARE and never wrote a fresh report; it is **not** counted as completed.
Its raw log is `/tmp/cna-demos-405-software-interrupted.log`. The isolated phone lifecycle demo
passed under private Xvfb, and the complete 406 rerun used private Xvfb for the whole script.
No phone-demo defect was reproduced. Validation now removes stale current JSON outputs at
startup while preserving named archives; the failure-propagation/stale-report tests pass.

Completed this session (details and issue reproductions are in CHANGELOG and CNA_ISSUES):
- Five new focused demos: quaternion double cover, typed effect parameters, custom CNB rules,
  XACT category instance limits, and custom CNJ enemy factories. The latter uses nlohmann-json,
  already a README/CI dependency, explicitly linked through CNA's portable-header helper.
- Existing curves now cover asymmetric tangents, negative loops and degenerate collections;
  manifest shows explicit refresh independently of caching; texture rectangles use nonzero
  CPU offsets with untouched padding. Manifest and texture screenshots were inspected on both
  renderers; no new screenshot or audible evidence is claimed for console/audio extensions.
- Four XACT demos own caller-allocated GetCue results; the cue lifecycle checks Disposing and
  repeated disposal. The instance-limit demo exercises Fail/ReplaceOldest, paused slots and
  category isolation through public cue states.
- Custom CNB loader distinguishes optional/mandatory unknown chunks and retry after repair
  (8 checks). Custom CNJ loader demonstrates named factories, per-manager registration, cache,
  Unload preserving factories and payload errors (7 checks). Its three generated fixtures
  match the committed bytes exactly.
- Dynamic streaming exercises buffer sizing, duration, unsupported loop/release operations,
  queue cleanup and disposal (11 checks; exactly 2 failures from ISSUE-038). Its playback check
  now runs once despite fixed-step catch-up.
- New CNA records: ISSUE-034/035 (stock EffectParameter getter/reflection), ISSUE-036 (Curve
  post-loop tangent), ISSUE-038 (dynamic audio getters after Dispose); ISSUE-037 is a header
  comment contradiction only. No CNA source was modified. XNA decompilation/spec inspection
  is explicitly distinguished from genuine-XNA runtime execution.
- Validation preserves command failures and full logs, catalog checking avoids repeated large
  JSON parsing, and coverage exposes IDs missing from old reports. Member audits now include
  content/binary formats, audio/XACT, texture ownership, sprites/text and pipeline APIs.

**Custom pipeline batch included in the 407 checkpoint:**
`content.custom.pipeline_components` registers a public importer, processor and writer, builds
an original patrol route into CNB, checks deterministic output, precomputes open/closed lengths
7/12, loads its runtime schema and verifies contextual rejection of an incorrectly typed
processor parameter. All 6 checks pass on SOFTWARE and OPENGLES3, build/metadata (407/0) and
its CTest/catalog pass. Reports: `/tmp/cna-pipeline-components-{SOFTWARE,OPENGLES3}.json`.
The generator reproduces `assets/custom/patrol.route` byte-for-byte. No CNA issue was found.
Earlier batches (historical evidence; immediate next action is above):
1. **408th demo included in the full checkpoint:** `content.custom.intermediate_serializer`
   is applied and built, with 6 checks passing under both renderer selections (it is a console
   build-time tool, not a rendering test). Named/optional/ignored fields, collection item names,
   XML round trip/defaults and ignored-field rejection are exercised. The separate tool links
   CNA::ContentPipelineBuild; metadata stays in the runtime browser. Evidence:
   `/tmp/cna-intermediate-{SOFTWARE,OPENGLES3}.{json,log}`. Standalone reports have checks=0;
   the actual 6 checks are in child output. Targeted CTest/catalog/metadata (408/0) pass.
   Next content work: explicit-null/flattened/shared-resource behavior, XNA adapters and
   composite asset dependency/reference graphs. Do not credit nominal .NET attribute wrappers
   just because corresponding descriptor behavior is used.
   **409th demo included in the full checkpoint:** `content.custom.intermediate_shared_resources`
   is applied and built. Its 6 child checks pass under both renderer selections: Resources
   emission, forward fixups, aliases, equal-but-distinct resources, inline copies and mutation.
   Evidence: `/tmp/cna-shared-xml-{SOFTWARE,OPENGLES3}.{json,log}`. Targeted CTest/catalog and
   metadata (409/0) pass. The graph is acyclic; cycles, malformed IDs, explicit Null, flattening,
   external references and custom serializer callbacks remain unclaimed.
2. Further ContentManager lifetime and lookup-order behaviors. The root mutation check is
   now completed in the existing cache_and_unload demo: both renderers execute 5 checks with
   exactly one failure (ISSUE-039), while original cache checks and the post-Unload root switch
   pass. Screenshots inspected on both; build/metadata (407/0), CTest and catalog checks pass.
   Reports/screenshots: `/tmp/cna-content-root-{SOFTWARE,OPENGLES3}.{json,png}`. This extension
   is included in the full 407 checkpoint.
3. Further sprite/text behavior gaps in the audit. The two prepared sprite drafts have now
   been applied: layer-depth sorting passes 4 pixel checks, and texture sorting passes 2 with
   blank-row rejection, on SOFTWARE and OPENGLES3. All four screenshots were inspected. Build,
   metadata (407/0), both CTests and catalog validation pass. Evidence:
   `/tmp/cna-{layer_depth_sorting,sort_mode_texture}-{SOFTWARE,OPENGLES3}.{json,png}`.
   No new CNA issue found. These revisions are included in the full 407 checkpoint.
4. Remaining Phase 9 work: authored RPC/variation tables, cue-level limits/priorities/fades;
   category limits are already complete. Preserve ownership and distinguish state observation
   from audible evidence.
5. The existing C content demo now checks texture pixels and font metrics after Unload and
   manager destruction, plus checker colors in the combined sprite/text frame. Both renderers
   pass (29 child checks with screenshots, 27 without). Images were inspected and the final GL
   image is pixel-identical to the original pre-extension demo. A temporary suspicion from
   a misread screenshot preview was disproved by the actual PNG pixels; no ISSUE-040 or
   expected-failure marker remains. Evidence: `/tmp/cna-c-content-{SOFTWARE,OPENGLES3}.{json,log,png}`.
   All diagnostic source variants were restored. Further ABI gaps are in COVERAGE_AUDIT.md.
   The C error-boundary extension is now applied: failed diagnostic queries preserve the original
   message/category; a refused type-name copy preserves all bytes; an exact-sized copy leaves
   the following sentinel intact. All 17 child checks pass under both renderer selections;
   reports `/tmp/cna-c-errors-{SOFTWARE,OPENGLES3}.{json,log}`, targeted CTest/catalog passed.
   Later batches below now cover thread isolation and canonical exception type/parameter
   diagnostics; further invalid-release guards remain a gap. No new CNA issue was found.

**Semantic extensions included in the 410 checkpoint:** existing demo IDs were extended, not added.
- `content.xnb.custom_type_reader`: 5 checks pass on SOFTWARE/OPENGLES3, including actual Read
  invocation counts (first load once, second cache hit) and the logical AssetName/owner pointer.
  Evidence: `/tmp/cna-xnb-reader-{SOFTWARE,OPENGLES3}.{json,log}`. The prepared /tmp draft is
  now applied; do not copy it again. Scene rendering was unchanged; no new XNB screenshot claim.
- `content.custom.intermediate_serializer`: 9 child checks pass under both renderer selections.
  Absent Optional preserves a nonempty constructor default, a present value overrides it,
  explicit Null clears it, and missing required HitPoints is rejected. Evidence:
  `/tmp/cna-xml-optional-{SOFTWARE,OPENGLES3}.{json,log}`. This remains a build-time console tool.
- `c_api.content.content_manager`: 31 child checks with screenshots (29 without) pass on both
  renderers. Destroying the live font's retained atlas returns INVALID_STATE; subsequent manager
  Unload/destruction and sprite/text drawing still work. SOFTWARE screenshot inspected; both
  resulting images are pixel-identical to their pre-guard baselines. Evidence:
  `/tmp/cna-c-atlas-guard-{SOFTWARE,OPENGLES3}.{json,log,png}`.
Build, metadata (409/0), all three targeted CTests and both catalog tests pass. No new CNA issue
was found. These extensions now have complete 410 checkpoint evidence; checkpoint-409 remains
the historical archive of revision 7a64310 before them.

**Full semantic 409 run completed at source revision ce972da:** tools/validate.sh exited 0 under
private Xvfb (`/tmp/cna-demos-409-semantic-validation.log`); build, metadata, 421 enabled CTests
and both complete smoke runs passed their per-demo gates. Both 409-ID/status maps exactly match
checkpoint-409; the XNB reader now records 5 checks instead of 3. Archives:
`reports/semantic-409-{SOFTWARE,OPENGLES3}.{json,log}` and `{build,ctest,validation}.log`.
However, the complete assertion review found **one additional SOFTWARE failure**: the network
simulation demo observed 79% unreliable delivery before enabling simulated loss (68 ms mean,
100% reliable). Raw failed-assertion totals are 25/25, against the corrected baseline 24/25.
OPENGLES3 is unchanged. An isolated SOFTWARE repeat delivered 100% unreliable and reliable,
with only ISSUE-030 failing (`/tmp/cna-network-baseline-repeat.{json,log}`).
The public SendDataOptions::None contract explicitly allows dropped packets; the demo's >90%
unsimulated delivery gate incorrectly treats an empirical loopback expectation as a guarantee.
The 410 revision corrects both statistical gates with coordinated finite bursts and exact counts. Do not describe the semantic-409 run as
having only the documented CNA assertion failures, or erase its original evidence. No new CNA
bug is established by allowed unreliable loss. Runtime XNB graph fixups remain separate from
IntermediateSerializer sharing and are not established by the reader-context checks.

**410-demo batch, full validation complete:** new standalone
`content.custom.intermediate_type_serializer` builds and passes 7 child checks under SOFTWARE and
OPENGLES3 selections (`/tmp/cna-custom-xml-codec-{SOFTWARE,OPENGLES3}.{json,log}`): compact NdM
syntax, root shortcut, described-field dispatch, round trip and invalid input/output. Metadata
validation reports 410/0. Coverage is regenerated (290/329 types, 1318/2326 named members);
full smoke now covers every 410 ID. The old /tmp draft is applied.

The existing network simulated_conditions demo now uses coordinated 24-pair bursts for baseline,
200 ms delay and complete outage, followed by one second of recovery with loss disabled. Exact
sent counts expose missing endpoints; wall-clock pacing avoids catch-up bursts. Both renderer
runs pass 4 checks with only ISSUE-030 failing: 24/24 reliable baseline/delayed, 0/24 after outage
and recovery. Evidence: `/tmp/cna-network-coordinated-{SOFTWARE,OPENGLES3}.{json,log,png}`.
The earlier stochastic failures remain in `/tmp/cna-network-baseline-*`; no new CNA issue or
widened expected-failure marker was introduced. Build, catalog and both changed demos' CTests
pass (4 tests, network WILL_FAIL); the raw CTest log contains only ISSUE-030's assertion.
Final screenshots were inspected on both renderers; the wording distinguishes no arrivals
from measured zero latency. Full validation is complete (section 7). Use
`tools/compare_checkpoints.py` on each renderer's JSON/raw log pair and the current catalog. Its
4 regression tests pass; real archive tests detect the masked semantic-409 failure and reject
an old 409 report against the 410 catalog. The tool never accepts a changed baseline automatically.

**Semantic boundary batch after checkpoint-410:** both prepared drafts are now applied and
validated; do not copy the old /tmp files again. Existing shared XML tool passes 10 child checks
under SOFTWARE/OPENGLES3, including null tables, missing/duplicate IDs and incompatible resource
types (`/tmp/cna-shared-resource-boundaries-{SOFTWARE,OPENGLES3}.{json,log}`). Existing C diagnostics
passes 24 child checks under both selections: native versus canonical argument errors, structured
ArgumentOutOfRangeException/ParamName=value and clearing those details after an invalid handle
(`/tmp/cna-c-error-canonical-{SOFTWARE,OPENGLES3}.{json,log}`). No new CNA issue. Invalid volume
is rejected before mixer initialization or mutation; no audio playback or thread-isolation claim.
Build, metadata (410/0), the two demos' CTests and both catalog checks pass (4 tests). Coverage
is regenerated: XNA 290/329 types and 1318/2326 members, C ABI 143/3210 routes. Full checkpoint-410
archives describe source 4994374 before these two extensions. The **full semantic-410 checkpoint
at b3fd470 now passes** under private Xvfb, with every ID/status and 24/25 failed-assertion
multiset unchanged. Archives are `semantic-410-*`; child checks are recorded in raw stdout. Then continue with C diagnostic thread
isolation or XML properties/flattening/XNA adapter/dependency behavior from the member audit.
Runtime XNB fixups and graph validation remain a separate unclaimed protocol.

**411th demo included in the full checkpoint:** `c_api.core.thread_local_errors` is applied and built;
the old /tmp draft must not overwrite later edits. All 8 child checks pass with SOFTWARE and
OPENGLES3 selections (`/tmp/cna-thread-local-errors-{SOFTWARE,OPENGLES3}.{json,log}`), plus its
CTest and both catalog tests (3 tests). Metadata is 411/0; coverage now names 146/3210 C routes
and unchanged XNA 290/329 types / 1318/2326 members. The complete reports now contain all 411 IDs. The example creates no Game and performs no rendering/audio. Two workers
write deliberately different argument failures in a controlled interleaving; each re-queries its
result/category/message after the other's write, and the caller's original diagnostic survives.
Bounded atomic waits avoid an abandoned barrier. Canonical exception-detail isolation and
foreign-thread Game dispatch remain separate gaps; the math routes exercise argument validation,
not successful arithmetic.

This one target uses C11 threads/atomics after a successful compile/link availability probe;
compile_commands.json confirms `-std=c11` here and `-std=c99` for existing C error_handling.
DEMO_AUTHORING/ARCHITECTURE document this narrow exception; missing C11 support omits the
executable while leaving metadata discoverable. Only Linux GCC 14 runtime is validated.
**Full 411 validation completed at source 82c5abc**, log `/tmp/cna-demos-411-validation.log`,
under private Xvfb. SOFTWARE 366/17/25/3, OPENGLES3 387/18/3/3 and 423 enabled CTests passed;
the exact 24/25 failed-assertion multisets match semantic-410. Archives are checkpoint-411.
The XML property example is now applied and validated as described in the immediate next action
above; continue with the XNA adapter/dependency or remaining C ABI member audit. Do not reset completed phases.

Effects constraints verified in the current sources/build: all four configured
`CNA_*_COMPILED_EFFECTS` switches are OFF and `CNA_FXC_EXECUTABLE` is empty. PbrEffect's public
ImageBasedLightEXT bundle and GraphicsDevice::SupportsImageBasedLightingEXT do exist, including
EasyGL support, but the EnvironmentProcessor named in old CNA audit placeholders is absent from
the tracked source tree. A meaningful IBL demo still needs a consistent authored/generated
irradiance cube, prefiltered specular cube and BRDF table; arbitrary unrelated textures do not
establish it. Do not restart completed phases while these focused gaps remain.

Not exercised here: `gamer_services.signed_in_gamer.signed_out` is manual -- its start-up was run
(GL, Xvfb), the sign-out itself needs a person at the Guide (no input injection in this container).

Net facts learned (two-process demos): a standalone program can be both sides -- the first process
hosts and starts itself again (`CnaDemos::RunProcess` on a worker thread, `--joiner`/`--player`),
the child signs in its own profile (set `CNA_GAMER_SERVICES_AUTO_SIGN_IN`, not defaulted: children
inherit the parent's environment). Host first, then the searcher (ISSUE-026). Without
AllowHostMigration the client gets SessionEnded(HostEndedSession) when the host disposes its
session; with it, HostChanged (old host remote, new host local), GamerLeft, and the survivor is
host. Session property filters in `Find` keep rounds apart.

Model facts learned (glTF batch): a metallic-roughness material imports as PbrEffect (unlit fallback
on SOFTWARE/HEADLESS/SDL_RENDERER); KHR_lights_punctual lights fill the effect's three directional
lights, and a file without lights gets EnableDefaultLighting; compiled models keep lights; the
content pipeline refuses KHR_materials_variants (the CNB model cannot carry them), so variant
models are loaded as loose glTF; clips keep only key times (ISSUE-031).

Gamer services facts learned (audit batch): the Guide and its notifications are a runtime overlay
drawn after `Game::Draw` and before `EndDraw` -- a demo that looks at them reads the back buffer in
an `EndDraw` override (then calls `DemoGame::EndDraw`); notifications slide in over 0.22 s of real
time, so an unattended run waits before looking. Offline, a leaderboard entry is saved when its
rating is set (ISSUE-025). `ReadOnlyCollection` returned by value (e.g. `getEntriesProperty()`):
bind it to a `const` variable before indexing -- the non-const `operator[]` of a read-only
collection throws "Collection is read-only", and a reference into a temporary dangles.

C API facts learned (Phase 14): a C game's device starts in XNA's Reach profile, where
`GetBackBufferData` is refused -- `cna_demo_use_hidef` (CnaDemos/CApiDemo.h) creates a graphics
device manager, requests HiDef and applies it; `cna_game_destroy` refuses (INVALID_STATE) while any
owned child lives and only then raises exiting/unload_content, so children are released between
`cna_game_run` and `cna_game_destroy`; destroying a game whose callback failed releases it and
returns CALLBACK again; XNA's Run calls Update once with zero elapsed time before the loop, and a
fixed-step update's TotalGameTime is the time before that step; the C API library
(`libcna_c_api.so`, CNA linked statically inside, ~3210 routes) is excluded from `all` and built
when a C demo links it -- enabling it rebuilds CNA's static modules as PIC (a long first build).

C API facts learned (second batch): a C component is a callback table and a context (C cannot
implement IUpdateable/IDrawable); the C game's draw callback runs first and the drawable components
draw after it, by DrawOrder, so a component with the highest DrawOrder sees the finished frame;
component and collection events are raised synchronously by the setters and `cna_game_components_*`;
event registrations keep their component alive and are released first. MediaPlayer's events are
process-wide (no game handle) and are raised by a later frame's dispatcher update, not by the call;
a song collection retains its songs, so the program's song handles can go at once; the media queue
handle is a view released with `cna_media_queue_destroy`. A `CNA_GamePadState` holds one button
mask: physical buttons, the D-pad and the derived stick/trigger bits.

Net facts learned (join refused): two processes that must take turns (find, then start, then join)
signal each step with empty files in a temporary directory passed as `--signals DIR`; a System Link
host refuses a joiner by disconnecting it (ISSUE-032), and the welcome lacks the session state
(ISSUE-033); `Find` lists only sessions with room for the searcher, so a full session is not
reachable through it.

Graphics/media facts learned: `VertexBuffer::GetData(data, startIndex, count)`'s startIndex is the
*destination* index (XNA); a source range needs the `GetData(offsetInBytes, data, startIndex,
count, stride)` overload. OpenGL ES 3.0 occlusion queries are boolean (`PixelCount` 0/1,
`isPixelCountPreciseEXT()` false). `VideoPlayer::GetTexture` is what decodes, and decoding past the
last frame sets State to Stopped (read State before it). Compiled songs/videos (`content_media/`,
FFmpeg needed at build time) are loaded through a ContentManager rooted at `assets/media`
(ISSUE-020).

Phase 13 facts learned: sensors are unsupported on a desktop (State NotSupported, Start throws
AccelerometerFailedException) unless the CNAEXT keyboard emulation is on, which reads (0, 0, -1) g
at rest; `Environment::DeviceType` is Device on mobile platforms and Emulator elsewhere;
`VibrateController::Start` is silent without a motor and refuses more than 5 s; this build has
`CNA_DIAGNOSTICS=OFF`, so the diagnostics macros compile to nothing and snapshots have no metrics;
CRTEffect/DepthEffect are ShaderEffects (GL/Vulkan only), AsciiPostProcessEffect reads back on the
CPU (slow on SOFTWARE: keep frames low); System::Object subclasses must override `GetTypeName`;
`PhoneApplicationService::AttachEXT` raises Launching at once and Closing from Game::Exiting.

Net facts learned: CNA's Local/LocalWithLeaderboards sessions open no sockets and deliver no
packets (`SendData` is a no-op there), System Link sessions do (UDP via ENet; a host binds the
discovery port 61190 and replies to searches); subscribing to `GamerJoined` replays it for every
gamer already in the session (XNA's add accessor), `GameStarted` likewise while Playing;
`StartGame`/`EndGame` take effect at the next `NetworkSession::Update`, and ending a game clears
every `IsReady`; `SendData(PacketWriter&)` sends the bytes before `Position` and rewinds it (Length
is kept); all local gamers on the host machine report `IsHost` (CNA follows FNA; XNA's exact rule
unverified), `NetworkSession::Host` names one; the caller owns a created session: `Dispose`, then
delete.

Gamer services facts learned: `GamerServicesDispatcher::Initialize` throws on a second call, hence
`ownProcess`; `GamerServicesComponent` must be added through the `shared_ptr` overload of
`Components.Add` (the raw-pointer one with a separately owned component double-frees);
auto-signed-in profiles appear (and raise `SignedIn`) in the first Update; the Guide overlay
draws message boxes, keyboard input and toasts on SOFTWARE and OPENGLES3 alike; `ShowSignIn`
skips player slots already signed in (a one-pane sign-in with player one signed in shows
nothing); `Guide::IsTrialMode` is true until the first dispatcher update and follows
`SimulateTrialMode` one update later; offline achievements come from
`GamerServices/Achievements.json` beside the executable (unknown keys throw
`GamerServicesNotAvailableException`) and are stored under the storage root; offline
`GamerProfile` totals are this title's earned achievements; `Gamer::GetFromGamertag` throws
`NotSupportedException` offline; a local profile may not play online sessions or purchase.
Avatars: `AvatarRenderer` draws on SOFTWARE and OPENGLES3; `ParentBones`/`BindPose` throw until
State is Ready; identity bone transforms are the bind pose (the custom-pose demo turns the avatar
with World = RotationY(Pi) to face the camera). Leaderboards offline: a rating written through
`LeaderboardWriter` is stored at once, and the offline store keeps the last rating written (it
applies no best-score rule; the demo writes only improvements, as a game would); `LeaderboardReader::Read` reads this machine's profiles.

Model facts learned: glTF with `KHR_materials_unlit` + `COLOR_0` maps to BasicEffect (stride 24),
skinned + unlit + COLOR_0 to SkinnedEffect (stride 56); without the unlit material a primitive
gets PbrEffect/SkinnedPbrEffect even with vertex colours. CNA adds a synthetic root bone named
"Root" above the scene's nodes (avoid that name for nodes). `Copy*BoneTransformsTo` need a
destination already sized to `Bones.Count` (XNA's rule). The glTF import report is filled only for
`.gltf`/`.glb` loaded at run time (and Model `.cnj`), not for pipeline-compiled models.

Media facts learned: CNA plays music through SDL_mixer, so generated WAVs work as songs; a file
Song's Duration is 0 without CNA's FFmpeg probe (playback still ends correctly); MediaPlayer's
events are raised by `FrameworkDispatcher::Update` (the Game loop calls it); `MediaPlayer::Play`
queues copies of the songs; `MovePrevious` on the first song stays there (FNA's rule).

Content facts learned: CNB schema 1 stores textures as RGBA8 only (DXT needs `--format xnb`;
`content_xnb/` exists for that); compiled textures are premultiplied by default, loose PNGs are
straight; `Texture2D` copies initially share one texture (Name/Tag are per-wrapper copied values).
Source audit: a partial rectangle update changes that shared texture, while `SetData(Color*, int)`
replaces only the writing wrapper's texture. The copy_ownership demo now verifies this sequence and shared sibling survival after disposal
with rendered pixels on both renderers; do not generalize it to every mip/raw-data overload. `ContentManager`'s
`Load<T>` ignores `OpenStream` overrides (ISSUE-012) and `Unload` only clears the cache
(ISSUE-011). Storage facts: demos call `StorageDevice::SetAppNameEXT("cna-demos")` and delete their
containers; saves go to `$XDG_DATA_HOME/cna-demos` or `~/.local/share/cna-demos`.

Input facts learned (Phase 10 is complete): windowless SOFTWARE has no clipboard/text input/devices
(use `Need::Window`); `Mouse::SetPosition` is reported by the next `GetState` on both renderers;
static CNA events (`Mouse::ClickedEXT`, `TextInputEXT::TextInput`, hot-plug events) must be
unsubscribed in `UnloadContent`; process-wide state (cursor, relative mode, keyboard emulation,
touch mouse emulation, enabled gestures) must be restored there too. GamePad keyboard emulation
(CNAEXT) makes player one connected, so gamepad code is checkable without hardware. Direct
`cna-demos <id>` runs are interactive (no frame limit; add `--frames N` or `--automated`), and SDL
turns SIGTERM into a quit event, so kill stuck runs with SIGKILL.

After content: Phase 7 (models/glTF) needs
generated glTF assets (a small script in tools/). Geometry for 3D demos: `CnaDemos/Geometry.hpp`
(sphere, cube). Review graphical demos with `python3 tools/contact_sheet.py --exe
build/linux-full/bin/cna-demos --renderer SOFTWARE --prefix <id-prefix> --out /tmp/sheet.png` (add
`--renderer OPENGLES3 --xvfb` for GL) and look at the image. Each: read the CNA header (and the
implementation when semantics matter), write demos, build, run tools/validate.sh, review contact
sheets, update this file, commit.


## Checkpoints 455/456 preparation history (superseded by complete 456 at e70eeb5)

The following is the former handoff's chronological preparation record. Live/pending statements
inside it are historical; HANDOFF.md carries the current continuation. Full 456 ultimately
passed 469 enabled CTests and both smokes, preserving all original 35/36 failures.

## 14. Next task

**Immediate next action:** resume full 456 validation session 65041 at source e70eeb5,
/tmp/cna-demos-456-validation.log. Build passed (seven scheduled targets), metadata 456/0,
All 469 enabled CTests pass (186.53 s); SOFTWARE smoke is archived/reviewed, OPENGLES3 is running. Runtime hashes captured after this build: 35 binaries/four assets.
Do not edit/rebuild sources/assets during runtime. Archive each smoke using
/tmp/cna-archive-456.py SOFTWARE|OPENGLES3, then /tmp/cna-456-finish.py. Helpers name e70eeb5.
Manifest now passes 13 focused checks on each renderer with empty stderr: sixteen obsolete
registration assertions replaced by three inventory/lazy-family checks; ten controls retained.
Its earlier six failed assertions remain in checkpoint-456-manifest-*. Helpers permit only the
explicit manifest 26 -> 13 count change and the existing title timing variance; all old raw
failures/statuses/known issues and C/custom-content child lists/traces must still match 455.

Full 455 at 9c023d8: tools/validate.sh exit 0, all 468 enabled CTests (199.23 s), SW 402/25/25/3,
GL 423/26/3/3. Exact ORIGINAL 453 comparisons add only the two four-check material/specular IDs;
every old status, check count, known issue, 35/36 failed assertions and C/custom-content child
list/trace remains. All 34 binaries/four assets and normalized stderr match. Evidence/catalog/
helpers: reports/checkpoint-455-*. Both scenes pass on both selections; all focused stderr empty.
Four 800x480 captures actually inspected: material differs only 41,472 ambient-card pixels by
one green level (26 SW /25 GL), specular only two pixels by one red level; original tolerance 1.

Initial build FAILED in external uncommitted CNA cgltf implementation splitting: lost private
helper declarations and transitive C string/memory includes. Both new demo objects compiled,
but the executable did not link; all 34 old runtime binaries remained unchanged. Evidence:
reports/checkpoint-455-initial-*. Recorded as ISSUE-052. The external author added string.h and
CgltfInternals wrappers; retry succeeded (111 scheduled targets, known Draco CMP0148 developer
warning only, no compiler diagnostics). ISSUE-052 is resolved for the exercised build; no CNA
file edited/reverted and no warning suppressed by cna-demos. CNA/GamerServices refactoring
was uncommitted over 3ee6ae3 during build; later clean ced719909 was observed during smoke.
No atomic build-input snapshot is claimed.

All 47 targeted C/graphics/CLI CTests passed (18.80 s: 46 passes incl. three WILL_FAIL, one PBR
skip). Evidence, four images, repaired-source observations and retry log:
reports/checkpoint-455-focused-*. /tmp/cna-review-455-focused.py verifies the focused results and
exact image differences. The specular design JSON is analytic design only, not native mutation
validation. /tmp/cna-catalog-455.json has the current catalog.

Full archives include the actual catalog, comparison/finish scripts and final CNA state.
Clean ced719909 observed at completion; source was changing during preparation, so no atomic
source/input or complete transitive binary snapshot is claimed. No source/assets were changed
by cna-demos during runtime.

Previous COMPLETE checkpoint: 453 at 91fc3f9 (closure 9a95a11), tools/validate.sh exit 0, all 466
enabled CTests (197.82 s), SW 400/25/25/3, GL 421/26/3/3. Exact ORIGINAL 451-prefix comparison
adds only automatic_dispatch/vertex_colors, preserves every old 35/36 failure, status and count,
and accepts exactly the TLS nine-check extension after its eight original controls. Other C/
custom-content lists/traces, all 34 selected hashes/four assets and normalized stderr match.
Evidence/catalog/helpers: reports/checkpoint-453-*. Its inspected vertex-color captures differ
only 864 blue pixels by one level. All older C/vertex-color /tmp source drafts are stale.

Current applied C candidate: standalone/c_api.game.launch_parameters/main.c (113 lines), with
its metadata and CMakeLists. Fifteen intended child checks cover explicit bounded StringViews,
owned UTF-8 bytes after caller overwrite, guarded exact/short buffer copies, ordinal enumeration
(Zeta before lowercase names), documented first-wins Add, malformed replacement preserving prior
state, null/zero-length empty values and explicit empty-list clearing. Output lengths are seeded
with UINT64_MAX before short/exact/empty copies so unchanged outputs cannot pass accidentally.
The current C source passes GCC 14 C99 -Wall/-Wextra/-Werror syntax-only validation; runtime is
pending ISSUE-053 repair. /tmp/cna-launch-parameters* source drafts are older, APPLIED AND STALE.
Prepared /tmp/cna-run-456-focused.py, /tmp/cna-archive-456.py, /tmp/cna-456-finish.py and
/tmp/cna-capture-456.py expect 15 child checks, unchanged ORIGINAL 455 statuses/failures and
old C/custom-content lists/traces, plus 35 selected binaries/four assets. The helpers now name source commit 8f2b04c; update that if source changes, and capture runtime
hashes after the checkpoint build succeeds. /tmp/cna-catalog-456.json and source-only coverage are current metadata evidence.
Then continue the semantic member audit. Remaining skinned gaps include independent diffuse contributions from lights 1/2, vertex/texture-
alpha interactions, vertex-lighting specular and arbitrary joint scales. C gaps include automatic Draw dispatch, other callback failures and destruction
cancellation; this batch demonstrates automatic Update specifically. Canonical TLS now covers
nonempty owner details versus empty worker details, not all families or two distinct concurrent
nonempty parameters. Invalid Matrix FOV, CurveKeyCollection index and pure SoundEffect sample
helpers were rejected as ParamName fixtures by source inspection (std exceptions or local guards/
zero-return paths); do not infer canonical detail coverage from those routes. Do not restart
completed phases or add trivial getter/enum variants.

Earlier checkpoint narratives and subsystem source observations are preserved in
[docs/CHECKPOINT_HISTORY.md](docs/CHECKPOINT_HISTORY.md). Consult the relevant history when
returning to an older area; its historical next-task instructions do not replace this section.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

## Active 456 batch

Source 8f2b04c adds c_api.game.launch_parameters (113-line C source, 15 child checks). Both
focused SOFTWARE/OPENGLES3 runs pass with empty stderr; all 35 related C/CLI/launch CTests pass
in 20.53 s, including two existing WILL_FAIL cases. Evidence: reports/checkpoint-456-focused-*.
Catalog/metadata: 456 demos, zero problems, 293/329 types, 1370/2326 XNA members, 166/3210 C routes.
No loop/image is claimed for this map example. Its old /tmp source drafts are APPLIED AND STALE.

Full 456 attempt 10188 stopped at the manifest CTest (see section 14). Its selected hashes and
start-state observation of CNA 4985e26e7 are preserved under checkpoint-456-manifest-*.
Earlier attempts stopped during build: public-dialect and header-availability archives preserve
those failures. The corrected manifest focused checks and build now pass. Before starting the
next checkpoint, update archive/finish helpers to the new source commit; recapture 35 binary/four
asset hashes after that checkpoint's build, then compare both smokes to the original 455.

ISSUE-053 is resolved for the exercised build: initial final-build attempt failed in external
FrameworkDispatcher namespace lookup; the corrected retry passed (200 scheduled targets).
Preserve the genuine failed compiler log. IMPORTANT capture correction: the files named
checkpoint-456-failed-FrameworkDispatcher.cpp and -failed-state.json were captured AFTER the
external array had moved to a global anonymous namespace. They already contain the repair.
Earlier hash-equality blocker reports were wrong and are superseded by
checkpoint-456-evidence-correction.json. Repair is in external 7d8eba687; later f15d59446 and
ongoing ContentManager/GamerServices edits were observed. This is not an atomic input snapshot.
No CNA source edited/reverted by cna-demos. Original first-build evidence remains separately
archived; it does not validate the later output-sentinel refinement.

Next graphics batch, AFTER full 456 closes: two existing animation extensions and one new cube
transfer scene, catalog 457. All three drafts are UNAPPLIED and GCC 14 syntax-checked only:
- /tmp/cna-palette-player-spaces.cpp: palette_sampling 14->19 checks. Reuses the three-bone
  authored hierarchy through SkinningData/AnimationPlayer to distinguish local/world/skin points
  and compare both public evaluators. Skeleton/clip outlive the player. No new image.
- /tmp/cna-node-animation-contracts.cpp: node_animation 4->9 checks. Nonidentity untracked ancestry,
  target-space refusal before writes, and time clamping after deliberately disturbing the pose.
  Restore authored placement before drawing. Refresh its baseline capture from COMPLETE 456 first.
- /tmp/cna-cube-face-transfers.cpp: 155 lines, eight intended checks, destination
  demos/graphics/texture_cube/face_transfers.cpp. Independent Color faces, source/destination array
  windows, 2x2/1x1 mips, refused writes with deliberately different texels and untouched output
  guards. Atlas pixels are public read-back data rendered as Texture2D; no direct GPU-readback or
  cube-shader-sampling claim. Metadata order corrected; final syntax diagnostics empty.

Drafts and syntax helpers/logs are preserved under reports/checkpoint-456-next-*. Next runtime/
comparison helpers are /tmp/cna-{run-457-focused,archive-457,457-finish,capture-457}.py (syntax
checked, not run). They require ORIGINAL 456, exactly the two count extensions/new cube ID,
unchanged other outcomes/C/custom-content child lists, 35 binaries/four assets, and PENDING_457
replacement after the actual source commit. Inspect all four new node/cube captures; do not
skip focused validation or the complete 457 checkpoint merely because the drafts compile.

Earlier baseline animation runs used the first successful 456 build: palette 14/node 4 pass on
both renderers, four empty stderr streams. Both 800x480 node first-frame captures were inspected;
only two pixels differ across renderers (maximum channel difference 73), not RGBA-identical.
Three selected hashes remained unchanged. Evidence: checkpoint-456-before-* and
-intermediate-binaries.json. Refresh this baseline after current full 456 because CNA changed.

Further source-only TextureCube member audit is in COVERAGE_AUDIT.md / ROADMAP phase 5.
Future DDS diagnostic preparation: /tmp/cna-cube-dds-fixture.py generates the original 506-byte
/tmp/cna-cube_levels.dds with six RGB24 faces and three independent mip colors. JSON records
exact offsets/colors/hash; Python checks structure only. Archived as checkpoint-456-next-*.
Current TextureCube::DDSFromStreamEXT reads once using total Length; unlike the exercised 2D
route, short reads and nonzero starting positions need a bounded runtime probe. Compare normal,
offset-16, seven-byte short-read and truncated streams (with/without prefix); the final payload
byte is deliberately nonzero. No DDS native decoding/upload/visual evidence or new CNA issue yet.

456 full attempt session 87099 TERMINATED exit 1 during build (310/567 progress), before CTest:
external CNA promoted ShaderDialectEXT into public CNA/ShaderDialectEXT.hpp / namespace CNA.
Four shader/MRT demos still imported the former internal name. This is fixed locally by using
the actual public header/namespace in all four sources; no shader text or assertions changed.
Evidence: checkpoint-456-public-dialect-{build,validation}.log and post-failure observed state.
This is demo compatibility maintenance, not a new runtime CNA issue. Rebuild/focus these demos,
commit the source changes, update 456 helper sourceCommit, then restart full validation. The
previous focused launch checks remain valid for their recorded binaries; full 456 is NOT complete.

Independent shader member audit now records ShaderEffect constructors/packages, success versus
failure diagnostics, uniform families/arrays, sampler kinds, block layout and clone/lifetime
boundaries. Existing GLSL success does not establish the error-log branch merely because metadata
names GetCompileErrorEXT. ShaderEffect::Clone currently rebuilds source and copies labels/language;
matrix/uniform/texture-state expectations need a bounded runtime probe before any CNA bug claim.
No additional shader demo, metadata route or runtime credit was added by this source-only table.

Compatibility build 49553 and focused session 12553 both completed successfully. The public
ShaderDialectEXT import replaces all four former internal imports; shader text/assertions are
unchanged. SOFTWARE skip stderr is the exact old custom-effects diagnostic (present in original
455), not empty; the focused helper was corrected after inspecting that original evidence.
Both GL shader results and C launch checks match expected counts. No runtime logs suppressed.

Full validation restarted as live session 41710 at source 9c6a604; hash capture is pending its
build completion. Helpers were updated and the successful dialect-focused review is archived.

Additional full attempt 41710 stopped before runtime: four CNA content loader translation units
reported CNA/ShaderDialectEXT.hpp missing. Post-failure direct reading and GCC preprocessing
confirmed the header is available again. Raw build/validation logs, post-failure observation and
empty successful preprocessing log are archived as checkpoint-456-header-availability-*.
This is a transient external file-availability event with unknown cause, not an established CNA
runtime defect. No file was restored/edited by cna-demos. Full retry 10188 is now live.

Historical 10188 checkpoint update (superseded by its archived CTest failure): build exited 0 (81 scheduled targets), metadata 456/0, full CTest
running. /tmp/cna-capture-456.py captured 35 binaries/four assets after the successful build;
checkpoint-456-start-state.json observes CNA 4985e26e7 (public shader dialect promotion committed).
Do not overwrite these runtime hashes or rebuild during the remaining runtime phases.

Session 10188 has now exited 1 at CTest. The lazy reader registration contract is explicit in
ContentManager::GetXnbReaderUsageSummary documentation. The manifest fix will verify inventory
aggregation and the reader family required by the loaded font; it will not require unrelated
model/audio readers to be pre-registered or assume a clean registry in shared smoke processes.

Manifest correction completed: thirteen focused checks pass on each renderer, stderr empty;
no screenshot/visual review claimed. No audio/model warm-up or internal registration shortcut
was added. The ten existing snapshot/asset controls remain. Full checkpoint restart is next.

Full checkpoint restarted at e70eeb5 as session 65041; seven-target build and metadata pass,
CTest live. 35 binary/four asset hashes captured after this build. 455 is still last complete.

65041 checkpoint update: all 469 enabled CTests pass; SOFTWARE smoke is running. No runtime
checkpoint closure yet. Two additional UNAPPLIED source-only drafts are retained for AFTER 457:
* /tmp/cna-cube-dds-streams.cpp (five intended checks): original in-source RGB24 six-face/three-mip
  bytes, all-level Color readback, current-position control, seven-byte Read override, and identical
  one-byte truncation at offsets zero/sixteen. Final byte is nonzero. Both stream defects remain
  source suspicions until actual native execution; no CNA issue or knownIssue marker exists.
* /tmp/cna-portable-package.cpp (seven intended checks): two owned GLSL language pairs, stable
  selection under reordered variants, incomplete/ambiguous refusal, package-created ShaderEffect
  language/validity, and stock versus channel-cycled sprite pixels. No invalid compiler source,
  SPIR-V/DXIL/WGSL or desktop-runtime claim. Potential renderer support must be exercised.
Both pass g++ -fsyntax-only with the existing compile_commands flags; logs empty. They are not
in the project source/catalog and have no native link/runtime/visual credit. Archived with hashes
as reports/checkpoint-456-next-* and checkpoint-456-next-stream-package-review.json.

65041 checkpoint update: SOFTWARE completed 403/25/25/3; original 455 comparison passes.
All 35 failed assertions, old statuses/known issues and old C/custom-content child lists/traces
match exactly. Only the new launch-parameter ID and planned manifest 26 -> 13 count differ.
SOFTWARE evidence/review is archived. OPENGLES3 remains live; do not run finish or rebuild yet.


## Complete 457 and its preparation (ab2a92b)

Full tools/validate.sh exited 0, 470 enabled CTests (393.71 s), SW 404/25/25/3 and GL 425/26/3/3.
Exact original456 comparison adds cube eight checks and palette/node extensions 14->19/4->9;
all 35/36 old failed assertions, statuses/known issues and C/custom child lists/traces remain.
Selected 35 binaries/four assets and normalized stderr are unchanged through runtime.
Four new captures actually inspected: node unchanged per renderer against its refreshed456
baseline (pre-existing cross-renderer two pixels, max delta73), cube pair RGBA-identical.
The following earlier continuation notes are historical; HANDOFF.md supersedes their live state.

## 14. Next task

**Immediate next action:** resume COMPLETE 457 validation session 33564 at source ab2a92b,
/tmp/cna-demos-457-validation.log. Its seven-target build and metadata 457/0 passed; all 470 enabled CTests passed (393.71 s),
SOFTWARE smoke is complete and reviewed; OPENGLES3 is running. Selected 35 binary/four asset hashes captured after this build; helpers now name
ab2a92b. Do not edit/rebuild sources/assets while runtime runs. Archive each completed smoke
with /tmp/cna-archive-457.py, then /tmp/cna-457-finish.py. Original complete 456 remains baseline.
Focused evidence: checkpoint-457-focused-*; all six runs passed with empty stderr and all four
new captures were actually inspected. Node matches original456 separately on each renderer;
its existing cross-renderer difference is two pixels/max delta 73. Cube captures are identical.
The following drafts are APPLIED AND STALE; do not copy them over further edits:

* /tmp/cna-palette-player-spaces.cpp -> demos/graphics/model/palette_sampling.cpp: 14 -> 19
  checks comparing local/world/skin transforms through AnimationPlayer, with the existing
  three-bone SkinnedModelEXT fixture and noncommuting scale/rotation/inverse binds.
* /tmp/cna-node-animation-contracts.cpp -> demos/graphics/model/node_animation.cpp: 4 -> 9
  checks for nonidentity untracked state, wrong-space refusal with all transforms preserved,
  and disturbed-pose negative/past-end clamps. Original scene placement is restored.
* /tmp/cna-cube-face-transfers.cpp -> demos/graphics/texture_cube/face_transfers.cpp: new eight-
  check Color face/rectangle/mip workflow, guarded array windows, refused transfers and a
  displayed 2D atlas of public readback. It does not prove GPU readback or cube shader sampling.

Use /tmp/cna-capture-457.py after the full checkpoint build, then /tmp/cna-archive-457.py for
each completed renderer and /tmp/cna-457-finish.py. Helpers expect cube 8 checks, palette 14 -> 19,
node 4 -> 9, otherwise original456 outcomes/counts/failures/traces (title timing variance only).
Catalog /tmp/cna-catalog-457.json is current. Expected totals remain projections until full runs:
470 enabled CTests, SW 404/25/25/3, GL 425/26/3/3. Do not edit/rebuild during runtime phases.

Then continue with the deeper TextureCube/ShaderEffect audit in docs/COVERAGE_AUDIT.md:
* /tmp/cna-cube-dds-streams.cpp: five intended checks with original RGB24 six-face/three-mip
  in-source data. Full/current-position controls, seven-byte Read chunks, and identical one-byte
  truncations at offsets zero/sixteen. Short-read/length issues are source suspicions only;
  exercise before filing any CNA issue. /tmp/cna-cube-dds-fixture.py is an independent Python
  structural fixture, not CNA runtime evidence.
* /tmp/cna-portable-package.cpp: eight intended checks for owned ES/desktop stage pairs,
  declaration-order independence, incomplete/ambiguous selection, owned refusal diagnostics, selected-language retention
  and stock/channel-cycle sprite pixels. No binary shader or compiler-error-path claim.
Both new drafts pass syntax only, with empty logs; neither is in the project catalog.

Remaining C work includes automatic Draw dispatch, other callback failures and destruction
cancellation. Existing automatic_dispatch establishes Update only. Remaining skinned work
includes independent diffuse lights 1/2, alpha interactions, vertex specular and joint scales.
Do not restart completed phases or stop at this batch. See docs/CHECKPOINT_HISTORY.md for
older checkpoints and preparation failures, including the non-atomic ISSUE-053 capture correction.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

Source-only shader contract clarification during 457 runtime: ShaderEffect's explicit code/package
constructors throw ShaderCompilationExceptionEXT for unusable package selection, but delegate
usable payload compilation to the legacy constructor; invalid syntax can still return an invalid
effect. The portable-package draft now has eight intended checks, including copying labeled
stage diagnostics from the selection-refusal exception and inspecting them after its destruction.
This is still outside the catalog and has no native runtime credit; bad-syntax/compiler-log work
remains separate. Do not infer exception behavior solely from the exception's name.

Further UNAPPLIED C preparation: /tmp/cna-dispatch-destroy-cancellation.c extends the existing
foreign_thread_dispatch source from 40 to 46 intended checks, retaining all original labels and
two ISSUE-050 failures. It re-enables dispatch at the final boundary, starts a permitted setter,
never pumps it, destroys on the owner, then bounds release before joining and requires the worker's
INVALID_STATE/STATE record plus stale-handle/output preservation. A delayed post-destruction
call cannot pass the actual-cancellation assertion. C11 -Wall -Wextra -Werror syntax passes,
empty log; no native link/runtime credit. Latest C and eight-check package drafts are archived
as checkpoint-457-next-* (the earlier checkpoint-456 package draft has only seven checks).
After the complete 457 checkpoint, prioritize the DDS stream and package runtime workflows;
this C extension is an independent productive follow-up. Update metadata/description and exact
child-check comparison only after the source is applied and runtime results are known.
Automatic Draw remains separate: ordinary alternating frames alone cannot establish which
step served a request; do not add a timing-only test and claim isolated Draw service.

457 SOFTWARE smoke completed 404/25/25/3; exact ORIGINAL 456 comparison passed. The new cube
has eight checks, palette changes 14 -> 19 and node 4 -> 9. Every old status/known issue, all
35 failed assertions and all C/custom-content child lists/traces match. SOFTWARE evidence is
archived/reviewed. Session 33564 remains live in OPENGLES3; finish and rebuild must wait.


## Complete 459 (5ed8055) and preparation record

Full validation exited 0: 472 enabled CTests (209.38 s), SW 404/26/26/3, GL 426/27/3/3.
Original457 comparison preserves all old evidence after six explicit new passing C labels;
DDS adds exactly two ISSUE-054 failures (35/36 ->37/38), package eight GL checks/SW skip.
Selected35 binaries/four assets remain stable; normalized stderr adds only the one declared
SOFTWARE CustomEffects skip diagnostic. GL package capture inspected and final capture identical.
The following older continuation statements are historical; HANDOFF.md supersedes live states.

## 14. Next task

**Immediate next action:** resume complete 459 validation session 65747 at source 5ed8055,
/tmp/cna-demos-459-validation.log. Its seven-target build and metadata 459/0 passed; CTest
passed all 472 enabled tests (209.38 s); SOFTWARE is complete/reviewed and OPENGLES3 is running. Runtime capture has 35 binaries/four assets; helpers name 5ed8055. Do not edit or
rebuild sources/assets during runtime. Archive each completed renderer with
/tmp/cna-archive-459.py, then /tmp/cna-459-finish.py; original COMPLETE 457 is the baseline.
Catalog /tmp/cna-catalog-459.json is current. Source /tmp drafts are APPLIED AND STALE.

Focused review /tmp/cna-review-459-focused.py passes: DDS has exactly two ISSUE-054 failures,
package has eight GL checks or its declared SOFTWARE skip, and C adds exactly six passing labels
to its original forty (44 pass/2 old failures total). All old C labels/order remain after removal
of the six named additions. C trace: Destroyed property: pending=1, result=3, category=3.
No new C route/ID. Final GL image is identical to the initial capture actually inspected.
The DDS catch correction was a demo bug: catch actual System::FormatException, not
std::runtime_error. Genuine ISSUE-054 follows separate five-check runs with complete/prefixed
controls, one seven-byte Read call and acceptance of corrupted truncated bytes at offset16.
No CNA source was edited, and no reader workaround was added.

Complete helpers compare to ORIGINAL 457 and permit only the new IDs, exact two DDS failures
(35/36 -> 37/38), six specified passing C labels, and positive title timing variance. The generic
comparator must request review (exit1) for the new failing ID/assertions; the wrapper explicitly
verifies that exact delta. Stderr may add only one exact SOFTWARE CustomEffects skip diagnostic
for the new package; all other normalized diagnostics must match. Expected, still unverified
full totals: 472 enabled CTests, SW 404/26/26/3, GL 426/27/3/3. Do not edit/rebuild during runtime.

After full 459, continue the semantic member audit. ShaderEffect::Clone deserves a bounded
public matrix/owned-program behavior probe against Effect::Clone's contract; existing source
recompiles shader strings and copies labels/language but appears not to preserve matrix state.
Do not file a bug from source alone or assume raw uniform setters have the same clone contract.
Invalid-syntax compiler diagnostics remain separate: unusable package selection throws
ShaderCompilationExceptionEXT, whereas usable payloads delegate to the legacy string compiler
and can return an invalid effect for bad syntax. Automatic Draw service is also still separate:
ordinary alternating frames cannot prove which step served a worker. Remaining cube gaps include
typed/compressed transfer paths, copy/shared-resource lifetime and actual sampler behavior;
remaining skinned gaps include independent diffuse lights1/2, alpha, vertex specular and joint scales.
No completed phase should be restarted, and no batch completion is a stopping condition.

## 15. Assumptions the next session must know

* Current workspace: `/rv/data/development/github.com/libcna/cna-demos`; CNA and the other
  dependencies are siblings (`../cna`, `../sharp-runtime`, `../easy-gl`, `../meta-gl`).
* Current verified toolchain: GCC 14.2 and CMake 3.31.6. Reuse `build/linux-full`; do not create
  temporary build trees. Set `CCACHE_DIR=/rv/cnaccache` and `CCACHE_BASEDIR=/rv` for every build.
* External work may change the sibling CNA checkout. Do not edit/revert it from this task;
  record the observed state and exercised binary hashes, and keep runtime checkpoints immutable.
* Running windowed renderers needs `xvfb-run`; audio needs a device or `SDL_AUDIO_DRIVER=dummy`.

Next UNAPPLIED draft: /tmp/cna-shader-clone-state.cpp, archived as checkpoint-459-next-*.
Nine intended checks need at least two frames (one-frame capture only executes eight): explicit
source/language identity, nonidentity W/V/P retention, independent World mutation with observed
clone state restored, equal-viewport pixel comparison, original program disposal, disposed-clone
refusal, and unchanged clone pixels next frame. Public GraphicsResource::Dispose is used through
a base reference because the protected bool overload hides ShaderEffect's parameterless name.
The live device outlives all effects; inspected Effect::Dispose retains device_, so the proposed
refusal call does not dereference a destroyed device. Corrected Viewport getters; syntax now
passes with empty log. No native/runtime/image credit or new issue yet. Examine the actual
clone contract carefully before classifying state-copy behavior; raw uniform/texture copying
is explicitly outside this draft's claim.

459 helper clarification: --quiet suppresses in-process GameContext.Print. DDS fixture/read-call/
accepted-truncation trace is checked in focused logs; full comparisons check five JSON checks and
exactly two raw failures. Ordinary C printf/check traces remain enforced in both full logs.

459 SOFTWARE completed 404/26/26/3 and passed the exact wrapper review against ORIGINAL 457:
all old outcomes/known issues/counts and child lists/traces remain after the six specified new C
passes; raw failed assertions are exactly 35 -> 37 (the two ISSUE-054 checks). The general
comparator correctly requested review, which is recorded explicitly. OPENGLES3 remains live
in session65747; do not finish/rebuild yet. SOFTWARE evidence is archived under checkpoint-459-*.

## Shader preparation after full459 (before combined461)


**460 focused reproduction complete; full validation is next.** Retry build2864 passed after
230 scheduled targets; previous build93935 terminated with exit 143 after511/602, without a
compiler diagnostic. Both logs are archived. Metadata460/0. Manifest13 passes on both renderers.
Clone frame1 has8 checks, frame2 has9: exactly three GL failures establish ISSUE-055, six positive
controls pass including surviving-clone pixels. Both images actually inspected; GL stderr empty.
SOFTWARE skips CustomEffects. Initial unmarked evidence/source/CNA observations are archived
as checkpoint-460-initial-*, -unmarked-clone_state.cpp, -observed-* and -focused-observation.json.
Marker applied; build1425 passed554 scheduled targets, /tmp/cna-460-marker-build.log.
Incremental build96026 is now live at /tmp/cna-461-build.log. Further external public platform/
render-target changes expanded it to602 targets; CNA2a5100d72 observed with uncommitted edits. To amortize that rebuild, the syntax-checked diagnostics
source is now APPLIED as graphics.shader_effect.compiler_diagnostics (its /tmp draft is stale).
Build96026 includes the new glob/object. After it finishes, run each
new demo focused, inspect diagnostic image/logs, update docs/coverage and commit. The combined
complete checkpoint will be461 against ORIGINAL COMPLETE459; there is no complete460 claim.

Next full461 checkpoint must compare ORIGINAL COMPLETE459, preserving exactly the old37/38 raw
failures and every C/custom-content child list/trace. GL adds exactly the three ISSUE-055 labels;
SOFTWARE adds two declared CustomEffects skip diagnostics. The compiler demo adds deliberate
GL compiler text: inspect and archive its exact focused output before approving that delta.
Reuse build/linux-full, shared ccache and -j4; no source/assets/rebuilds during live full runtime.
Continue after this checkpoint.

Further meaningful gaps: bad-syntax shader diagnostics (selection refusal already shown),
typed/compressed cube transfers, copy/shared-resource lifetime and actual cube sampler behavior;
automatic Draw dispatch needs proof of the servicing step, not ordinary alternating-frame
completion. Skinned gaps include independent diffuse lights1/2, alpha, vertex specular and
joint scales. Continue after each batch; completed phases are the baseline, not a restart list.

## Complete459 before shader state/diagnostics

**Complete 459 checkpoint passed at 5ed8055.** tools/validate.sh exited 0; all 472 enabled
CTests passed (209.38 s), both complete isolated-X11 smokes and coverage regeneration finished.
Metadata 459/0; coverage 293/329 types, 1372/2326 XNA members, 166/3210 C routes.
Original complete 457 comparison adds only the DDS/package IDs and six passing C destruction
controls. Every old status/known issue/count and C/custom-content child list/trace remains
(after removing those six explicitly named C additions). Raw failures change exactly 35/36 ->
37/38: short DDS reads and prefixed truncation, ISSUE-054. The generic comparator's requested
review is explicitly resolved by exact checks; raw evidence is unmodified. No title count change.
All 35 selected binaries/four assets stayed unchanged through runtime. Normalized GL stderr is
identical; SOFTWARE adds exactly one declared CustomEffects skip diagnostic for portable_package.
CNA f130abff4 observed at runtime capture; final source state archived separately. This is not
an atomic build-input or full transitive dependency snapshot.

| Check | Result |
|---|---|
| Existing linux-full build | Passed; seven scheduled checkpoint targets, no compiler diagnostics |
| Metadata/assets --validate | 459 demos, 0 problems |
| tools/validate.sh | Passed, both complete smoke runs and coverage regeneration |
| CTest | 446 passed (including 26 WILL_FAIL), 26 skipped, 3 manual disabled; 0 failures |
| SOFTWARE smoke | 404 passed, 26 expected failures, 26 skipped, 3 manual not run |
| OPENGLES3 smoke | 426 passed, 27 expected failures, 3 skipped, 3 manual not run |
| New behavior | Package 8 GL checks/SW skip; DDS 3 passes and exactly 2 ISSUE-054 failures; C 44 passes and the 2 old ISSUE-050 failures |
| Artifacts/diagnostics | Selected hashes stable; only the one declared SOFTWARE skip diagnostic added |

The labeled package GL capture was actually inspected; its final capture is RGBA-identical.
No DDS or C screenshot claim. The initial DDS catch bug (std::runtime_error versus actual
System::FormatException) was fixed before the five-check CNA reproduction and remains archived.
Complete reports/catalog/helpers: build/linux-full/reports/checkpoint-459-*.
Previous complete 457 at ab2a92b (closure 768797e) has 470 enabled CTests and SW 404/25/25/3,
GL 425/26/3/3. Historical preparation details are in docs/CHECKPOINT_HISTORY.md.

## Complete461 before retained-cube and automatic-Draw coverage

**Complete 461 checkpoint passed at a447ad2.** tools/validate.sh exited 0; all 474 enabled
CTests passed (234.89s), both full isolated-X11 smokes and coverage regeneration completed.
Metadata 461/0; coverage 293/329 types,1372/2326 XNA members, 166/3210 C routes. This is semantic
shader progress without implying additional public types or comprehensive member coverage.
Original complete 459 comparison adds only clone_state/compiler_diagnostics. Every older
status/known issue/count, raw failure and C/custom-content child list/trace is retained.
SOFTWARE raw failures stay37; GL38 ->41 adds exactly the three ISSUE-055 clone assertions.
The generic GL comparator's requested review is resolved explicitly by these exact checks.
No title count variance occurred. All 35 selected binaries/four assets remained unchanged through
runtime. Normalized stderr adds only two SOFTWARE CustomEffects skips and exactly the focused
GL compiler diagnostic (including its blank continuation line). No diagnostics were suppressed.
CNA 2a5100d72 observed at runtime capture; final external state archived separately. No atomic
build-input or full transitive dependency snapshot is claimed.

| Check | Result |
|---|---|
| Existing linux-full checkpoint build | Passed420 scheduled targets; no compiler diagnostics |
| Metadata/assets --validate | 461 demos, 0 problems |
| tools/validate.sh | Passed, both complete smokes and coverage regeneration |
| CTest | 446 passed (including 26 WILL_FAIL), 28 skipped, 3 manual disabled; 0 failures |
| SOFTWARE smoke | 404 passed, 26 expected failures, 28 skipped, 3 manual not run |
| OPENGLES3 smoke | 427 passed, 28 expected failures,3 skipped, 3 manual not run |
| New behavior | Compiler 7 GL checks pass; clone 6 pass/exactly 3 ISSUE-055 failures; two SOFTWARE capability skips |
| Artifacts/diagnostics | Selected hashes stable; exact declared skip/compiler stderr additions only |

Both initial clone images and the diagnostic cyan-card image were actually inspected; the
marked focused clone frame2 is RGBA-identical to its initial capture. No full-smoke screenshot
or physical GPU claim. The manifest retains 13 checks on both renderers; wording acknowledges
query-triggered built-in registration without asserting a lazy transition.
Complete reports/catalog/helpers: build/linux-full/reports/checkpoint-461-*; initial unmarked
clone evidence remains checkpoint-460-*. Previous complete 459 at 5ed8055 (closure 19bd3dd) has
472 enabled CTests, SOFTWARE404/26/26/3, GL426/27/3/3 and37/38 raw failed assertions.

## Complete 462 checkpoint (superseded by 464)

**Complete 462 checkpoint passed at bbddd34.** tools/validate.sh exited 0; all 475 enabled
CTests passed (228.67 s), both full isolated-X11 smokes and coverage regeneration completed.
Metadata 462/0; coverage 293/329 types, 1372/2326 XNA members, 166/3210 C routes.
Original complete 461 comparison adds only sampler_ownership and six passing automatic-Draw
child controls. Every older status/known issue/raw failure and C/custom-content child list/trace
is retained after filtering those six specifically named additions. Raw failures remain 37/41.
Only GL game.window.title checks vary 10 -> 11 (unchanged per-Update checks under a frame limit).
All 35 selected binaries/four assets stayed unchanged through runtime. Normalized stderr adds
exactly one SOFTWARE CustomEffects skip; GL is identical, including the existing deliberate
compiler diagnostic and 84 Xprotocol lines. No diagnostics suppressed. CNA 2a5100d72 observed
at runtime capture; final external HEAD90bd2d33c/state archived separately, not an atomic
input snapshot.

| Check | Result |
|---|---|
| Existing linux-full checkpoint build | Passed seven scheduled targets; no compiler diagnostics |
| Metadata/assets --validate | 462 demos, 0 problems |
| tools/validate.sh | Passed, both complete smokes and coverage regeneration |
| CTest | 446 passed (including 26 WILL_FAIL), 29 skipped, 3 manual disabled; 0 failures |
| SOFTWARE smoke | 404 passed, 26 expected failures, 29 skipped, 3 manual not run |
| OPENGLES3 smoke | 428 passed, 28 expected failures, 3 skipped, 3 manual not run |
| New behavior | Cube 5 GL checks/SW skip; C automatic_dispatch 21 child checks on both selections |
| Artifacts/diagnostics | Selected hashes stable; only the one declared SOFTWARE skip diagnostic added |

The focused cube GL image was actually inspected: six solid faces survive source disposal,
including the shared orange +Z edit. No in-face orientation/mip filtering/direct GPU-readback
claim. The C extension preserves all 15 Update controls and adds six first-Draw checks; exact
trace pending=1/before=333333/observed=500000/updates_between=0/result=0. No C image, Draw-served
device borrow or latency guarantee. Complete reports/catalog/helpers: checkpoint-462-*.
Previous complete 461 at a447ad2 (closure 4f9fe87) has 474 enabled CTests, SW 404/26/28/3,
GL 427/28/3/3 and the same 37/41 failed assertions; full history is CHECKPOINT_HISTORY.md.

## Complete 464 checkpoint (superseded by 466-timing)

**Complete 464 checkpoint verified at 7bcb39f.** The original tools/validate.sh invocation
passed its seven-target build, metadata and all 477 enabled CTests (241.42s), then ended with
SIGTERM/exit 143 during SOFTWARE smoke before JSON publication. The signal origin is unknown;
no remaining demo process was found. Its partial evidence remains checkpoint-464-interrupted-*.
Both full smoke phases and coverage were resumed without rebuilding in /tmp/cna-resume-464.sh;
that runner exited 0, and exact comparisons plus /tmp/cna-464-finish.py passed. This is completed
validation with resumed runtime phases, not a claim that the original script exited zero.

Metadata 464/0; coverage 293/329 types,1372/2326 XNA members, 166/3210 C routes. Original complete 462
comparison adds only compressed_blocks and uniform_palettes. Every old status/known issue,
raw failure and C/custom-content child list/trace is retained exactly. Failures remain 37/41.
Only GL game.window.title checks vary 11 ->10 (unchanged per-Update checks under a frame limit).
All 35 selected binaries/four assets stayed unchanged, including across the interruption.
Normalized stderr adds exactly one SOFTWARE CustomEffects skip; GL is identical, including
the deliberate compiler diagnostic and 84 Xprotocol lines. No diagnostics suppressed.
CNA 90bd2d33c was observed at runtime capture and final inspection, with external documentation
edits; this is not an atomic source/input snapshot.

| Check | Result |
|---|---|
| Existing linux-full checkpoint build | Passed seven scheduled targets; no compiler diagnostics |
| Metadata/assets --validate | 464 demos, 0 problems |
| Full validation phases | Build/CTest passed; interrupted smoke attempt retained; both resumed complete smokes and coverage passed |
| CTest | 447 passed (including 26 WILL_FAIL), 30 skipped, 3 manual disabled; 0 failures |
| SOFTWARE smoke | 405 passed, 26 expected failures, 30 skipped, 3 manual not run |
| OPENGLES3 smoke | 430 passed, 28 expected failures, 3 skipped, 3 manual not run |
| New behavior | Compressed cube 8 checks on both; uniform palettes 4 GL checks/SW skip |
| Artifacts/diagnostics | Selected hashes stable; only the declared SOFTWARE skip diagnostic added |

The focused palette GL image was actually inspected: both matrix translations and vec3 colors
must update for the lower viewport's changed arrangement. Compressed cube checks are public
byte transfers, not sampling/decoded-image/direct GPU-readback evidence. No new CNA issue.
Complete reports/catalog/helpers: checkpoint-464-*. Previous complete 462 at bbddd34 (closure
84a19c9) has 475 enabled CTests, SW 404/26/29/3, GL 428/28/3/3 and the same 37/41 failures.


## Complete 466-timing and rejected initial466 (2026-10-12)

Source6f4feed adds typed_cube8checks both renderers, sampler_coordinates6GL/SW capability skip,
and eight C sprite cancellation checks (19 without capture/21 with). Focused images inspected;
GL C before/after identical, SOFTWARE animation differs with exact stable center/corner controls.
Initial full466 builds7 targets, validates466/0 and passes479 enabled CTests in247.81s. Both
smoke JSONs and coverage finish, but outer wrapper exits143 (origin unknown). Status totals
SW406/26/31/3 and GL432/28/3/3 conceal a SOFTWARE replacement of ISSUE-030 by the aggregate
20-second deadline before Evaluate. Exact raw review rejects that baseline. GL retains all41
old raw failures; selected35 binaries/four assets stable. Original evidence:466-initial-*.

Unchanged isolated original and repaired focused net runs retain all five host checks with
four passes and only ISSUE-030. Source5e7c950 resets a20-second watchdog only at finite
startup/burst/recovery transitions, including sign-in in the watchdog; packet counts, pacing,
settling, recovery and all evaluation predicates stay unchanged. Full466-timing is complete:
build7, metadata466/0,479 enabled CTests237.52s (448passed incl26WILL_FAIL,31skipped,3disabled),
SW406/26/31/3,GL432/28/3/3. Both inner tools/validate.sh and outer Xvfb exit0 explicitly logged.
Exact originalcomplete464 comparison retains all37/41 failed assertions, old statuses/counts,
all five net checks and every C/custom child list except the eight named C sprite additions.
Only the two declared new IDs are added.35binary/fourasset hashes stable. Normalized stderr
adds only one SOFTWARE CustomEffects skip; GL unchanged including intentional compiler error
and84Xprotocol lines. Separate outerXvfb log retains31amdgpu diagnostic pairs; no hardware claim.
CNA0a2918c1d observed clean at capture/final, not an atomic input snapshot. Coverage293/329types,
1373/2326XNA,166/3210C. All capture/archive/finish helper reviews pass.

Controlled native regression uses /tmp binaries from original6f4feed/repaired5e7c950 and the
same existing compile/link configuration, identical8-second delays before three host phases,
own child basenames and absolute existing asset root. Old22.884857839s: sender deadline then
host partial evaluation yields four failures (deadline, sender incomplete, no delayed packets,
ISSUE-030). Fixed37.413048156s: all five original host checks, four pass and only ISSUE-030.
Both host/child SOFTWARE selections explicit; stderr INFO retained without WARN/ERROR.
Selected35 shipped binaries remain unchanged; no new build tree, stalled-peer injection or
screenshot validation. Probe-induced partial failures are not new CNA issues. All evidence
and helpers:checkpoint-466-timing-delayed-*. The original398/401 validation debt is long closed.


## Complete 466-transfer (2026-10-12)

Source59440706d3127a7f9f02422a95bb9c1511918f7d extends volume_data3->10checks and C content
39->47childchecks without capture (41->49with), preserving all original controls. All eight
before/after images inspected, four pairs RGBA-identical, focused stderr empty. Initial build
615scheduled/612completed passes but external Game.hpp changes its capability value to
std::optional at16:55:34.496679375, just after DemoGame.o16:55:33.956675335. The volume object
compiles later17:05:11.668997158; SIGSEGV occurs at its ORIGINAL first Check with context pointing
at GraphicsDeviceManager's vtable. An unchanged clear control passes but has no checks. The
64scheduled/61completed corrective rebuild resolves the mixed layout without demo changes.
All original crash/backtrace/object hashes remain checkpoint-466-transfer-initial-*.

Full build32scheduled/29completed, metadata466/0,479enabled CTests281.68s (448passed including
26WILL_FAIL,31skipped,3disabled), SOFTWARE406/26/31/3 and GL432/28/3/3. Exact originalcomplete
466-timing comparison retains37/41raw failures, all five network checks, all C/custom lists
and traces except8named content additions; only volume JSON3->10changes.35selected binaries
and4assets stable. SW normalized stderr unchanged; GL retains all application diagnostics
including intentional shader compile failure but known X protocol lines fall84->42. Initial
strict diagnostic review rejects this; an explicit four-record removal counter15/15/6/6 then
records the finite variation. Cause not isolated, no repair/suppression claim. Separate outer
Xvfb log has7amdgpu diagnostic pairs rather than31; inner/outer exit journals both0. Capture,
archive and final helpers all pass; CNA59598bfac clean at capture/final, not an atomic snapshot.
Coverage293/329types,1373/2326XNA,168/3210C. Archives checkpoint-466-transfer-*.

Subsequent /tmp-only EOF probes leave35project hashes unchanged. Direct FileStream reads4
nonzero bytes with guards, reports-1after short/empty reads instead of4, and rewinds/rereads
correctly. C storage draft has30passes/one failure (SUCCESS/-1instead of24), preserving all23
original checks plus8named additions on both requested renderer environments. Empty stderr;
no Game/rendering/visual claim. Root is the sharp-runtime FileStream EOF state/position path,
exposed by the C adapter (ISSUE-056). Sources/helpers/review:checkpoint-466-transfer-eof-*.


## Complete467 (2026-10-12)

Sourcec45349fb38fea2295aad924a8cfe9f316069ae9e adds sample_kernel5GLchecks/SW CustomEffects
skip and eight C save_game controls (23->31children;30pass/one ISSUE-056). Actual800x480GL
image inspected, all27sample centers/neighbors/outer pixels exact; focused GL stderr empty.
C original23child list retained, only EOF position fails (SUCCESS/-1instead of24), no Game or
visual claim. Direct/C probes in466-transfer-eof isolate underlying sharp-runtime behavior.
Focused build36scheduled/33completed, metadata467/0,24targeted CTests5.55s (16pass/8skip).

Full build7scheduled operations, metadata467/0,480enabled CTests217.01s (448passed including
27WILL_FAIL,32skipped,3manualdisabled), SW405/27/32/3,GL432/29/3/3. Inner/outer exits0 recorded
separately. Exact originalcomplete466-transfer comparison retains37/41old failures and adds
only EOF056 (38/42total); only storage changes old status/knownIssue. Old23storage child lines
plus8named additions, all other C/custom lists/traces and all5net checks exact. Only other JSON
count change: positive per-Update GL title10->11.35selected binaries/fourassets stable.
Normalizedstderr+one SOFTWARECustomEffects skip; GL unchanged incl42Xprotocol records and
intentional shader compiler error. OuterXvfb retains14amdgpu diagnostic pairs, no hardware
claim. CNA714f9f76a clean observed at capture/final, not an atomic snapshot. Coverage293/329,
1373/2326XNA,170/3210C. All capture/archive/finish helpers pass; checkpoint-467-* immutable.


## Complete467-scene and superseded predecessor notes

Complete checkpoint **467-scene**, source `11dbd5b`, preserves all 467 IDs. Build: seven
scheduled operations; metadata 467/0; all 480 enabled CTests pass in 211.34 s (448 passed,
including 27 WILL_FAIL, 32 capability skips; three additional manual tests disabled).
SOFTWARE: 405 passed / 27 failed / 32 skipped / 3 not run. OPENGLES3: 432 / 29 / 3 / 3.
Both inner validation and outer Xvfb independently exit zero. All 38/42 original failed
assertions, C/custom child lists and traces, and five network evaluation checks are retained,
allowing precisely seven new positive storage permission checks (31 -> 38 with only ISSUE-056).
Palette GL checks increase 4 -> 6; SOFTWARE keeps its declared skip. The only other JSON count
change is GL window.title 11 -> 10, the reviewed positive per-Update variation.
All 35 selected binary and four asset hashes remain unchanged. Normalized application stderr
is identical, including 42 known GL X protocol records and the deliberate shader compiler
error. Separate outer Xvfb diagnostics contain 13 exact amdgpu pairs (previously 14); no cause
or hardware GPU claim. CNA 714f9f76a was observed clean at capture/final, not an atomic input
snapshot. All capture/archive/finish helpers pass. Immutable evidence: checkpoint-467-scene-*.
Coverage now records 293/329 types, 1373/2326 XNA members and 174/3210 C routes. These counts
do not establish comprehensive semantic coverage.

The following earlier handoff sections are preserved as historical evidence, not next tasks:

## 7. Build and test status

Focused467-scene is also verified (see next task): palette6GLchecks and C38childchecks with
only original ISSUE-056, inspected images,24targeted CTests. Current metadata names174C routes;
full467-scene is pending. No new catalog ID.

**Complete 467 at source c45349f is verified.** tools/validate.sh and outer Xvfb both return 0,
independently journaled; session 41309 is CLOSED. Build: 7 scheduled operations, metadata 467/0,
all 480 enabled CTests in 217.01 s (448 passed incl27 WILL_FAIL,32 skipped,3 manual disabled).
SOFTWARE405/27/32/3, OPENGLES3 432/29/3/3. Generated coverage now uses these complete results:
293/329types, 1373/2326XNA members, 170/3210 C routes; comprehensive coverage is not claimed.

Exact originalcomplete466-transfer comparison retains all 37/41 prior failed assertions and
adds only ISSUE-056 (38/42 total). Only C save_game changes status/knownIssue; its23 old child
checks remain plus8 named additions (31 total, 30 pass/1 failure). All other C/custom lists/traces
and five network evaluation checks are exact. New kernel5GL/SWskip matches focused evidence.
Only other JSON count change is GL game.window.title10->11, its positive per-Update variation.
All35 selected binary/four asset hashes remain stable. Normalized stderr adds exactly one
SOFTWARE CustomEffects skip; GL is unchanged, including42known X protocol records and the
intentional shader compile error. Separate outerXvfb diagnostics retain14 amdgpu pairs; no
hardware GPU claim. CNA 714f9f76a observed clean at capture/final, not an atomic input snapshot.
All capture/archive/finish helpers pass; immutable evidence: reports/checkpoint-467-*.

Focused build36scheduled/33completed, metadata 467/0 and all24targeted CTests pass (5.55s,
16passed/8capability skips). The actual800x480kernel image was inspected and all27sample pixels
are exact. C EOF behavior is independently reproduced through direct FileStream and C API:
position-1after short/empty EOF reads despite valid data/guards/rewind. All35shipped binaries
remain unchanged through those probes. Proof:checkpoint-466-transfer-eof-*, ISSUE-056.
Previous complete466-transfer source5944070/closureaf64eff, its mixed-layout build correction
and explicitly reviewed GL diagnostic reduction remain in CHECKPOINT_HISTORY.md.

Current complete helpers /tmp/cna-capture-467.py, /tmp/cna-archive-467.py
SOFTWARE/OPENGLES3 and /tmp/cna-467-finish.py all passed. Current diagnostic differences are
only one declared SOFTWARE skip; the previous finite GL reduction belongs to466-transfer.
All runtime hashes/exits/reports are immutable. The volume transfer and game-content-borrow
/tmp predecessors are APPLIED AND STALE. Do not reapply or restart old466-timing work.

APPLIED shader predecessor /tmp/cna-sample-kernel.cpp (STALE): one bright texel, original kernel, horizontal
1:2:1 spread and vertical1:6:1 spread using the same shader. Actual SetUniformVec2Array offsets,
SetUniformFloatArray weights and SetUniformVec2 texel size remain visible. Caller arrays are
overwritten before drawing; expected centers/neighbors/off-axis/outer pixels distinguish tap
contributions. Five intended checks. Native GL5checks and inspected image now pass; SW declares its capability skip.

APPLIED C predecessor /tmp/cna-storage-stream-lifetime.c (STALE): eight additions preserve the save format and
all23original checks. Device/container parent destruction is refused while children live.
A seek selects the last8bytes (nonzero name tail plus padding), a12byte request into14allocated
bytes proves short-read data/guards, a second EOF read preserves output, position must be24,
then rewind and original full-save read still work; a closed handle leaves query output intact.
Two routes plus knownIssue056 are applied in metadata. Native probe confirms exactly the position failure;
remaining30checks pass on both requested renderer environments, empty stderr. No Game is created.

EOF proof helpers /tmp/cna-build-storage-eof-probe.py and -canonical-eof-probe.py write only
/tmp objects/executables using existing compile/link configuration (no dependency-file edits,
no second build tree). /tmp/cna-run-eof-probes.py and /tmp/cna-review-eof-probes.py completed.
Direct canonical probe uses a unique /tmp file, reads4nonzero bytes with intact guards, gets
position-1 after short/empty reads, then successfully rewinds/rereads. Exit1 solely means the
position contract failed; fixture/other-control failures would be exit2. C probes each exit1
with30/31checks passing and SUCCESS/-1 instead of24. Exact original23child list plus8named
additions verified; all35shipped binary hashes unchanged. Source observations are clean CNA
59598bfac at first review and714f9f76a at later review, sharp-runtime c20373bae clean; none is
an atomic input snapshot. Evidence/sources/helpers: reports/checkpoint-466-transfer-eof-*.
Focused runner /tmp/cna-run-467-focused.py before/after and exact review completed.


APPLIED follow-up /tmp/cna-uniform-scene.cpp (predecessor now stale) extends existing uniform_palettes rather
than adding an ID. It retains four original checks in the first two resized viewports and
adds a third scene-wide parent transform/premultiplied tint, using actual scalar SetUniformMat4
and SetUniformVec4 alongside the existing arrays. Explicit16float matrix layout, caller-buffer
overwrite, expected shifted centers RGB(0,32,128)/(64,0,128), vacated old centers and retained
gap distinguish transform order and tint/alpha. Six intended checks. Before/after images will
change from two rows to three; preserve old assertions but do not claim pixel identity.
Source proposal; syntax passed, native/visual after evidence now verified (see section14). Full467is closed.

APPLIED follow-up /tmp/cna-storage-read-only.c (predecessor now stale) extends the now-applied31check C
save_game. It adds7coherent permission controls: Read+Truncate refusal before file changes,
valid explicit Read open, combined CanRead/CanWrite/CanSeek inspection, refused one-byte
write, unchanged position/length, exact original save bytes and successful close. Actual
storage.h routes and canonical exception mapping (NotSupported -> CNA_RESULT_NOT_SUPPORTED)
were reviewed. Existing31checks including ISSUE-056 remain; expected38total/one known failure.
Four new C routes would need metadata (open_file_access and three capability queries).
Syntax and native validation passed (37positive checks/only existing056). Preserve the same demo-owned file/cleanup, no new ID.

Next helpers are now running as described in section14: /tmp/cna-467-next-syntax.py checks both follow-up drafts
with existing C++/C flags; /tmp/cna-run-467-scene-focused.py captures current palette/storage
before application, then after. Full467is closed; before captures/syntax completed successfully. Palette4->6GLchecks/SWskip;
C31->38childchecks with exactly the same ISSUE-056 failure. Two GL images should be inspected;
layout changes from2rows to3, so pixel identity is not expected. Planned same-count checkpoint
label467-scene avoids overwriting current467archives. New C route total would174, no new ID.

## Complete467-light

Complete checkpoint **467-light**, source `2588b74`, preserves all 467 IDs. Build: seven
scheduled operations; metadata 467/0; all 480 enabled CTests pass in 235.91 s (448 passed,
including 27 WILL_FAIL, 32 capability skips; three additional manual tests disabled).
SOFTWARE: 405 passed / 27 failed / 32 skipped / 3 not run. OPENGLES3: 432 / 29 / 3 / 3.
Both inner validation and outer Xvfb independently exit zero. All 38/42 original failed
assertions, C/custom child lists and traces, and five network evaluation checks are retained,
allowing precisely ten new positive C SpriteBatch End-release checks (19 -> 29).
Skinned lighting checks increase 4 -> 8 on both renderers. Exact JSON count variations are
archived in each renderer's review; only positive window.title Update variance is allowed.
All 35 selected binary and four asset hashes remain unchanged. Normalized application stderr
is identical, including 42 known GL X protocol records and the deliberate shader compiler
error. Separate outer Xvfb diagnostics contain 16 exact amdgpu pairs (previously 13); no cause
or hardware GPU claim. CNA 714f9f76a was observed clean at capture/final, not an atomic input
snapshot. All capture/archive/finish helpers pass. Immutable evidence: checkpoint-467-light-*.
Coverage remains 293/329 types, 1373/2326 XNA members and 174/3210 C routes. No new demo ID or
route was needed for these behavioral extensions; comprehensive coverage is not established.

JSON count review: `{"OPENGLES3": {"graphics.skinned_effect.specular_lights": {"after": 8, "before": 4, "reason": "Four reviewed independent diffuse/additive controls."}}, "SOFTWARE": {"graphics.skinned_effect.specular_lights": {"after": 8, "before": 4, "reason": "Four reviewed independent diffuse/additive controls."}}}`.

## Complete467-field

Complete checkpoint **467-field**, source `d4eee34`, preserves all 467 IDs. The external CNA
checkout advanced from714f9f76a to clean1bc9ff044 (LINK-032 mold archive extraction) at the
build boundary. CMake regenerated and scheduled53operations (50numbered completions), including
relinking CNA and its consumers. Metadata467/0 and all480enabled CTests pass in326.00s
(448passed incl27WILL_FAIL,32capability skips; three additional manual tests disabled).
The known vendored Draco CMP0148 developer warning remains; no compiler diagnostic is hidden.
Concurrent cc1plus processes/host load22.10were observed, without isolating a performance cause.
SOFTWARE405/27/32/3 and OPENGLES3 432/29/3/3; independently journaled inner/outer exits both0.
Exact comparisons retain all38/42old failed assertions, all C/custom child lists/traces and
all five network evaluation controls, allowing only one new C sprite array-refusal child
(29->30). Volume10->16is the intended JSON count change on both. Any positive window.title
Update variance is recorded explicitly in the renderer reviews.
All35selected binary/fourasset hashes captured after the rebuild remain stable. Normalized
application stderr is identical, including42known GL X protocol records and the intentional
shader compiler error. Separate outerXvfb diagnostics contain15exact amdgpu pairs (previously16);
no hardware GPU claim. After-build source capture observed cleanCNA1bc9ff044; external sources
subsequently advanced (mid-run547c3092a plus optional-service changes; final 60aedeab57 with external uncommitted changes).
Selected runtime hashes stayed fixed across that source work. Focused images used prior714f9f76a.
The full checkpoint exercises the captured binaries, not those later source edits; this is not
an atomic source/input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-467-field-*.
Generated coverage remains293/329types,1373/2326XNA members,174/3210C routes. Semantic coverage
grew without adding IDs/routes; comprehensive coverage is not established.

JSON count review: `{"OPENGLES3": {"graphics.texture3d.volume_data": {"after": 16, "before": 10, "reason": "Six reviewed typed vector-field transfer controls."}}, "SOFTWARE": {"graphics.texture3d.volume_data": {"after": 16, "before": 10, "reason": "Six reviewed typed vector-field transfer controls."}}}`.

## Complete467-move

Complete checkpoint **467-move**, source `1721771`, preserves all 467 IDs. Focused validation
retains six original GL sampler controls plus two ownership controls, with inspected identical
before/after RGBA and fifteen verified centers. SOFTWARE retains its declared CustomEffects skip.
The broad focused build and a consistency build each observed unchanged hashes for 3,906 watched
CNA module/header/CMake files. Source commits advanced while prior dirty contents were committed;
this does not itself imply changing build inputs. The known vendored Draco CMake developer warning
is preserved. The full validation build scheduled seven operations; metadata467/0 and all480enabled
CTests pass in207.06s (448passed incl27WILL_FAIL,32skips; three additional manual tests disabled).
SOFTWARE405/27/32/3 and OPENGLES3 432/29/3/3. All38/42previous failed assertions, every C/custom child
list/trace and all five network evaluation controls remain exact, with no additions or filtering.
The planned JSON change is GL sampler6->8; positive window.title Update variance, if present,
is recorded separately in the renderer reviews. All35selected binary/fourasset hashes captured
after the full build remain stable. Normalized application stderr is unchanged, including the42
known GL X protocol records and intentional shader error. Separate outerXvfb diagnostics contain
15exact amdgpu pairs (previously15). Both independently journaled inner/outer exits are0.
Capture source observation: CNA0ae1c41b18 (dirty);
final observation: CNA5a85d5ed24 (clean).
Selected artifact verification is not an atomic source or full transitive dependency snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-467-move-*.
Generated coverage remains293/329types,1373/2326XNA members,174/3210C routes. Semantic ownership
coverage grew without adding IDs; comprehensive coverage is not established.

JSON count review: `{"OPENGLES3": {"graphics.texture3d.sampler_coordinates": {"after": 8, "before": 6, "reason": "Two reviewed volume ownership controls."}}, "SOFTWARE": {}}`.

## Complete467-readers

Complete checkpoint **467-readers**, source `583ee6f`, preserves all 467 IDs. The existing
reader_versions demo retains every17custom-policy control and adds six built-in lifetime controls:
withdrawal, cache independence, refusal without resurrection and a subsequent Game declaration.
Both requested environments pass23checks; neither Game runs and no renderer/visual credit is claimed.
Console ownProcess keeps the mutation out of the parent/browser registry. Original three fixtures
remain byte-identical; the new90byte signed primitive is generated and staged identically.
Focused build9operations/8numbered completions, metadata467/0 and23targeted CTests3.16s passed.
The full build schedules7operations; metadata467/0 and all480enabled CTests pass in169.66s
(448passed incl27WILL_FAIL,32skips; three additional manual tests disabled).
SOFTWARE405/27/32/3 and OPENGLES3 432/29/3/3. All38/42previous failed assertions, every C/custom
child list/trace and all five network controls remain exact. Only planned JSON changes are
reader17->23both; positive window.title Update variance, if present, is recorded in the reviews.
All35selected binary/fourexistingasset hashes and both copies of the newfixture remain stable
through runtime. Normalized application stderr is unchanged, including42known GL X protocol
records and the deliberate shader compiler error. Separate outerXvfb diagnostics contain11
exact amdgpu pairs (previously15); independently journaled inner/outer exits are0.
Capture source observation: CNAe842e74733 (clean);
final observation: CNAe842e74733 (clean).
The e842e7473 commit before capture changed only external link-architecture documentation.
These source observations and selected runtime hashes are not an atomic/full dependency snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-467-readers-*.
Generated coverage now records293/329types,1374/2326XNA members,174/3210C routes; explicit
Game construction adds one named member. Semantic reader
lifetime coverage grew without a new ID; comprehensive coverage is not established.

JSON count review: `{"OPENGLES3": {"content.xnb.reader_versions": {"after": 23, "before": 17, "reason": "Six reviewed built-in reader lifetime controls."}}, "SOFTWARE": {"content.xnb.reader_versions": {"after": 23, "before": 17, "reason": "Six reviewed built-in reader lifetime controls."}}}`.

## Complete468-formats

Complete checkpoint **468-formats**, final source `aad1917`, adds one coherent
stereo buffer-format console example. All16new controls pass in both requested environments:
byte/sample ranges, integer frame-alignment refusals, preserved paused streams and stopped
format transitions. Original event-refill source/ISSUE038 are unchanged. No renderer/image,
output-sample, audible-quality or concurrent-producer credit is claimed.
The initial full attempt (source d70f2ca, session6704) stopped at CTest after play_cue's fixed700ms
idle assertion failed; one failure among481tests in182.28s. No smoke ran. Original logs, exit1
journals and runtime hashes are preserved as checkpoint-468-formats-initial-ctest-refusal-*.
Ten independent public XACT probe trials did not reproduce the transient; actual buffer-hint
acceptance and the original cause remain unisolated. The demo now polls actual bank state while
pumping with a3s steady-clock limit, retaining all three original predicates. Ten corrected
native repetitions and24XACT/CLI CTests pass, empty stderr; this is no CNA or audible-timing fix.
A fresh full attempt (session98882) builds in7scheduled operations, validates468metadata/0problems
and passes all481enabled CTests in211.17s (449passed incl27WILL_FAIL,32skips; three additional
manual tests disabled). SOFTWARE406/27/32/3 and OPENGLES3 433/29/3/3. Every old38/42failed assertion,
C/custom child list/trace and all five network controls remain exact. The new audio example
passes16; old count differences are limited to any separately reviewed positive title Update
variance. All35selected binary/fourexistingasset hashes plus readerfixture copies stay stable.
Normalized application stderr is identical, including42known GL X protocol records and the
intentional shader error. The fresh outerXvfb log contains16exact amdgpu pairs (previously11);
independently journaled inner/outer exits are0. Capture source observation CNAe842e74733
(clean), final CNAe842e74733 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable complete evidence: checkpoint-468-formats-*.
Generated coverage: 293/329types, 1374/2326XNA members, 174/3210C routes.
Semantic format coverage and the completion-check correction do not establish comprehensive CNA coverage.

JSON count review: `{"OPENGLES3": {}, "SOFTWARE": {}}`.

## Complete 469-shadow

Complete checkpoint **469-shadow**, source `a4a2a31`, adds authored-map shadow
reception after skinning. Four paired-pixel controls pass on OPENGLES3: disabled reception,
shadowed direct diffuse with retained ambient/emission, excessive bias and detached map.
The actual image was inspected; eight selected pixels match predictions within tolerance 1.
SOFTWARE explicitly skips the unsupported stock path. OPENGL33 is declared but untested.
No shadow-map generation, PCF kernel, cascade, punctual/specular-shadow or other-renderer credit.
Full session 13123 builds in 7 scheduled operations, validates 469 metadata entries / 0 problems,
and passes all 482 enabled CTests in 220.45s (449 passed including 27 WILL_FAIL, 33 skips;
three additional manual tests disabled). SOFTWARE 406/27/33/3, OPENGLES3 434/29/3/3.
Every old 38/42 failed assertion, C/custom child list/trace and all five network controls remain
exact. Old JSON count differences: {"OPENGLES3": {}, "SOFTWARE": {}}.
All 35 selected binaries / four existing assets and reader fixture copies remain unchanged.
Normalized stderr differs only by the expected SOFTWARE shadow skip diagnostic; GL retains
42 known X protocol records and its intentional shader error. The outer Xvfb log contains
22 exact amdgpu pairs (previously 16); independently journaled inner/outer exits are zero.
Capture CNA e842e74733 (clean), final CNA e842e74733 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-469-shadow-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
Semantic coverage remains incomplete; named-member counts do not establish all behaviors.

## Complete 469-pcf

Complete checkpoint **469-pcf**, source `b8f3d0d`, extends shadow_receiver
without adding an ID. Four original paired-pixel controls remain; three lower-row controls
compare one-tap, 3x3 and 5x5 kernels with visibility 0, 1/3 and 2/5. All seven GL checks pass.
The actual image was inspected; eight original and three new selected pixels match within
tolerance 1. Markers locate the kernel samples. No image-identity claim after the layout change.
SOFTWARE retains its declared skip; OPENGL33 remains untested. No map generation, cascades,
other edge orientations, out-of-range radius clamping or punctual/specular-shadow credit.
Full session 18517 builds in 7 scheduled operations, validates 469 metadata entries / 0 problems,
and passes all 482 enabled CTests in 223.34s (449 passed including 27 WILL_FAIL, 33 skips;
three additional manual tests disabled). SOFTWARE 406/27/33/3, OPENGLES3 434/29/3/3.
Every old 38/42 failed assertion, C/custom child list/trace and all five network controls remain
exact. Reviewed JSON count differences: {"OPENGLES3": {"graphics.skinned_effect.shadow_receiver": {"after": 7, "before": 4, "reason": "Three authored-map kernel controls added; original four paired-pixel predicates retained."}}, "SOFTWARE": {}}.
All 35 selected binaries / four existing assets and reader fixture copies remain unchanged.
Normalized application stderr is identical, retaining 42 known GL X protocol records and the
intentional shader error. The outer Xvfb log contains 18 exact amdgpu pairs (previously 22);
independently journaled inner/outer exits are zero. Capture CNA e842e74733
(clean), final CNA 123c36c34b (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-469-pcf-*.
Generated coverage remains 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
Semantic coverage grew without increasing named-member or demo counts; it remains incomplete.

External CNA advanced during the frozen checkpoint from e842e7473 to clean123c36c34b:
LINK-041 changes declared CNB loader installation/removal locking, LINK-042 refines static
linker-flag matching. Selected runtime hashes remained unchanged; these newer source changes
are not credited as exercised by469-pcf. The next watched build must incorporate them.
Evidence: checkpoint-469-pcf-external-source-delta.json. No sibling files were edited.

## Complete 470-cascades

Complete checkpoint **470-cascades**, source `370bf37`, adds a focused
six-control authored-atlas scene: near/inclusive-split/far selection, half visibility in a
blend band, independent fitting CameraView and Count0 restoring the single-map transform.
All six GL checks pass; the actual image was inspected and six selected colors match within
tolerance1. SOFTWARE explicitly skips the unsupported stock path; OPENGL33 remains untested.
No shadow-map generation, atlas-border filtering, four-cascade maximum or DebugTint credit.
Initial build33371 incorporates external CNA LINK-041/LINK-042 and completes584scheduled /
581numbered operations, but detects six watched CMake input changes during external work.
No native checks ran until consistency90087 completed32scheduled/29numbered operations with
all3,909watched inputs unchanged. Focused/targeted16253 passes26CTests in4.36s, two shadow skips.
Full21772 builds in7scheduled operations, validates470metadata/0problems and passes all483
enabled CTests in221.58s (449passed including27WILL_FAIL,34skips; three additional manual
tests disabled). SOFTWARE406/27/34/3, OPENGLES3 435/29/3/3. Every old38/42failed assertion,
C/custom child list/trace and all five network controls remain exact. Old JSON count differences:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All35selected binaries/four existing assets and readerfixture copies remain unchanged.
Normalized application stderr adds only the declared SOFTWARE cascade skip; GL retains42known
X protocol records and the intentional shader error. The outer Xvfb log contains12exact
amdgpu pairs (previously18); independently journaled inner/outer exits are zero.
Capture CNA123c36c34b (dirty), final CNAdc9c34f62f (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-cascades-*.
Generated coverage: 293/329types, 1374/2326XNA members, 174/3210C routes.
Semantic coverage remains incomplete; authored cascade sampling does not establish all shadows.

The initial full470-query attempt stopped before CTest on a content-tool staging warning.
A private native probe using unchanged CNA staging code reproduces the exact diagnostic in
all three concurrent trials (1,000 owners each), while a1,000-owner sequential control reports
zero diagnostics. No owner errors/removals, empty final private parents, unchanged35selected
binaries/four assets. This is CNA-DEMOS-ISSUE-057, a build-tool diagnostic race; the original
build interleaving remains unobserved and no content corruption is claimed. Original failure,
probe sources/executables/logs and review are archived under checkpoint-470-query-initial-staging-refusal-*
and checkpoint-470-query-staging-probe-*. The warning gate remains unchanged; a fresh full retry
is pending. No runtime demo knownIssue marker or extra demo ID is added.

## Complete 470-query

Complete checkpoint **470-query**, source `68980d0` (query code a699a2d),
extends the existing occlusion scene with eight ordering/disposal controls. All eleven checks
pass on SOFTWARE and OPENGLES3; the three original visibility controls and complete
Update/Draw/EndRun source remain unchanged. Four actual before/after images were inspected.
Fixed counts remain 0/4422 (SOFTWARE, precise) and 0/1 (OPENGLES3, boolean). Moving-light
positions are time-dependent; no image-identity or GPU completion-latency claim.
Initial watched build81666 passes9scheduled/8numbered operations, all3,909watched inputs
unchanged at clean dc9c34f62. Before70538 and after/targeted95414 pass;16CTests in3.46s.
The initial full attempt (sourcea699a2d) stopped before metadata/CTest/smoke after a staging
warning, inner/outer exits1. Original evidence: checkpoint-470-query-initial-staging-refusal-*.
Private unchanged-CNA-header concurrency probes reproduce the missing-metadata diagnostic in
all three1,000-owner trials, while a1,000-owner sequential control has no diagnostics. Zero
owner errors/scavenger removals and empty final private parents; selected35binaries/4assets
stay unchanged through probes. This is open CNA-DEMOS-ISSUE-057, a tool diagnostic race, not
an occlusion failure or demonstrated content corruption. Original exact interleaving remains
unobserved. Probe evidence: checkpoint-470-query-staging-probe-*. A clean retry does not fix it.
Fresh full16616 builds in7scheduled operations without warning, validates470metadata/0problems
and passes all483enabled CTests in165.69s (449passed including27WILL_FAIL,34skips; three
additional manual tests disabled). SOFTWARE406/27/34/3, OPENGLES3 435/29/3/3. Every old38/42
failed assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences: {"OPENGLES3": {"graphics.occlusion_query.visible_pixels": {"after": 11, "before": 3, "reason": "Eight scratch-query lifecycle controls added; original three visibility predicates retained."}}, "SOFTWARE": {"graphics.occlusion_query.visible_pixels": {"after": 11, "before": 3, "reason": "Eight scratch-query lifecycle controls added; original three visibility predicates retained."}}}.
All35selected binaries/four existing assets and readerfixture copies remain unchanged through
this fresh runtime attempt. Normalized application stderr is identical, including42known GL
X protocol records and the intentional shader error. The fresh outer Xvfb log contains13
exact amdgpu pairs (previous complete12, initial refusal1); independent inner/outer exits0.
Capture CNAeff3131017 (clean), final CNAeff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All fresh capture/archive/finish helpers pass; immutable complete evidence: checkpoint-470-query-*.
Generated coverage: 293/329types, 1374/2326XNA members, 174/3210C routes.
Semantic coverage remains incomplete; no extra demo or runtime knownIssue marker was added.

## Complete470-border

Complete checkpoint **470-border**, source `54dfda7`,
extends the existing cascade demo with two shared-boundary isolation controls. All eight
GL checks pass; SOFTWARE retains its declared stock-path skip. The actual three-row image
was inspected and six relocated original plus two new selected pixels match within 1.
Both new controls use atlas UV (.5,.5), receiver depth .5 and requested radius 2. Selected
near/far slices remain independently occluded/lit. This establishes slice isolation, not
all 25 filter weights inside a cascade, arbitrary atlas layouts or other renderers.
Watched build 36795 passes 9 scheduled / 8 numbered operations with all 3,909 watched inputs
unchanged. Focused/targeted session 19479 passes 26 CTests in 2.69 s; original six labels retained.
Full session 88833 builds in 7 scheduled operations without warning, validates 470 metadata entries with 0 problems
and passes all 483 enabled CTests in 150.80 s (449 passed including 27 WILL_FAIL, 34 skips;
three additional manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3.
Every old 38/42 failed assertion, C/custom child list/trace and all five network controls
remain exact. Reviewed JSON count differences:
{"OPENGLES3": {"graphics.skinned_effect.shadow_cascades": {"after": 8, "before": 6, "reason": "Two shared-edge slice-isolation controls added; original six cascade predicates retained."}}, "SOFTWARE": {}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X protocol
records and the intentional shader error. Outer Xvfb has 18 exact amdgpu pairs (previous 13);
independent inner/outer exits 0. Capture CNA eff3131017 (clean),
final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-border-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
The additional source-only spatial-audio audit classifies listener/emitter properties,
Doppler, orientation, multi-listener approximation and Pan/3D mode changes. It corrects
an old range-error gap: volume_pitch_pan already checks three refused ranges. No listening
or runtime credit comes from that source review.
No new ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057 stays
open with its original refusal and independent private staging probe preserved.

## Complete470-specular

Complete checkpoint **470-specular**, source `c9660d8`,
extends the existing shadow receiver from seven to nine GL controls. All nine pass with
empty focused stderr; SOFTWARE retains its exact declared stock-path skip. Seven original
assertion labels remain unchanged. The actual three-row 800x480 image was inspected and
all 15 selected pixels, including relocated original controls, match within tolerance 1.
The new specular-only panels have zero diffuse, ambient and emission. Disabled reception
preserves both RGB(76,38,13) highlights; enabled reception makes only the occluded half
black. This is single-map per-pixel directional specular evidence, not per-vertex, alpha,
cascade or punctual-specular behavior; no whole-image identity after the layout change.
Watched build 10959 passes 9 scheduled / 8 numbered operations with all 3,909 watched
inputs unchanged. Focused/targeted session 75346 passes 26 CTests in 2.64 s (two skips).
Full session 80513 builds in 7 scheduled operations without warning, validates 470 metadata
entries with 0 problems and passes all 483 enabled CTests in 150.15 s (449 passed,
including 27 WILL_FAIL; 34 skips; three additional manual tests disabled).
SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed assertion, C/custom child
list/trace and all five network controls remain exact. Reviewed JSON count differences:
{"OPENGLES3": {"graphics.skinned_effect.shadow_receiver": {"after": 9, "before": 7, "reason": "Two isolated specular shadow controls added; original seven receiver predicates retained."}}, "SOFTWARE": {}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 17 exact amdgpu
pairs (previous 18); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-specular-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
No new ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057 stays
open with its original refusal and independent private staging probe preserved.

## Complete470-release

Complete checkpoint **470-release**, source `f08554f`,
extends the existing looping demo from three to six checks. Ten dummy-audio repetitions
(five per requested renderer environment) pass with empty stderr. The complete original
function prefix through its three old controls remains unchanged. Stop(false) leaves a
paused voice paused with configured IsLooped true; Resume starts it, and it then naturally
stops within a 3 s steady-clock deadline. Observed post-Resume waits in focused repetitions
are 0.239-0.261 s; these are polling observations, not audible duration/latency or sample
content evidence. Pausing before release avoids a racy immediate-Playing assertion.
Watched build 64542 passes 9 scheduled / 8 numbered operations with all 3,909 watched
inputs unchanged. Focused/targeted session 78295 passes 37 enabled CTests in 5.65 s;
one additional manual microphone test is disabled. Metadata has 470 entries / 0 problems.
Full session 59103 builds in 7 scheduled operations without warning and passes all 483
enabled CTests in 157.07 s (449 passed including 27 WILL_FAIL; 34 skips; three additional
manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed
assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences:
{"OPENGLES3": {"audio.sound_effect_instance.looping": {"after": 6, "before": 3, "reason": "Three paused-loop release controls added; original three looping predicates retained."}}, "SOFTWARE": {"audio.sound_effect_instance.looping": {"after": 6, "before": 3, "reason": "Three paused-loop release controls added; original three looping predicates retained."}}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 18 exact amdgpu
pairs (previous 17); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-release-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
No extra ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057
stays open with its original refusal and independent private staging probe preserved.

## Complete470-spatial

Complete checkpoint **470-spatial**, source `9a6234b`,
extends the existing apply_3d demo from one to seven checks. Two dummy-audio runs (one per
requested renderer environment) pass with empty stderr. The complete original audible
sweep/default-speed check source remains verbatim. Six added controls demonstrate paused
Pan/3D refusals, selecting the other mode after Stop, and re-aiming a paused3D voice.
The refused Pan setter exposes its newly assigned .75 public value while State remains
Paused. This is observed CNA property/state behavior, not by itself a CNA defect or a
measurement of output panning. No attenuation, Doppler or new listening credit; SmokeOnly
remains appropriate for the original audible sweep.
Watched build 53385 passes 9 scheduled / 8 numbered operations, all 3,909 watched inputs
unchanged and diagnostic gate 0. Focused/targeted session 84713 passes 37 enabled CTests
in 5.60 s; one additional manual microphone test disabled. Metadata 470 / 0 problems.
Full session 17556 builds in 7 scheduled operations without warning and passes all 483
enabled CTests in 148.64 s (449 passed including 27 WILL_FAIL; 34 skips; three additional
manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed
assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences:
{"OPENGLES3": {"audio.sound_effect_instance.apply_3d": {"after": 7, "before": 1, "reason": "Six public Pan/3D mode controls added; original sweep and speed-default predicate retained."}}, "SOFTWARE": {"audio.sound_effect_instance.apply_3d": {"after": 7, "before": 1, "reason": "Six public Pan/3D mode controls added; original sweep and speed-default predicate retained."}}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 18 exact amdgpu
pairs (previous 18); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-spatial-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
No extra ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057
stays open with its original refusal and independent private staging probe preserved.

## Complete470-bounds

Complete checkpoint **470-bounds**, source `41af406`,
extends buffer_formats from 16 to 20 checks. Two dummy-audio runs (one per requested
renderer environment) pass with empty stderr. Negative float offset and max-int count
preserve the staged integer queue; negative count and a nonempty range at the vector end
preserve the staged float queue. Each refusal is ArgumentOutOfRangeException with
Stopped/Pending1 retained. All 16 original assertion calls remain verbatim/in order,
including original mode guards and integer/float playback transitions. No sample readback,
partial-float output, live-playing/concurrent-producer or listening evidence. Float range
bounds differ from integer frame alignment; no invented alignment exception is expected.
Watched build 1507 passes 9 scheduled / 8 numbered operations, all 3,909 watched inputs
unchanged and diagnostic gate 0. Focused/targeted session 14824 passes 37 enabled CTests
in 5.60 s; one additional manual microphone test disabled. Metadata 470 / 0 problems.
Full session 24481 builds in 7 scheduled operations without warning and passes all 483
enabled CTests in 149.72 s (449 passed including 27 WILL_FAIL; 34 skips; three additional
manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed
assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences:
{"OPENGLES3": {"audio.dynamic_sound_effect_instance.buffer_formats": {"after": 20, "before": 16, "reason": "Four float-range refusal controls added; original sixteen buffer-format predicates retained."}}, "SOFTWARE": {"audio.dynamic_sound_effect_instance.buffer_formats": {"after": 20, "before": 16, "reason": "Four float-range refusal controls added; original sixteen buffer-format predicates retained."}}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 11 exact amdgpu
pairs (previous 18); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-bounds-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
No extra ID or issue marker. Semantic coverage remains incomplete; build-only ISSUE-057
stays open with its original refusal and independent private staging probe preserved.

## Complete470-clear

Complete checkpoint **470-clear**, source `10a9593`,
extends buffer_formats from 20 to 23 checks. Guarded application verified the unchanged
validated41af406 base. Two dummy-audio runs (one per requested renderer environment) pass
with empty stderr. ClearBuffers retainsPaused/Pending0; a valid float submission still
refuses with that same state; same-format refill and visible QueueInitialBuffers retain
Paused/Pending1 before originalResume/Stop. All 20 original assertion calls remain verbatim
and in order. Counts combine staged/submitted data and do not independently prove native
transfer or output samples. No live-playing/concurrent-producer or listening claim.
Watched build 47439 passes 9 scheduled / 8 numbered operations, all 3,909 watched inputs
unchanged and diagnostic gate 0. Focused/targeted session 51461 passes 37 enabled CTests
in 5.61 s; one additional manual microphone test disabled. Metadata 470 / 0 problems.
Full session 79728 builds in 7 scheduled operations without warning and passes all 483
enabled CTests in 149.94 s (449 passed including 27 WILL_FAIL; 34 skips; three additional
manual tests disabled). SOFTWARE 406/27/34/3, OPENGLES3 435/29/3/3. Every old 38/42 failed
assertion, C/custom child list/trace and all five network controls remain exact.
Reviewed JSON count differences:
{"OPENGLES3": {"audio.dynamic_sound_effect_instance.buffer_formats": {"after": 23, "before": 20, "reason": "Three paused-queue clearing and format-retention controls added; original twenty predicates retained."}}, "SOFTWARE": {"audio.dynamic_sound_effect_instance.buffer_formats": {"after": 23, "before": 20, "reason": "Three paused-queue clearing and format-retention controls added; original twenty predicates retained."}}}.
All 35 selected binaries/four existing assets and reader fixture copies remain unchanged
through runtime. Normalized application stderr is identical, including 42 known GL X
protocol records and the intentional shader error. Outer Xvfb has 14 exact amdgpu
pairs (previous 11); independent inner/outer exits 0. Capture CNA eff3131017
(clean), final CNA eff3131017 (clean).
These are selected artifact/source observations, not an atomic/full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-clear-*.
Generated coverage: 293/329 types, 1374/2326 XNA members, 174/3210 C routes.
Original mono streaming/ISSUE-038 is untouched. No extra ID or issue marker; semantic
coverage remains incomplete. Build-only ISSUE-057 stays open with its original refusal
and independent private staging probe preserved.

## Complete470-fourth

Complete checkpoint **470-fourth**, source `6f27142`,
extends the existing shadow_cascades from eight to ten GL pixel checks. At Count4,
fitting depth7 selects slot3: its comparison depth .5 is lit RGB(173,130,77), while
.9 with DebugTint produces the shadow floor RGB(20,28,15). CNA uses four horizontal
atlas strips. Both actual800x480 GL images were inspected; all eight original pixels
remain, two new pixels match within tolerance1, and regions outside the replaced ninth
cell are RGBA-identical. The original eight drawing/check loops remain verbatim.
SOFTWARE retains its exact declared skip; GL stderr is empty. This does not demonstrate
every four-cascade index/debug color, arbitrary atlases, invalid counts, a producer or
other renderers. No whole-image identity or additional demo ID is claimed.
The watched build and focused review passed before source commit; original evidence is
under checkpoint-470-fourth-focused-*. Full validation passes all483enabled CTests in
148.94s, with three additional manual tests disabled. SOFTWARE406/27/34/3 and
OPENGLES3 435/29/3/3 retain all38/42prior failed assertions, C/custom child evidence and
five network controls. Reviewed JSON count changes:
{"OPENGLES3": {"graphics.skinned_effect.shadow_cascades": {"after": 10, "before": 8, "reason": "Two fourth-slot depth/debug-tint controls added; original eight cascade predicates retained."}}, "SOFTWARE": {}}.
All35selected binaries, four existing assets and reader fixture copies stay unchanged
through runtime. Normalized application stderr is identical, including42known GL X
protocol records and the intentional shader error. Outer Xvfb has17exact amdgpu
pairs (previous14); independent inner/outer exits are0. Capture CNA
eff3131017 (clean), final CNA
eff3131017 (clean). These are selected
artifact/source observations, not an atomic or full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-fourth-*.
Semantic coverage remains incomplete, and build-only ISSUE-057 stays open with its
original staging refusal and independent private reproduction preserved.

## Complete470-authored

Complete checkpoint **470-authored**, source `f44b514`,
extends cue_states from nine to fifteen checks. Six dummy-audio runs pass with empty
stderr. All nine original predicates remain in order; the misleading Immediate label
is corrected. Public controls now compare immediate no-fade AsAuthored stop, an authored
cue entering Stopping and reaching Stopped, and Immediate interruption of a fresh tail.
The generator adds an original three-second PCM bank and a complex single-sound cue with
500ms fade-out. All six existing XACT assets remain byte-identical; all eight source/staged
banks are checked. Observed state waits are 500–510ms, including caller
scheduling and polling; no acoustic envelope, audible quality or output-sample timing claim.
The watched build and focused review passed before source commit; original evidence is
under checkpoint-470-authored-focused-*. Full validation passes all483enabled CTests in
149.50s, with three additional manual tests disabled. SOFTWARE406/27/34/3 and
OPENGLES3 435/29/3/3 retain all38/42prior failed assertions, C/custom child evidence and
five network controls. Reviewed JSON count changes:
{"OPENGLES3": {"audio.xact.cue_states": {"after": 15, "before": 9, "reason": "Six authored-release controls added; original nine predicates retained, one misleading label corrected."}}, "SOFTWARE": {"audio.xact.cue_states": {"after": 15, "before": 9, "reason": "Six authored-release controls added; original nine predicates retained, one misleading label corrected."}}}.
All35selected binaries, four existing assets and18fixture hashes (source/staged builtinXNB
and all eight XACT banks) stay unchanged through runtime. Normalized application stderr
is identical, including42known GL X protocol records and the intentional shader error.
Outer Xvfb has9exact amdgpu pairs (previous17); independent inner/outer exits0.
Capture CNA eff3131017 (clean), final CNA
eff3131017 (clean). These are selected
artifact/source observations, not an atomic or full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-470-authored-*.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is untouched;
build-only ISSUE-057 retains its staging refusal and private reproduction evidence.

## Complete471-punctual

Complete checkpoint **471-punctual**, source `ed7daab`,
adds one coherent SkinnedEffect point/spot receiver demo with eight GL pixel checks.
Authored constant-distance cube/2D maps store64/255; post-skin distance3/Range6 compares
at.5. No map preserves direct diffuse, the map removes it, and normalized bias.3 restores
it. The spot matrix gives projecteddepth.1, deliberately different from distance.5,
so its shadow control distinguishes the comparison quantity. Separate controls exclude
the receiver by range or cone. Linear light intensity4
produces RGB(74,64,43), versus the ambient/emissive floor RGB(20,28,26). The actual800x480
GL image was inspected and all eight selected pixels match within1. GL stderr is empty;
SOFTWARE has the exact declared unsupported-stock-path skip. No producer, all cube
faces/seams, partial spot-kernel weights, cone-edge interpolation or other-renderer proof.
PunctualLightEXT offers diffuse only; no public punctual specular capability is invented.
Initial focused build22225 compiles33scheduled operations but its diagnostic gate
correctly refuses the recurrent ISSUE-057 missing-owner warning. Original logs/input
observations/headers and post-build hashes remain under
checkpoint-471-punctual-initial-staging-refusal-*. The unchanged consistency retry passes
7scheduled operations with3909watched inputs stable and gate0; no CNA edit, warning
suppression or staging-directory deletion. Native/27CTest/pixel review then pass before
source commit. A clean retry/full run does not resolve the independently reproduced race. Evidence:
checkpoint-471-punctual-focused-*. Full validation passes all484enabled CTests in159.93s,
with three additional manual tests disabled. SOFTWARE406/27/35/3 and OPENGLES3 436/29/3/3
retain all38/42prior failed assertions, C/custom child evidence and five network controls.
The new ID is graphics.skinned_effect.punctual_shadows. Reviewed old JSON count changes:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All35selected binaries, four existing assets and18fixture hashes (source/staged builtinXNB
and all eight XACT banks) stay unchanged through runtime. Normalized stderr differs only
by the expected new SOFTWARE skip; GL retains42known X protocol records and the intentional
shader error. Outer Xvfb has14exact amdgpu pairs (previous9); independent
inner/outer exits0. Capture CNA eff3131017 (clean),
final CNA eff3131017 (clean). These are selected
artifact/source observations, not an atomic or full transitive input snapshot.
All capture/archive/finish helpers pass; immutable evidence: checkpoint-471-punctual-*.
Generated coverage:293/329types,1374/2326XNA members.
Semantic coverage remains incomplete; original mono streaming/ISSUE-038 and build-only
ISSUE-057 remain intact, with the latter's staging refusal/private reproduction retained.

## Complete471-cue3d

Complete checkpoint **471-cue3d**, source `057d783`,
extends positional_cue from zero to six checks while retaining its entire original
8-position audible sweep and SmokeOnly classification. Two dummy-audio runs pass with
empty stderr. Explicit owned Cue::Apply3D publishes Distance5, OrientationAngle0 and
neutral Doppler beforePlay; reorientation yields90degrees. While paused, a new position
and velocity produce Distance10 and raw Doppler10/9. These are public variable/state
observations, not acoustic panning, mixed pitch, RPC curve application or listening.
The watched build/diagnostic gate and37targeted CTests pass before source commit;
original loop preservation and actual observations are checkpoint-471-cue3d-focused-*.
Full validation passes all484enabled CTests in150.01s, with three additional manual
tests disabled. SOFTWARE406/27/35/3 and OPENGLES3 436/29/3/3 retain all38/42prior failed
assertions, C/custom child evidence and five network controls. Reviewed count changes:
{"OPENGLES3": {"audio.xact.positional_cue": {"after": 6, "before": 0, "reason": "Six public Cue spatial-variable/state controls added; entire original audible loop retained."}}, "SOFTWARE": {"audio.xact.positional_cue": {"after": 6, "before": 0, "reason": "Six public Cue spatial-variable/state controls added; entire original audible loop retained."}}}.
All35selected binaries, four existing assets and18fixture hashes stay unchanged through
runtime. Normalized stderr is identical, including42known GL X protocol records and the
intentional shader error. Outer Xvfb has13exact amdgpu pairs (previous14);
independent inner/outer exits0. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). These are selected artifact/source
observations, not an atomic or full transitive input snapshot. All capture/archive/finish
helpers pass; immutable evidence: checkpoint-471-cue3d-*. Generated coverage:
293/329types and1378/2326XNA members. No extra demo ID.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is unchanged;
ISSUE-057 retains its original staging refusal, private reproduction and471-punctual
recurrence. Subsequent clean runs do not establish a repair of that race.

## Complete472-banks

Complete checkpoint **472-banks**, source `5fe4921`,
adds audio.xact.bank_lifetime with12 public ownership checks in each requested environment.
A paused consuming cue holds both banks in use; WaveBank disposal disposes it while a
prepared never-played cue and SoundBank survive. SoundBank disposal releases prepared
cues too. Sender/exactly-once counts persist across repeated disposal and deletion of
disposed wrappers. A fresh graph of banks/cues is released by AudioEngine disposal.
Caller-owned allocations remain unique_ptr-owned. No global callback-order, concurrent
teardown or acoustic-latency claim. Two focused runs have empty stderr;38enabled audio/CLI
CTests, watched build/diagnostic gate and focused reviewer pass before source commit.
Full validation passes all485enabled CTests in165.34s, with three additional manual
tests disabled. SOFTWARE407/27/35/3 and OPENGLES3 437/29/3/3 retain all38/42prior failed
assertions, C/custom child evidence and five network controls. One newID passes12checks;
reviewed old count changes:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All35selected binaries, four existing assets and18fixture hashes stay unchanged through
runtime. Normalized stderr is identical, including42known GL X protocol records and the
intentional shader error. Outer Xvfb has18exact amdgpu pairs (previous13);
independent inner/outer exits0. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). These are selected artifact/source
observations, not an atomic or full transitive input snapshot. All capture/archive/finish
helpers pass; immutable evidence: checkpoint-472-banks-*. Generated coverage:
293/329types and1386/2326XNA members.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is unchanged;
ISSUE-057 retains its original staging refusal, private reproduction and471-punctual
recurrence. Subsequent clean runs do not establish a repair of that race.

## Complete472-cuelimits

Complete checkpoint **472-cuelimits**, source `edc32f7`,
extends instance_limits from 7 to 12 public checks, preserving its complete original
category-limit scenario. With spare category capacity, one paused LowTone occupies its
cue-definition slot and a duplicate is refused. A different same-category definition and
a separate SoundBank loaded from the same file have independent capacity. Stop frees the
relevant slot while those other voices remain paused. Two focused runs pass with empty
stderr; 38 enabled audio/CLI CTests, watched build/gate and reviewer pass before source
commit. No replacement-policy, fade, mixed-output or concurrency claim. The new 364-byte
cue_limits.xsb reuses original PCM; all 8 old bank assets remain byte-identical and all
9 source/staged bank pairs match. No extra demo ID.
Full validation passes all 485 enabled CTests in 195.64s; three additional manual tests
are disabled. SOFTWARE 407/27/35/3 and OPENGLES3 437/29/3/3 retain all 38/42 prior failed
assertions, C/custom child evidence and five network controls. Reviewed count changes:
{"OPENGLES3": {"audio.xact.instance_limits": {"after": 12, "before": 7, "reason": "Five per-cue definition capacity controls added; original seven category checks retained."}}, "SOFTWARE": {"audio.xact.instance_limits": {"after": 12, "before": 7, "reason": "Five per-cue definition capacity controls added; original seven category checks retained."}}}.
All 35 selected binaries, four existing assets and 20 fixture hashes stay unchanged through
runtime. Normalized stderr is identical, including 42 known GL X protocol records and the
intentional shader error. Outer Xvfb has 14 exact amdgpu pairs (previous 18);
independent inner/outer exits are zero. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). These are selected artifact/source
observations, not an atomic or full transitive input snapshot. All capture/archive/finish
helpers pass; immutable evidence: checkpoint-472-cuelimits-*. Generated coverage:
293/329 types and 1386/2326 XNA members.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is unchanged;
ISSUE-057 retains its original staging refusal, private reproduction and 471-punctual
recurrence. Subsequent clean runs do not establish a repair of that race.

## Complete473-music

Complete checkpoint **473-music**, source `979a99b`,
adds an isolated tagged music-library workflow with 15 identical child checks in each
requested environment. Focused stderr retains six exact TitleContainer requested/resolved
INFO lines; the initial empty-stderr review refusal is preserved without changing code. Parent JSON has zero checks; the child
lists are explicitly preserved. Both public XDG roots are verified private before library
access. Three original tagged FLAC tracks create two artists, two same-name albums scoped
by artist and unequal by Album::Equals, one genre and an ordered M3U playlist. Public metadata, group membership and
Song back-references share library-owned objects. No duration, playback, album-art,
other-tag-format or other-platform claim. Two private encodings produced identical bytes
and ffprobe verified tags; all three source/staged fixtures match. Watched build/gate,
28 enabled media/CLI CTests and focused reviewer pass before source commit.
Full validation passes all 486 enabled CTests in 149.86s; three additional manual tests
are disabled. SOFTWARE 408/27/35/3 and OPENGLES3 438/29/3/3 retain all 38/42 prior failed
assertions, C/custom child evidence and five network controls. The new standalone's 15
child checks match the focused run exactly. Reviewed old count changes:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All 36 selected binaries, four existing assets and 26 fixture hashes stay unchanged through
runtime. Normalized stderr adds only those six exact INFO lines, retaining 42 known GL X protocol records and the
intentional shader error. Outer Xvfb has 20 exact amdgpu pairs (previous 14);
independent inner/outer exits are zero. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). These are selected artifact/source
observations, not an atomic or full transitive input snapshot. All capture/archive/finish
helpers pass; immutable evidence: checkpoint-473-music-*. Generated coverage:
297/329 types and 1400/2326 XNA members.
Semantic coverage remains incomplete. Original mono streaming/ISSUE-038 is unchanged;
ISSUE-057 retains its original staging refusal, private reproduction and 471-punctual
recurrence. Subsequent clean runs do not establish a repair of that race.

## Complete473-hierarchy

Complete checkpoint **473-hierarchy**, source `be5ac32`,
extends categories from5to10checks while preserving all original predicates/independent
Music/SFX behavior and correcting only the equality label to names. A separate160byteXGS
makes Master parent of SFX: live paused direct/descendant voices, parent Resume/Pause,
child-only Resume and parent Stop from verified both-paused state pass in two dummy-audio
runs with empty stderr. All9oldXACTassets are unchanged; all10source/staged pairs match.
Focused38enabled audio/CLI CTests and reviewer passed before source commit. No new ID,
future-voice pause inheritance, audible volume, deep hierarchy or category-fade credit.

Full tools/validate.sh passes metadata473/0 and486enabled CTests in 166.07s (three further
manual tests disabled). SOFTWARE408/27/35/3 and OPENGLES3 438/29/3/3 preserve all38/42old
failed assertions, C/custom child lists/traces and five network controls. Music import retains
15exact child checks; parentJSON0. Count changes:
{"OPENGLES3": {"audio.xact.categories": {"after": 10, "before": 5, "reason": "Five authored hierarchy controls added; original five predicates retained, name-equality label corrected."}}, "SOFTWARE": {"audio.xact.categories": {"after": 10, "before": 5, "reason": "Five authored hierarchy controls added; original five predicates retained, name-equality label corrected."}}}.
All36selected binaries/four existing assets/28fixture hashes stay unchanged through runtime.
Normalized stderr is identical, including six music TitleContainer INFO lines,42known GL
X protocol records and the intentional shader error. Outer Xvfb has14exact amdgpu pairs
(previous20); independent inner/outer exits are zero. Capture CNAeff3131017
(clean), final CNAeff3131017
(clean). These are selected artifact/source observations,
not an atomic or full transitive input snapshot. All evidence helpers pass; immutable
checkpoint-473-hierarchy-*. Generated coverage:297/329types,
1400/2326XNA members. Semantic coverage remains incomplete.
Original mono streaming/ISSUE-038 and build-only ISSUE-057 remain unchanged; clean runs do
not establish a repair of the staging race. Original refusals and reproduction remain archived.

## Complete473-duration

Complete checkpoint **473-duration**, source `65011a3`,
extends music_index from 15 to 18 child checks. All original relationship and borrowed-object
controls remain; three added checks establish Song durations of 250/500/375 ms, artist-specific
Album sums of 750/375 ms and the two-track Playlist total of 750 ms excluding the third song.
Two focused runs pass 18 identical child checks with the same six TitleContainer INFO lines;
parent JSON remains PASSED/0. The current build has CNA_FFMPEG_AVAILABLE. The optional
non-FFmpeg branch has syntax-only evidence, preserves the original relationship workflow and
prints the unexercised duration boundary. No zero-sum, playback, audible timing or art claim.
Focused 28 enabled media/CLI CTests, watched build/gate and reviewer passed before source commit.
No new ID or fixture changes.

Full tools/validate.sh passes metadata 473/0 and 486 enabled CTests in 159.22s (three further
manual tests disabled). SOFTWARE 408/27/35/3 and OPENGLES3 438/29/3/3 preserve all 38/42 prior
failed assertions, C/custom child lists/traces and five network controls. Music alone gains
three exact child checks; all first 15 remain. Parent count changes:
{"OPENGLES3": {}, "SOFTWARE": {}}.
All 36 selected binaries, four existing assets and 28 fixture hashes stay unchanged through
runtime. Normalized stderr is identical, retaining the six music INFO lines, 42 known GL
X protocol records and intentional shader error. Outer Xvfb has 16 exact amdgpu pairs
(previous 14); independent inner/outer exits are zero. Capture CNA eff3131017
(clean), final CNA eff3131017
(clean). Selected artifact/source observations are not
an atomic or full transitive input snapshot. All evidence helpers pass; immutable archives:
checkpoint-473-duration-*. Generated coverage: 297/329 types,
1402/2326 XNA members. Semantic coverage remains incomplete.
Original mono streaming/ISSUE-038 and build-only ISSUE-057 remain unchanged; clean runs do
not establish a staging-race repair. Preserve all original refusals and reproduction evidence.

## Complete474-album-art

Complete checkpoint **474-album-art**, source `2524511`, adds one coherent standalone over
verified private Linux XDG roots. Two focused and two full-smoke runs pass eight identical child
checks: folder-art/no-art HasArt, both missing-art exceptions, exact original bytes, a distinct
public-decodable128x64thumbnail, and caller-owned streams after MediaLibrary destruction.
Full validation passes metadata474/0 and487enabled CTests in162.75s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failed assertions and protected child/network evidence.
The sole new ID adds eight exact checks.37selected binaries, four assets and30fixture paths remain
unchanged. Stderr adds only six expected album-art INFO lines per renderer. Outer Xvfb has13
exact amdgpu pairs (previous16); exits are zero and CNAeff3131017 stays clean. Coverage is
297/329types and1405/2326members. Evidence:checkpoint-474-art-*.

## Complete474-C-media-ownership

Complete checkpoint **474-C-media-ownership**, source `7d23c6f`, extends the existing C media
demo from32to46child checks without another ID or fixture. Both focused and full runs retain all
32old labels and add14exact collection/queue ownership controls with empty demo stderr. Full
validation passes metadata474/0 and487enabled CTests in154.33s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 preserve all38/42old failures and protected evidence.37binaries, four assets
and30fixture paths remain exact; normalized stderr is identical. Outer Xvfb has15known pairs,
exits are zero and CNAeff3131017 remains clean. C ABI route coverage rises174->184/3210.
Evidence:checkpoint-474-c-media-*.

## Complete474-C-media-lifecycle-bounds

Complete checkpoint **474-C-media-lifecycle-bounds**, source `5141458`, extends the existing C
media demo46->53checks with collection Dispose/idempotence/shared-handle lifetime and empty-queue
bounds/output/index behavior. Both renderers preserve all46old labels and pass seven additions;
ISSUE-059 records the active-output documentation mismatch without a failure marker. Full
validation passes metadata474/0 and487enabled CTests in171.80s. SOFTWARE409/27/35/3 and
OPENGLES3439/29/3/3 retain all38/42old failures and protected evidence.37binaries, four assets
and30fixture paths remain exact; normalized stderr is identical. Outer Xvfb has14known pairs,
exits are zero and CNAeff3131017 stays clean. C ABI routes rise184->188/3210.
Evidence:checkpoint-474-c-media-bounds-*.
