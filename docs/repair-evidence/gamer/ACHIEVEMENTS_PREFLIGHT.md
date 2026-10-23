# Achievement contracts:017/077/078 preflight

After018 repair494e24d09 and its validation, no achievement implementation or demo oracle
has changed. This is investigation, not a current disposition for any of these three IDs.

The genuine XNA4 Windows AchievementCollection implementation uses a read-only IList:
IsReadOnly is true and Add/Insert/Remove/RemoveAt/Clear/index assignment always throw
NotSupportedException. Its string lookup uses Dictionary and therefore has the missing-key
KeyNotFoundException contract. Current CNA follows FNA's permissive mutators while reporting
read-only, and uses FNA's IndexOutOfRangeException for a missing key. The C route promises
INVALID_ARGUMENT but that native exception falls through its generic INTERNAL mapping.

The public C create_ext route explicitly promises copied values; it does not explicitly
describe a mutable extension to XNA. Its mutator descriptions nevertheless promise success.
Review that boundary before repairing077: changing only IsReadOnly to false would preserve
the native XNA divergence. Preserve all behavior controls, and document any correction of
demo expectations against actual XNA evidence. Do not add a broad exception-barrier mapping
merely to hide native IndexOutOfRange faults;078 may need a precise route-level translation
after correcting the native key exception.

AchievementCollectionXnaOracle.cs is a new original, managed-only reference probe for the
genuine XNA DLL/Framework4 CLR. It uses an empty internal collection to avoid live-service
initialization and verifies mutation refusal/error precedence, key types and read controls.
It passes24/24 checks using the installed Microsoft XNA4 GamerServices assembly and genuine
Framework4 CLR under Wine: strict x86 csc build1.939s, runtime0.353s. Assembly token/version and
non-Mono runtime are checked. Reports repair-achievements-xna-oracle-* include input hashes.
This is not native Windows or online-service validation.

CNA's own public native header says these mutators map FNA's explicit IList/ICollection members,
not an intentional mutable CNA extension. Its read-only comment incorrectly defends writable
mutators as a conventional .NET pattern. The measured XNA contract and existing owner XNA-over-FNA
policy therefore require mutation refusal, with explicit corrections to native/C docs and demo
expectations. Keep copied-value creation, read access, ownership, numeric bounds and copy controls.
Do not silently replace one failed property assertion with a pass while leaving mutations wrong.

017 still has earnedOnline_=true in Achievement's constructor. EndGetAchievements has distinct
service, offline-catalog and offline-no-catalog paths; none assigns that field. Contrary to a
historical inventory sentence, SaveEarnedAchievementEXT's last bool is onlyIfUnearned, not an
earned-online flag. Inspect the store and service records before selecting a representation.
The synthetic C achievement demo also expects earned_online=true; changing factory defaults
requires contract review rather than blindly changing the original assertion.

Before repair, all728 native GamerServices tests ran:709 existing PASS, exactly18 new FAIL and1 unchanged SKIP
in93.759s. The witness build reported30 pre-existing nodiscard warnings in unchanged tests.
Original C demo:29/30 on each SOFTWARE/OPENGLES3, one readonly-property failure, before any
implementation or expectation changes. Reports: repair-achievements-before-*.

Candidate changes only mutation refusal and absent string-key behavior in the native collection,
and translates KeyNotFoundException only inside the C key lookup route. It does not change the
shared exception barrier, numeric bounds, copy semantics, ownership or Achievement factory flags.
The ten old collection tests with FNA-based expectations are corrected against genuine XNA;
eight are renamed to describe refusal (InsertAtIndex, InsertAtCount, InsertOutOfRange, RemoveAt,
RemoveAtOutOfRange, Add, Remove, Clear); IndexByKeyNotFound and IsReadOnlyIsAlwaysTrue keep names.
All existing read/copy/equality controls remain. The C smoke now requires exact missing-key result
and output clearing; its copied-handle storage is initialized and its source lookup handle is
released before invalid-index output checks. These fixture fixes prevent false cleanup failures.

The external demo retains its original30 controls, with seven explicit contract corrections:
readonly property, missing-key result, Add/Insert, invalid mutator indices, present removal,
absent removal, Clear. It adds three controls: empty collection creation, full ordered value
preservation after all refusals, and empty-key refusal/output clearing. No tolerance, retry,
exception suppression or unexpected-to-expected failure conversion is involved. Marker077 stays
until the repaired implementation passes. Online flag017 remains outside this batch.

After native repair:727 PASS/1 unchanged SKIP (728 registered) in75.355s. All18 red witnesses pass, eight old mutation tests
have explicit refusal-based names, and other702 names/outcomes remain unchanged. Build passes;
17 existing nodiscard warnings come from unchanged GamerServicesGamerTests.cpp. No new diagnostics.
The first six-test C run was not green: GeneratedRoutesSmoke and BoundaryDetail passed,
AchievementsSmoke failed, and three gamer executables had not yet been built. Keep this evidence.
Investigation found an incorrect new test assumption: generated scalar getters assign only after
native success, so Remove's bool output remains untouched on NotSupportedException; only output
handles are proactively cleared. No old public contract promised false on failure. The test and
new docs now require the original sentinel unchanged, preserving that established boundary; no
production output policy or generator change. A bounded gdb probe was stopped during symbol load
without yielding evidence; source inspection identified this directly. The three missing C
executables are being built before rerunning all six. Generated routes remain current3197/57.
The corrected C run now passes all six controls in0.19s. The complete consumer rebuild is running
with one worker after verifying all275 hashes of the preceding018 consumer. Native/runtime
collection logic did not change after the727-PASS/1-unchanged-SKIP native run; the follow-up changed only test/docs
for scalar output semantics. No memory ownership implementation changed, so no sanitizer claim.

Next017 investigation: the available genuine Windows XNA Achievement property getters throw the
ProFeatureNotSupported exception; do not claim their online/offline values were runtime-verified.
The property documentation defines earned-while-online. Current CNA service records hold only
an earned timestamp and all awards are authenticated service calls; offline storage holds key and
ticks only (its bool parameter controls write-once, not online provenance). Inspect the synthetic
factory's explicit contract separately from SignedInGamer's three materialization paths.

The first surrounding consumer run exposed another FNA-based demo expectation in
`gamer_services.achievement.award`: it catches IndexOutOfRangeException for a missing collection
key and later requires all five collection mutations to succeed while IsReadOnly is true. After
the native fix this prematurely aborts at the missing key after10 checks, beyond its existing
EarnedOnline017 failure. The same genuine24-check oracle justifies correcting these two controls
as part of077/078: catch exact KeyNotFoundException, require exact NotSupportedException for every
mutator and compare all values/order. All its other controls, including the existing failing017
assertion and marker, remain. Preserve this first-run evidence; it is not a passing checkpoint.
After complete consumer build (zero compiler warnings), focused C demo passes33/33 on SOFTWARE
and33/33 on OPENGLES3. Only then remove077 marker and rebuild metadata. The subsequent35-scenario
run retains the first failure of the stale C++ award oracle as evidence. After correcting that
oracle, rerun that affected scenario on both renderers; unchanged surrounding scenarios need not
be repeated solely because their common executable relinks. Verify all other274 frozen artifacts
are unchanged, and compare the latest complete result for each of the35 IDs against075. This is
an explicit sequential validation checkpoint, not a claim that the first run was green.

Final077/078 validation checkpoint:
- C++ award demo now completes its original14 controls on SOFTWARE and OPENGLES3 with only the
  original017 failure/message/marker. Both collection controls use the measured XNA contract.
- Latest results for35 surrounding scenarios per renderer:32PASS/2unchangedFAIL/1unchangedNOTRUN.
  The exact outcome/check-count/marker/message and logged-assertion comparison against075 has
  only one changed ID: c_api.gamer.achievement_collection changes FAILED to PASSED, removes077,
  and passes33 controls. The other34 IDs retain their previous outcomes;017/025 remain open.
- The first comparison correctly rejected the incomplete award run. Final comparison uses the
  explicit focused rerun for that one ID. Its parser was corrected for the single-demo CLI's
  absent heading and the standalone wrapper's repeated heading; these are tooling corrections,
  not changes to expected product results. Reports and first-run failures remain on disk.
- Only cna-demos relinked after that oracle correction; all other274 consumer artifacts retained
  their first-run hashes. All275 final hashes verified after final tests; freeze released.
- Catalog719 remains valid, with metadata changed only for the repaired C collection.
  Five registered achievement/catalog CTests PASS in11.48s; the award CTest still uses its
  original known017 expected-failure policy. Generated3197 routes/57 files remain current.
- Full native727PASS/1unchangedSKIP, six native C controlsPASS, complete consumer buildPASS. This is not a new
  full719 smoke/native aggregate/sanitizer/visual/browser/online-service/platform checkpoint.
  The last complete cross-subsystem checkpoint remains075 with its six unchanged CTest residuals.
  All builds use existing trees/shared ccache/one worker/6GiB memory ceiling; GPU runs isolated.

Validated native repair commit: `4a756413f779f5f74d5d3e48e92bcb53ecc00b0e` (077/078). Permanent historical descriptions retained; both dispositions are FIXED_VALIDATED.017 remains open.

Reporting correction: the first summary treated tests-without-failures as PASS and missed the
existing GTest <skipped> element. Exact077/078 totals are709PASS/18FAIL/1SKIP before and
727PASS/1SKIP after. AvatarRendererTest.AFailedGpuUploadFailsEveryDrawCleanlyRatherThanHalfBuilding
skips on OPENGLES3 because it specifically requires a renderer without3D. The same skip is in
both original logs/XML; no runtime regression or new suppression occurred. Corrected matrix,
inventory and handoff retain this limitation explicitly. Subsequent comparisons check skips too.
