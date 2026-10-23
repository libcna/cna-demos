# ISSUE-085: offline leaderboard stream persistence

Baseline: published CNA `108fc0e46be3024e815f9f83c3636ac100e3ffe7`, demos
`1a978376acd795bed3efb3ae2e3cf2b3c75a9a9d`. The017 checkpoint has native
732 registered /731PASS /1 declared rendererSKIP and35 gamer scenarios per renderer,
33PASS /1FAIL(085) /1 manualNOTRUN. Last complete cross-subsystem checkpoint remains075;
this batch does not claim its six CTest residuals are repaired. Shared worktree foreign changes
remain untouched.025 has a separate write-timing root cause and stays open.

The public PropertyDictionary header promises dictionary-owned writable streams for writers and
read-only streams for reader entries. Offline GetLeaderboard never sets createsStreams_, and
ColumnsToJson explicitly skips Stream*. Enabling creation alone would silently lose bytes.
Available genuine Microsoft XNA Windows PropertyDictionary.GetValueStream is a
ProFeatureNotSupported stub, as are the other members. This repair implements CNA's stated offline
contract; no live Xbox behavior or Xbox size limit has been measured. CNA already bounds service
streams to256 bytes and documents that as its own rule. The same explicit bound is used offline.

Four new regression witnesses fail before production changes: three throw map::at on first
stream access; the malformed-store test preserves its scalar but fails to restore the valid
stream.4FAIL /0PASS,0.001s. Reports: repair-leaderboard-stream-before-native.{xml,log}.
First compilation found two fixture errors caused by ReadOnlyCollection's mutable element proxy;
explicit entry copies and const collection views correct the tests. The second build passes.
No old test, production contract or failure policy changed to obtain this red evidence.

Candidate repair enables offline writer streams before loading columns, writes bounded byte
contents as hex, and restores owned streams through PropertyDictionary. Writer reloads use an
expandable MemoryStream; reader snapshots remain read-only. Invalid or oversized encodings are
ignored per existing corrupt-column read policy, preserving valid neighboring columns. Oversized
writes throw before file replacement, and the existing Rating rollback retains the prior value.
Serialization preserves position and uses MemoryStream.ToArray, including after Close, matching
the existing service path. No serialized address, new public API, field layout or callback owner.

Coverage also checks an empty stream, growing a restored writer, an entry copy surviving reader
disposal/destruction, prior-reader snapshot independence, the256/257-byte boundary and recovery,
malformed hex/shape/oversize data. A fifth test covers committing a closed stream. This fifth
case was added after the four red witnesses and is not claimed as separately run before repair.

Demo preflight found two secondary assertions hard-coded to eight columns because the broken
implementation omitted Replay. Once Replay persists there are exactly nine columns; Keys/Values
and CopyTo must check that exact shape, including Replay and outer sentinels. Retain the original
stream assertion and all other controls; document those justified expectation changes explicitly.
Do not remove085 metadata until the repaired original reproduction passes on both renderers.

Full native validation:737 registered /736PASS /1 unchanged declared rendererSKIP,74.437s.
Exact name/status comparison retains all732 previous outcomes and adds five passes. ASan/UBSan
with leak detection:41 writer/reader testsPASS,0.018s. Eight translation units are instrumented:
store, dictionary, writer, reader, entry, MemoryStream, native test fixture and user-data isolation.
Other native static dependencies and third-party libraries are not instrumented. The first custom
link omitted the canonical WebGPU library search path; restored that path, then v2 passed with
no sanitizer findings and no changed frozen inputs. No suppression/retry of a runtime failure.
Reports repair-leaderboard-stream-sanitizer-v2-* retain commands, hashes and the precise scope.

The writable restoration uses an empty MemoryStream followed by Write/Position=0 so expansion
does not depend on byte-buffer constructor capacity semantics. Current Sharp Runtime's byte-buffer
constructor also expands; it was not a separately reproduced Sharp defect, and no Sharp source
change is made. Readonly construction still copies and owns the decoded bytes.

Six rebuilt native C gamer/boundary controlsPASS0.20s. Complete consumer buildPASS, no compiler
warnings. Native build retains17 pre-existing nodiscard warnings. Generated3197 routes/57 files
are current. Original demo now has21 controls (historical085 recorded17 before later collection
coverage). First focused run:SOFTWARE21PASS, then OPENGLES3 fails only writer Count. The first
successful run persisted Replay, so the next writer correctly restores nine columns before the
old pre-stream assertion expected eight. Move that assertion after stream access and require
exactly nine including Replay on both first and repeated runs. This is a third justified shape
correction, not a renderer defect or relaxed count. Keep the failed first run evidence.
Rebuild and rerun both renderers;085 marker stays until both pass.

Final validation:
- Focused v2 original demo21/21PASS SOFTWARE2.839s and OPENGLES3 1.682s. Only then remove085
  restriction/knownIssue and update description, preserving the behavior assertion and permanentID.
- Both35-scenario gamer/C gamer sweeps:34PASS/1manualNOTRUN. Exact017 checkpoint comparison
  changes only085 FAILED->PASSED; all other34 rows retain status, count, marker, message and logged
  assertion multiplicities.025 is still open despite this category being green.
- Seven registered focused/catalog CTestsPASS16.65s, including paging normally with no
  expected-failure declaration. Catalog719 validates, only paging metadata changed. Generated
  3197 routes/57 files current. All275 frozen consumer executable/library hashes verified after
  the complete validation; freeze released.
- No new full719 smoke/native aggregate/visual reference/other-platform/live-service claim.
  Last complete cross-subsystem checkpoint remains075 and its six residual failures are unchanged
  by this statement. No assertion deleted or tolerance widened; three exact nine-column shape
  corrections and the first repeat-run failure remain documented above.

Validated native source repair: `420a4572a721f750978e496c3cc0b3b34e2e26a2`.
085 permanent disposition FIXED_VALIDATED;156 historical issues now128 final/28 without.
Next025 must persist post-rating changes at the game boundary, including retained streams and
mutable references, without writing unchanged stale cached rows over another writer's update.
