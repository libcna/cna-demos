# ISSUE-017: achievement award provenance

Baseline: published CNA f3ac0166b4e79f855b28df156e5cb0d4319e74b8, demos
46b16ee3398e83cddbc981222c62e7e200592319. Both077/078 are closed; do not regress their
read-only collection/key contracts. Original award demo completes14 controls on SOFTWARE and
OPENGLES3 with exactly the EarnedOnline failure. Previous full native GamerServices727PASS/1unchangedSKIP.

The public native getter and genuine XNA XML comment describe whether an achievement was earned
while online. An unearned value cannot be earned online; offline storage also cannot establish
an online award. The available genuine Windows XNA getter throws ProFeatureNotSupported, so
there is no runtime online/offline value comparison to claim. The reference source is
../xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework.GamerServices/
Microsoft.Xna.Framework.GamerServices/Achievement.cs. A web lookup returned an empty tool error;
no external page was obtained or used as evidence.

Current CNA initializes earnedOnline_ to true unconditionally. SignedInGamer's service, offline
catalog, and offline no-catalog materializers never assign it. ServiceAchievement records have
an earned timestamp; awards go through an authenticated service operation. Offline records hold
key and ticks, with no online flag. The historical issue's sentence claiming the store already
records online provenance is incorrect: SaveEarnedAchievementEXT's bool is onlyIfUnearned.
Do not invent a new persistence field to recover information that the offline path never had.

CreateInternal/create_ext are synthetic value factories with no online-provenance parameter;
the C header explicitly calls out offline fixtures. Defaulting such values to earned-online is
unsupported by their inputs. Planned repair: default to false, then mark actual earned service
records online in their materializer. Retain independent IsEarned/date/text/equality semantics,
ABI, storage serialization and offline write-once behavior. Explicitly document this default and
correct only old assumptions that synthetic/unearned values are necessarily online.

Four new native tests precede runtime changes: synthetic earned/unearned values, offline catalog
before/after award and reload, offline no-catalog persistence, and service before/after award with
an unchanged prior snapshot. Existing service fixture is deterministic; it is not a live online
service validation. Build and red evidence pending. No existing oracle or implementation changed.

All four focused witnesses fail before repair (4/4FAIL,0.002s), at the online flag assertions;
service award positive control is already true, as expected. Reports repair-achievement-origin-
before-native.{xml,log}. The candidate changes only the constructor default to false and assigns
true for earned records in the service materializer. Offline records need no new fields or I/O.
The header and C documentation state the synthetic default explicitly. One old native assertion
(unearned synthetic value reports true) and the C synthetic fixture's true flag expectation are
corrected because neither has online provenance. Their remaining fields and controls stay exact.
C smoke now asserts false for both synthetic values and an actual persisted offline award.
The C signed-in demo adds one control covering offline values before and after award. Original
C++ award14 assertions and017 marker remain unchanged until the repaired build passes.
The native service adapter sends both achievements.list and achievements.award through tokenFor(user),
and the fake backend requires its signed-in identity for both. No protocol field claims an offline
upload: earnedTicks is the only award indicator. This supports setting online provenance only in
the service materializer, while leaving offline/default creation false. Live service validation is
not inferred from the deterministic fake. None of the077/078 collection implementation changed.

After native repair:732 registered tests,731PASS/1 unchanged declaredSKIP in78.538s, no failures.
Exact name/status comparison (including <skipped>) retains all728 previous outcomes and adds4PASS.
The no-3D upload fixture legitimately skips on OPENGLES3; previous077/078 reporting was corrected
to727PASS/1SKIP and published separately, rather than hiding this count error. Six C gamer/boundary
controls PASS0.20s. Native build has17 existing nodiscard warnings in unchanged test expressions;
complete consumer build passes without compiler warnings. Generated/current route behavior remains
unchanged; no ABI changes, memory ownership changes or sanitizer coverage claim.

Original award demo now passes14/14 unchanged assertions on SOFTWARE (4.908s) and OPENGLES3
(2.474s). Only after those passes remove017 marker and rebuild metadata. The complete consumer
regression run is using275 frozen executable/library hashes; both gamer categories on both
renderers, the exact077/078 checkpoint comparator, and five registered achievement/catalog tests
are running. No closure until these finish. Next candidate batch025/085 remains read-only triage:
offline writer saves only from its rating hook and ColumnsToJson deliberately drops Stream*.

Final validation complete:
- SOFTWARE35 and OPENGLES3 35 each:33PASS/1unchanged085FAIL/1manualNOTRUN. Exact comparison
  against the077/078 checkpoint changes only original award017 FAILED->PASSED (all14 assertions
  retained, marker removed) and the one additional C signed-in provenance control. The other33
  rows retain status/check counts/markers/messages/assertion multiplicities, including the repaired
  C collection's33 controls. No unrelated failure disappears or turns into an expected failure.
- Five registered achievement/catalog CTests PASS11.38s. The award now passes normally; its old
  expected-failure property is removed together with its validated marker. Catalog719 remains
  valid, with only017 metadata changed. Generated3197 routes/57 files current.
- All275 frozen artifact hashes verify after the complete consumer regression/checkpoint sequence;
  freeze released. Native731PASS/1unchangedSKIP, six C controlsPASS, complete buildsPASS.
- No new full719 smoke, full native aggregate, sanitizer, visual-reference, other-platform or
  live online-service claim. Last complete cross-subsystem checkpoint remains075 and retains its
  six separately tracked CTest failures. Hardware/service validation is not inferred from fake tests.

Next read-only intake025/085: offline writer streams are not enabled, stream values are skipped by
ColumnsToJson/JsonToColumns, and the persistence hook runs only when rating changes. The existing
stream owner is PropertyDictionary; use that owner when restoring persisted streams, with reader
read-only versus writer writable behavior and bounded decoding.025 must account for post-rating
column writes and retained stream/reference writes at the documented commit boundary; a callback
before SetValue or merely enabling streams would not fix both root causes. No025/085 code changed.

Validated source repair: `2ac45d2f06b8f3fe7e8a38d8419919ad9bf0ff16`. Permanent017 disposition FIXED_VALIDATED;156 historical issues now127 final/29 without.
