# ISSUE-032: preserve a System Link join refusal

Repair: CNA `f593368783b652164035a96a03f66545f6c9e3fe`, 2026-10-22.
Base: CNA `824851ac3`, demos `82c90be`.

The original demo correctly distinguishes an existing game that forbids joining in
progress from a missing host. Genuine XNA's `NetworkSessionJoinError` reference
documents precisely this case as `SessionNotJoinable`; CNA's online binding already
makes the distinction. The System Link host instead disconnected with data zero,
and `EndJoin` converted every unsuccessful welcome to `SessionNotFound`.

The fix carries a dedicated ENet disconnect reason, records it only from the
upstream host before the initial welcome, and captures it before failed-join
cleanup. Unknown/zero reasons retain the old fallback; established-session ends
are not join refusals. No packet layout, public API, wait or retry policy changes.

## Red baseline

`repair-museum-issue032-before-*` in `build/linux-full/reports`:
native build PASS (84249 ms), the strengthened existing host-refusal test FAILS
with actual disconnect data 0. This failure is retained, not suppressed.
Peak 1378267136 bytes; all memory events zero; no source drift.

The preceding `repair-museum-issue033-final-*` checkpoint runs the original demo
on SOFTWARE/GLES: both fail the exact JoinError assertion (SessionNotFound) and
the host's propagated child-exit check. The late-join Playing assertion already
passes due to the separate ISSUE-033 repair.

## Candidate validation, before removing knownIssue

`repair-museum-issue032-after-*`: CNA net build PASS (33221 ms), **549/549 native
network tests PASS** (46500 ms). All old 547 tests remain; two new regressions
cover pre/post-welcome handling, teardown, zero and unknown disconnect reasons.
The strengthened host-refusal test now passes.

Demo rebuild PASS (63554 ms). Six scenarios on SOFTWARE and OPENGLES3, **12/12
PASS**: `join_refused`, `find_and_join`, `host_leaves`, `add_local_gamer`,
`game_flow`, `private_slots` under `net.network_session`. Relative to the preceding
checkpoint, only both `join_refused` statuses/exits change FAIL/1 to PASS/0.
Every other status, check count, knownIssue marker and exit code is unchanged.

In both standalone child logs, the exact SessionNotJoinable, base-first exception
serialization, late-join Playing and host child-exit assertions pass. The wrapper
reports zero checks for standalone programs, so the child evidence is essential.
No demo assertion changes; only the now-obsolete restriction/knownIssue is removed.

Peak 3037581312 bytes; all memory events zero; no source drift. Changed translation
units have no warnings; existing unrelated native-test nodiscard warnings remain.

Final marker revalidation (`repair-museum-issue032-final-*`): **549/549 native PASS**
(46502 ms), demo build PASS (3121 ms), **12/12 external PASS**, **2/2 catalog CTests
PASS** (6055 ms). Outcomes, check counts and exits remain unchanged; only the two
join_refused reports lose the resolved knownIssue marker. Peak 1277517824 bytes,
all memory events zero, no source drift.

## Isolation and scope

Existing `cna/cmake-build-multi` and `build/linux-full`, one compiler,
CCACHE_DIR `/rv/cnaccache`, CCACHE_BASEDIR `/rv`. MemoryHigh 4 GiB / MemoryMax
6 GiB / no swap. Private user/network namespace with a dummy broadcast interface,
owner UID and all capabilities dropped before tests, private Weston/Xwayland.
No host networking changes. No external service, other-platform runtime or
full-CNA-suite claim; no lifetime algorithm changed and no new sanitizer run.
