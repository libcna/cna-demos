# ISSUE-051: C host-run completion after a callback failure

Resolved by CNA `9b0f481f554c2961d6bbe15d1d445b045b49a745`.

## Baseline and contract

Source base CNA `82a93ae97`, demos `4e9f461`, Sharp Runtime `2f644cf47`, with the
ISSUE-050 route-policy candidate. No ISSUE-051 implementation or test change yet.
The frozen complete `repair-owner-full` runs freshly reproduce the unchanged standalone
`c_api.game.host_failure`: **65 passing and eight failing assertions out of73** on each
of SOFTWARE and OPENGLES3. Parent JSON checks are zero for this standalone child.

`CNA/C/runtime.h` explicitly promises that `cna_game_run_frame_ext` completes a failed
run by calling `exiting`, then `end_run`, reports CALLBACK and sets out_running false.
The native Game already invokes OnExiting and EndRun. The C adapter's Invoke guard
suppresses the two C callbacks once callbackFailure_ is set. UnloadContent separately
opts into runAfterFailure, so it still runs. This is a C extension contract defect;
it does not establish a different genuine XNA exception/lifecycle contract.

| Failure source | Observed callback trace |
|---|---|
| second Update | ILBUDUZ |
| Initialize | IZ |
| LoadContent | ILZ |
| BeginRun | ILBZ |
| BeginDraw | ILBUPZ |
| Draw | ILBUPDZ |
| EndDraw | ILBUPDAZ |
| Exiting | ILBUXZ |
| EndRun | ILBUXEZ |
| UnloadContent | ILBUXEZ |

I=initialize, L=load, B=begin run, U=update, P=begin draw, D=draw, A=end draw,
X=exiting, E=end run, Z=unload. All other existing checks pass: reported failure,
diagnostic, terminal state without replay, handle release and exactly-once unload.

## Repair constraints and next validation

Keep the native Game loop and ordinary callback suppression unchanged. Deliver the
promised completion hooks after a failure and preserve the first callback diagnostic.
Simply enabling those hooks is insufficient: RecordCallbackFailure currently replaces
the error, and a cleanup callback may itself fail or call another failing C route.
The enclosing call and subsequent terminal/destroy calls must report the original game
failure even if the thread-local error was changed by another operation.

Add a native public-C regression for completion order, exactly-once cleanup and first
diagnostic retention, including multiple failing cleanup hooks and a successful cleanup
hook that invokes a refused C API. First demonstrate failure against the unchanged
adapter. Then repair locally, run the native lifecycle/dispatch controls and the original
73-assertion demo on both renderers before removing its marker. The existing external
matrix already exercises all ten failure positions; preserve every assertion.

The ISSUE-050 checkpoint completed and was published as CNA `e2074613f`, demos `77346b7`.
Its275 artifact hashes matched after both sweeps and CTest, releasing the freeze before this work.

## Native red witness and candidate

`HostDrivenRunSmoke.c` now preserves its normal/failed-run controls and adds six cleanup cases:
an Update failure with successful cleanup, with all cleanup hooks failing, and with successful
cleanup that calls a refused reentrant C API; then first failures in Exiting, EndRun or Unload.
Every case checks the exact completion sequence, no terminal replay, release of the handle and
the original diagnostic. A refused Boolean setter deliberately overwrites TLS between host calls.

The corrected baseline (`repair-host-cleanup-before-v3`) has five FAIL and one PASS; the passing
control first fails in Unload after otherwise complete cleanup. Earlier witness versions did
not zero-terminate their diagnostic byte-copy buffer, which made message comparisons unreliable.
That test bug was corrected before any production edit; all earlier logs are retained.

The candidate changes only the C adapter: Exiting and EndRun opt into the existing
runAfterFailure policy, the game retains the first normalized callback message, and run/destroy
routes republish that failure after completion. Secondary callback failures do not replace it.
The native Game loop and ordinary callback guards are unchanged. No ABI signature changes.
Strict native build passes; six native lifecycle/dispatch CTests pass SOFTWARE in0.76s and
OPENGLES3 in13.16s. The complete consumer build passes without diagnostics. The unchanged external
scenario passes all73 child assertions on both renderers. Exact assertion-name/order comparison
with the050 full checkpoint preserves every check and removes exactly the eight former failures.
Only after those passes is marker051 removed and its now-inaccurate restriction sentence retired.
The final consumer rebuild passes. Catalog719 has zero problems and changes only those two
metadata fields. All275 consumer artifacts are frozen in repair-host-cleanup-full-artifacts.sha256.
The complete SOFTWARE sweep finished719 IDs:646PASS/22FAIL/48SKIP/3NOTRUN. Exact comparison
with050 changes only051 and removes its eight failures (48->40 failed assertions). All other
statuses, markers, check counts, result messages and failed-assertion multiplicities are unchanged.
OPENGLES3 also finished719 IDs:688PASS/21FAIL/7SKIP/3NOTRUN. Its exact comparison changes only051
and removes the same eight failures (47->39). All275 hashes match after both full sweeps.
Final serial CTest (647.07s) has735 registered/732 enabled tests:678PASS/48SKIP/six unchanged FAIL and
three disabled. Exact per-test status comparison with050 is DELTAS[]. All275 artifact hashes
match after both full sweeps and CTest; the freeze is released. The unchanged residuals are
c_api.content.content_manager, c_api.graphics.resource_lifecycle,
content.content_manager.read_asset_ownership, content.content_manager.several_managers,
content.content_manager.unload_disposes and graphics.occlusion_query.visible_pixels.
This closes the051 regression, not the remaining campaign. No new visual, sanitizer,
full native corpus, browser or additional-platform validation is claimed.

Reports are retained under `build/linux-full/reports/repair-host-cleanup-*`.
