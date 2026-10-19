# Value-semantics repair checkpoint (120/121/122/136)

Validated runtime: CNA `9786d4c93` (including `bf209dcbe` and `eef23e6f8`).
Linux x86_64, private displays, Radeon 780M/Mesa; SOFTWARE and OPENGLES3 only.

All thirteen original C++ reproductions pass under both renderer labels, retaining
their assertions. CurveKey adds an exact wrapped field-hash sum assertion; the
Vector4 mixed reversed-bound control is corrected against genuine XNA, not removed.
The extended C CurveKey scenario passes ten checks. Strict C17/Werror
MathValueHashSmoke, VectorSmoke, MathValuesSmoke, MatrixSmoke and QuaternionSmoke
pass against the actual rebuilt C API DSO (not an overridden shared CNA library).

Both complete 95-demo math sweeps have 68 passes/27 expected failures. Exact
comparison with the campaign baseline finds 23 cumulative expected status changes
and no unrelated status, check-count or failed-assertion changes. Metadata: 719/0;
tool self-tests: 7+4+5 passes. All six visual references match exactly, with zero
differing pixels/channels. Full CTest: all 732 enabled outcomes accepted, zero
failures, 48 skips/three disabled unchanged, 542.11 seconds.

Complete 719-ID SOFTWARE: 580 passes/88 expected failures/48 skips/three manual.
Complete OPENGLES3: 619/90/seven/three. Both have the expected 23 cumulative status
changes. SOFTWARE also has no unrelated failed-assertion difference. OPENGLES3 has
one investigated pre-existing networking assertion difference, detailed below.
Its exact comparator exits 1; that result is retained, not whitelisted or rerun
until green. No previously passing assertion becomes a failure.

Reports are `build/linux-full/reports/repair-values-*`, including the full reports,
their comparison JSON, CTest log, focused runs and visual comparisons. The original
Complete1128 and current-tree baseline remain distinct historical checkpoints.

## Investigated networking delta, not a math repair

`net.network_session.simulated_conditions` still has known issue 030 and fails
its minimum-200-ms check. Its outage assertion changes from fail to pass solely
because the measured unreliable count changes from one to zero. Earlier Matrix
checkpoint counted four; an unchanged focused follow-up counts four again. All
these runs deliver all 24 reliable packets in each phase. Original logs are kept:

- `repair-baseline-parts/OPENGLES3/net.log`: phase 2 unreliable 1/24.
- `repair-115-full-corrected-parts/OPENGLES3/net.log`: 4/24.
- `repair-values-full-parts/OPENGLES3/net.log`: 0/24.
- `repair-030-unchanged-OPENGLES3.log`: 4/24, both original failures reproduced.

The standalone configures simulation on the receiving host, not the sender.
CNA's pre-campaign `01534c2e7f33d67191efe26c4434c874a244bfa3` intentionally moved
simulation to the sender and exempted reliable packets, matching Microsoft's
XNA 4.0 `NetworkSimulator`. CNA's public header explicitly documents that contract.
The receiver therefore does not impose the demo's supposed outage: ordinary
unreliable transport delivery varies. Its zero count is a vacuous outage pass,
not evidence that math changes repaired networking. There are no networking
production changes since the campaign baseline. The follow-up uses runtime
`5a44d2c50`, because the next integer build had already completed; it is not
misrepresented as another frozen `9786d4c93` run.

Existing deterministic `NetworkSimulatorTest` tests pass 7/7. Four existing ENet
integration tests pass 4/4, directly proving receiver settings leave incoming
packets alone, sender loss=1 exempts reliable packets, loss=0 preserves delivery,
and frozen-clock sender latency releases at its exact due time. Logs:
`repair-030-native-simulator.log` and `repair-030-native-enet.log`.
The genuine decompiled simulator also proves latency is a bounded Gaussian around
the configured mean, not a per-packet minimum. The old demo's minimum assertion
is independently incorrect. These facts explain the regression delta; they do
not close 030. Its standalone oracle must still be repaired and validated in its
own batch. No networking assertion or knownIssue marker was changed here.
