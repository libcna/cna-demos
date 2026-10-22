# ISSUE-033: System Link join in progress

Validated 2026-10-22 against CNA `28145a2017fd2421ce2554772923b96a31c1ce36`,
demos `adbeeaf` before this documentation-only closure. Repair:
`04fb4978ae6d3304442c3899bf7a3746f3d77d85` (MUSEUM-NET-002).

The host previously omitted the current session state from its welcome. The repaired
codec carries Playing while retaining legacy Lobby bytes, and the joiner applies the
state after establishing the roster. It raises a state transition once; the existing
XNA-compatible GameStarted subscription replays Playing for a later subscriber.

Permanent native tests cover state round trips, legacy welcome compatibility,
invalid state rejection, a host already Playing, event ordering/non-duplication,
and a two-process public Join returning Playing with one GameStarted replay.
These tests were added with the repair, not replaced with this closure.

## Revalidation

One worker, shared ccache `/rv/cnaccache` (`CCACHE_BASEDIR=/rv`), existing
`cna/cmake-build-multi`; private Weston/Xwayland and private user/network namespace,
dummy interface `cna-test0`, owner UID and all capabilities dropped before tests.
MemoryHigh 4 GiB / MemoryMax 6 GiB / no swap. No host network changes.

Evidence prefix: `build/linux-full/reports/repair-museum-issue033-baseline-*`.
Native build passes in 362378 ms; **547/547 network tests pass** in 46486 ms.
Source drift zero; peak 2062786560 bytes; all memory events zero.

Six original scenarios run on SOFTWARE and OPENGLES3:
`join_refused`, `find_and_join`, `host_leaves`, `add_local_gamer`, `game_flow`,
and `private_slots` under `net.network_session`. Ten runs PASS; both `join_refused`
runs still FAIL due to the independent ISSUE-032. In both failing logs:

- `the JoinError is SessionNotJoinable` still fails (actual SessionNotFound).
- `the joiner is in the game in progress (SessionState Playing)` **passes**.
- The host's child-exit assertion fails because the child retains the ISSUE-032 failure.

The standalone wrapper reports zero checks, so its aggregate status alone is not
evidence for ISSUE-033. The child assertions above are the issue-specific evidence.
No assertion, wait, retry, tolerance or knownIssue marker changed. Only stale
restriction/comment text is updated. ISSUE-032 remains open, individually visible.

Final metadata revalidation (`repair-museum-issue033-final-*`): native 547/547 PASS
again (46350 ms), demo rebuild PASS (6764 ms), the same twelve scenario outcomes,
check counts, issue markers and exit codes. Both child Playing assertions pass and
the exact two ISSUE-032 failure labels remain. Native outcome comparison is identical
for all 547 tests. Two catalog CTests pass after rebuild (8.43 s).
Source drift zero; peak 1485955072 bytes; all memory events zero.

Museum independently reported 23 passed / 0 failed in its network reproducer at
CNA `5b6aab981`. That corroborates the root cause but does not replace the original
cna-demos reproduction. No online-service, Windows, macOS or browser runtime pass
is claimed here.
