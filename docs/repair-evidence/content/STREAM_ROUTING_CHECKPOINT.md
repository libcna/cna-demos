# ISSUE-012 validated repair checkpoint

2026-10-20. CNA runtime `d7ce398d8`, demo marker/evidence `dc9bfac`.
ISSUE-012 is resolved. This is a locally validated repair, not a globally green
campaign checkpoint: the separate C API RootDirectory contract conflict remains.

## Repair and preserved behavior

Generic Load, move-only SoundEffect loading and erased XNB references now honor
virtual OpenStream. Standalone resource managers register the required reader
family without constructing a Game or pulling in unrelated families. Explicit
factory precedence/withdrawal and deliberate re-registration remain protected.
CNA's lower file tiers remain available only for the base missing-XNB condition;
override, malformed-stream and I/O failures do not silently fall back. The strict
stream-only extension retains its behavior. No public API or exception subclass
was added. `STREAM_ROUTING.md` preserves design and before/candidate evidence.

## Native evidence

Genuine cold XNA resource oracle14/14 passes without Game or a graphics service.
The ordinary virtual-stream contract is also covered by the genuine ownership
oracle. Native before failures are preserved;20 focused regressions pass normally
and with ASan/UBSan/leak detection on the five actual production translation units.
Linked dependency libraries are not all instrumented. The original528-byte SDL
foreign-thread fixture leak is retained; documented fixture-owned cleanup fixes
it without suppression. Canonical native tests use the existing Headless seam.

Canonical native build:1205 printed steps,51 exactly matched prior warnings and
no new diagnostics. Native20 focus passes; module1985=1981PASS/four unchangedSKIP;
pipeline559=558PASS/one unchangedSDK-fixtureSKIP (852.057s); consumers1412=1408PASS/
four unchangedSKIP; runtime241=238PASS/three unchangedSKIP; default XML4PASS/two
fixtureSKIP and original XML6/6PASS. All six link-footprint CTests pass, no skips.
Exact case comparisons, nine source hashes and four native artifact hashes pass.
The first canonical configure refusal for native SDL calls remains documented;
the platform audit was not relaxed.

## External evidence

The large build was deliberately rescheduled from one to three workers after native
gates freed capacity. Original exit2/interruption log remains; no compiler failure
was retried. Both segments reconcile566+484 printed steps,1040 unique body tasks
and exactly six repeated independent asset tasks. Zero warnings, nine source hashes
unchanged. The subsequent marker rebuild completes10 steps without warnings.

Both initial and marker-aware146-case focus audits pass:67PASS/six unchangedFAIL
per renderer. Only the two012 demos improve. Stream-override keeps all four checks;
cold resources rise from two to four because two existing success-branch assertions
now execute. Both markers were removed only after proof. Cold-process isolation,
every behavior assertion and all tolerances remain unchanged.

| Renderer | PASS | FAILED | SKIP | Manual |
|---|---:|---:|---:|---:|
| SOFTWARE | 624 | 44 | 48 | 3 |
| OPENGLES3 | 663 | 46 | 7 | 3 |

Each complete inventory contains all719 IDs exactly once. There are67 intended
status improvements versus campaign-start HEAD,69 versus Complete1128. Exact
comparison with the preceding089 checkpoint allows only the two012 improvements;
all other status/check/marker/failure multisets remain unchanged. Both95 math
sets, metadata/tool checks and six exact visual references pass, with zero pixel
differences and no relaxed tolerance.

Final serial CTest:735 inventory,732 enabled;684PASS/48 unchangedSKIP/three disabled,
658.42s. Every735 outcome and319 artifact hashes match the required comparison.
The original-baseline audit still reports exactly the C root-contract assertion
replacing the old103 witness and explicitly sets globallyAccepted=false. Overall
driver exits1 for that retained conflict; every independent gate passes.048 remains
the last globally accepted checkpoint, and039/045/043/103 are not closed here.

Reports: `build/linux-full/reports/repair-content-stream-routing-*`, particularly
the focused audits, both full comparisons, predecessor comparison, final CTest
audit, final-gates summary and frozen artifact manifests. The next boundary batch
has separate native binaries/reports; none of its uncommitted sources were rebuilt
into these frozen external artifacts. No push or merge.
