# ISSUE-050: owner-only C game routes must not enter the dispatch queue

Resolved by CNA `12837955407ec964ebce9752b1fae0b3e47b6b14`.

## Baseline and contract

CNA base `82a93ae97`, demos `4e9f461`, Sharp Runtime `2f644cf47`.
`CNA/C/runtime.h` permits ordinary foreign-thread handle calls to be served by the game
thread, but explicitly reserves game-driving/destruction and queue configuration/servicing
for the calling owner thread. These are C API extension contracts; no new XNA behavior is
being inferred.

All generated routes previously used `CallWithExceptionBarrier`. A refused handle lookup
marked its THREAD result as dispatchable, so the barrier queued even owner-only operations.
The game-specific reentry guard did not alter that retry policy.

The new native `CApi_OwnerThreadRuntime` regression tests run, one frame, host frame, tick,
destroy, dispatch configuration and queue pumping. With dispatch disabled all seven return
THREAD directly. Before repair, enabling dispatch makes all seven block until cancellation,
then return INVALID_STATE/STATE. The worker is never pumped. A condition-variable deadline
is only a failure-detection bound; cancellation cannot make a case pass. The test also checks
the worker's error record and that the owner can still read and destroy its unchanged game.
The existing ordinary-dispatch control passes on this baseline.

## Repair and focused validation

The route generator now accepts a boolean group `dispatch` policy. Default behavior remains
dispatchable. Owner-only groups choose `dispatch = false` and the existing
`CallHereWithExceptionBarrier`, preserving exception translation and argument/handle checks
without retrying the call on another thread. The formatted-copy shortcut respects this policy
too. The Runtime specification puts exactly the seven owner-only routes in these groups.

An independent comparison of generated function bodies finds exactly seven changes, each
solely replacing the barrier. No exported route, signature or ordinary dispatch behavior changes.
Generated smoke assertions are unchanged; two checks move with their specification group.

- New generator regression fails before the repair; all five generator tests pass afterwards,
  including malformed dispatch values, null-output ordering and deterministic regeneration.
- Generated-file check passes. All 3205 declared routes remain named by tests.
- Strict native build passes without diagnostics. Six native CTests pass on OPENGLES3
  (12.53s) and SOFTWARE (0.96s): the 14 owner-thread combinations, ordinary foreign calls,
  host-driven run, runtime game/components, and the 3197-route generated smoke.
- The unchanged external standalone demo passes all **46 child assertions** on both renderers.
  Exact assertion names and order match the complete ISSUE-019 baseline; exactly the two
  ISSUE-050 failures disappear. Its parent JSON reports zero embedded checks, as expected for
  a standalone child. The initial focus harness incorrectly expected the historical count30
  in that parent JSON; this harness error and the successful child output are retained.
- Only after both focused passes was the knownIssue marker removed. The C source and all
  behavior assertions remain unchanged.

## Complete regression checkpoint

Full consumer builds before and after the marker change pass without diagnostics. Catalog719
has zero problems and differs only in the ISSUE-050 marker. All275 consumer binaries are frozen
in `repair-owner-full-artifacts.sha256` for the full SOFTWARE, OPENGLES3 and CTest comparison.
Both complete smoke runs have finished:

| Renderer | PASS | FAIL | SKIP | NOT RUN | Failed assertions |
|---|---:|---:|---:|---:|---:|
| SOFTWARE | 645 | 23 | 48 | 3 | 48 (was 50) |
| OPENGLES3 | 687 | 22 | 7 | 3 | 47 (was 49) |

Each has all719 IDs. Exact comparison with the previous complete019 checkpoint changes only
ISSUE-050 from FAILED to PASSED and removes its marker and precisely its two failed assertions.
Every other status, marker, check count, result message and failed-assertion multiplicity is
unchanged. Final serial external CTest (652.53s) has735 registered/732 enabled tests:
678PASS,48SKIP,six pre-existing FAIL,three disabled. Exact per-test comparison with019 is
DELTAS[]. All275 artifact hashes match after both sweeps and CTest; the freeze is released.

The six unchanged CTest residuals are c_api.content.content_manager,
c_api.graphics.resource_lifecycle, content.content_manager.read_asset_ownership,
content.content_manager.several_managers, content.content_manager.unload_disposes and
graphics.occlusion_query.visible_pixels. An unexpected pass under an old expected-failure marker
still fails CTest; those markers were not changed here. This is a validated ISSUE-050 repair,
not an all-green campaign claim.

Reports: `build/linux-full/reports/repair-owner-*`. The existing full checkpoint is ISSUE-019;
the intervening ISSUE-150 retirement left all275 binaries byte-identical. The separate019
native smoke fixture correction is `8a21c594d`. No sanitizer or additional renderer/platform
claim is made for this thread-routing change.
