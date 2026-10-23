# ISSUE-019 timing repair

CNA fix: `f9747ea4964c4aa65cda1d5fa437ec40ab89911a`.

The repair is validated against genuine XNA4 (17 clock checks plus six live control/reset checks),
seven deterministic native regressions, and the original unchanged four demo assertions.
Native runtime:244PASS/4 unchanged SKIP of248, no failures. SOFTWARE719=644PASS/24FAIL/48SKIP/3NOTRUN;
OPENGLES3719=686PASS/23FAIL/7SKIP/3NOTRUN. Against the directory checkpoint, the only changed
scenario is019 FAILED->PASSED, with its knownIssue removed and all four assertions preserved.
Six exact visual references pass. Serial CTest finishes713.22s with678PASS/48SKIP/6FAIL/3disabled
of735 registered tests: exact per-ID comparison gives DELTAS[]. All275 binary hashes match
after validation; the freeze is released. No native Windows, other renderer, sanitizer or full
CNA-native-corpus pass is claimed for this batch.

The six independent external residuals remain c_api.content.content_manager,
c_api.graphics.resource_lifecycle, content.content_manager.read_asset_ownership,
content.content_manager.several_managers, content.content_manager.unload_disposes and
graphics.occlusion_query.visible_pixels. Separate coverage inventory audit failures below
also remain visible. This is one repaired issue, not completion of the campaign.

## Investigation history

Current CNA base9c068f632, demos499a58e5a, Sharp2f644cf47. Directory checkpoint is published
and its binary freeze released. Native pre-repair full runtime:246 tests,238PASS/4SKIP/4FAIL;
all failures are new reset regressions, with the variable-reset control passing.
Candidate edits Game.cpp/Game.hpp only: reset both timing modes, discard the next clock sample
while retaining prior debt, and clear slow history without mutating the current Update argument.
First candidate native runtime validation passes243/247 with the same four capability skips and no
failures. Exact per-ID comparison preserves every old outcome, changes the four new failing
reset witnesses to PASS and adds one passing fractional-debt test. All six reset tests pass.
Both native builds are warning-free. Reports repair-reset-native-{after,final}-build.log,
repair-reset-runtime-{before,after,final}.{log,xml}, repair-reset-runtime-comparison.log.
Complete consumer rebuild36976 passes (967 planned/966 printed steps), without compiler warnings.
Original demo passes all four assertions on SOFTWARE and OPENGLES3: one update after loading,
slow=false. Reports repair-reset-focused-{SOFTWARE,OPENGLES3}.{log,json}; marker still retained.
Single compiler, shared ccache,6GiB cap, existing build directories.
The first consumer rebuild was deliberately interrupted in its own scope (exit130) after
a documentation-only expansion of Game.hpp changed downstream debug line information and
prevented reuse of cached demo objects. The corrected public brief now occupies its original
single line; semantics and native tests are unchanged. The complete restart is recorded in
repair-reset-demos-build-v2.log. The interrupted build log is retained, not a validation pass.

V5 genuine reference additionally observes20 retained whole steps: last Update slow=false,
Draw slow=false, first history0/second historyInt32.MaxValue. This exposes a remaining candidate
edge: adding the retained step count back into CNA's lag counter immediately marks Draw slow.
A seventh native test uses a smaller TargetElapsedTime to expose previously fractional debt
as nine whole steps and checks the Draw flag. Before correction:6PASS/1FAIL, exactly the Draw
flag (repair-reset-debt-before.*). The correction excludes retained reset debt from restoring
the lag counter. Genuine reference v6 now asserts both large-debt observations and passes17/17.
Final native run48764 passes244/248 with four unchanged skips, no failures; all seven reset
tests pass. Reports repair-reset-native-v2-final-build.log, repair-reset-runtime-v2-final.*.
The original demo's marker is now removed after both renderer passes, with all four assertions
unchanged. Consumer rebuild2181 passes (228 planned/227 printed steps), no compiler diagnostics.
Final timestep category5/5 and C runtime controls1/1 pass on each renderer; catalog719 has zero
problems. Complete renderer/CTest comparisons remain pending. No final issue closure yet.

All275 consumer/CNA/C API binaries are frozen in repair-reset-full-artifacts.sha256.
Complete SOFTWARE719=644PASS/24FAIL/48SKIP/3NOTRUN; OPENGLES3719=686PASS/23FAIL/7SKIP/3NOTRUN.
Exact comparison with the directory checkpoint changes only019 FAILED->PASSED and removes its
knownIssue/one burst failure. Every other status, marker, check count and failed-assertion
multiset is unchanged (51->50 SW,50->49 GLES). All275 hashes match after both complete sweeps.
Reports repair-reset-full-{SOFTWARE,OPENGLES3}{.json,.log,-comparison.log}.
Final serial CTest94813 is running; no rebuilding those artifacts until validation ends.
An additional C++→C coverage audit fails on two pre-existing stale mapping rules: the haptic
constructor gained noexcept in974c1ac08 but its signature rule was not updated; the
ResolveExistingAssetPath helper became private inb0b0207e6 but its public inventory rule remains.
Both are in the published base HEAD, independent of the Game repair. This is a recorded separate
audit failure, not a passing gate. Prose export counts pass8/8 against3205 exports.

## Reproduction

Native regressions live in CNA modules/runtime/tests/Microsoft/Xna/Framework/ResetElapsedTimeTests.cpp.
Build `CnaRuntimeTests` in the existing `cmake-build-multi` tree, then run through
`tools/platform/run_gpu_tests_private.sh --exec ./cmake-build-multi/CnaRuntimeTests` with
`SDL_AUDIO_DRIVER=dummy`. The focused filter is `--gtest_filter=ResetElapsedTimeTest.*`.
Use the shared ccache and one compiler under the campaign's6GiB scope limit.

For the genuine reference use the existing `.wine-cna-xna40` prefix and its Framework4
`csc.exe`, `/warnaserror+`, referencing the sample Microsoft.Xna.Framework, .Game and .Graphics
assemblies. Output goes in the existing demos build-probe directory. The runtime must print
the GAC identity/location recorded below, not load the sample DLL next to the executable.
Run ResetElapsedTimeOracle without DISPLAY/WAYLAND_DISPLAY. Run ResetElapsedTimeLiveOracle
through the private display runner, once with argument `control` and once in a fresh process
with `reset`. Never combine the two cases into sequential Game.Run calls in one process.

During the frozen ISSUE-010 directory regression run (CNA4d384865c plus owned directory
candidate; demos27fd84c plus directory assertions), the SOFTWARE game category still fails
the original ResetElapsedTime scenario. Its existing assertions/knownIssue are untouched.

`ResetElapsedTimeOracle.cs` uses actual installed XNA4 through Framework4/Wine. Constructor-free
private clock injection isolates the real public Game.Tick/ResetElapsedTime methods without
creating a window or device. It does not claim Game.Run, initialization, native Windows or
graphics validation. The separate live-game and native checks are described below.

The corrected v3 oracle passes11/11; v4 adds four mutation/order checks and passes15/15.
A fixed reset sets the pending-zero flag and clears both
slow-history counters. With no existing debt, the next Tick performs no Update and advances
past the injected stale clock origin. Existing two-step debt plus a1234-tick remainder survives
the reset; the two updates retain the ordinary166667-tick elapsed time and reset slow history.
A variable reset produces one zero-elapsed update with IsRunningSlowly false.
Reset inside Update leaves that callback's current GameTime slow flag unchanged; the next
already-planned Update sees cleared slow history. Already-planned steps still execute and
the time-reset flag remains pending for the following Tick.

The live Game.Run oracle is now validated in separate fresh processes on the private display:
without Reset, the first post-load frame performs12 updates (2/2 control checks); with Reset,
it performs1 update with166667 elapsed ticks and slow=false (4/4 checks). Actual XNA4 GAC,
Framework4/Wine/DXVK2.6/RADV; not native Windows. Its original200ms load is retained.
The first two live harnesses attempted two Game.Run calls in one process: the second returned
before its witness, even after correcting Draw-count termination. Those failed logs remain;
scenario isolation fixes the harness and changes no CNA/demo assertion. V3 runs each case once
in its own process. Reports repair-reset-elapsed-live-{control,reset}-v3.log and preceding
repair-reset-elapsed-live-xna{,-v2}* logs.

Native tests use the existing platform decorator with a scripted counter, no real sleeps.
The first four give1PASS/3FAIL before repair. A fifth genuine-backed mutation-order regression
has been added; the full pre-repair run confirms all four regressions fail and existing tests
retain238 passes/four skips in aggregate. Reports repair-reset-runtime-before.{log,xml}.

## Reference provenance correction

The sample-bin Microsoft.Xna.Framework.Game.dll is not the actually loaded GAC binary.
An initial disassembly-based assumption put GameClock in Microsoft.Xna.Framework.dll and
suggested a zero-step reset left the old clock origin behind. The first reference program
therefore failed with TypeLoadException; its exact Wine process/debugger were identified and
stopped, without stopping the concurrent demo sweep or another Wine session. A monodis run
against the real mixed-mode GAC binary aborted. These are retained failed tools, not CNA defects.

V2 resolves the clock type from Game's actual field. It passes10/12 and disproves the two
sample-derived clock/catch-up assumptions. V3 corrects those reference-only expectations,
removes the uncontrolled second-Tick wall-clock claim, and validates11 deterministic state
checks. No CNA or demo expectation is changed. Actual Game and Clock both load from:

`C:\windows\Microsoft.Net\assembly\GAC_32\Microsoft.Xna.Framework.Game\v4.0_4.0.0.0__842cf8be1de50553\Microsoft.Xna.Framework.Game.dll`

Reports: build/linux-full/reports/repair-reset-elapsed-xna{,-build,-v2,-v2-build,-v3,-v3-build,-v4,-v4-build}.log.
The actual Runtime4 compiler receives the sample assemblies only as compile-time references;
runtime identity/location are printed and checked separately. Existing prefix .wine-cna-xna40,
DISPLAY/WAYLAND_DISPLAY unset. V2/v3 compile scopes are capped at6GiB and one compiler; the initial
small v1 reference compile preceded the scope wrapper. No live-desktop process is created.

Pre-repair CNA restricts the reset flag to variable steps; the fixed path never reads it.
The candidate follows the measured clock consumption and preserves accumulated debt.
The original four demo assertions remain the regression oracle. This evidence belongs to the
timing batch, separate from the published directory repair.
