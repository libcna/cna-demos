# ISSUE-030 current-state audit — 2026-10-20

Status: ALREADY_FIXED before the campaign, now runtime and regression verified.
The permanent issue and earlier failed evidence remain; the marker was removed
only after corrected rebuilt focused executions passed.

## Why the registry checkpoint stopped at the strict comparator

The full OPENGLES3 registry sweep has the intended654PASS/55FAIL/7SKIP/three manual
results, but one unrelated child assertion changes: simulated_conditions' complete
outage check passes instead of failing. The latency check still fails. The strict
comparator returns nonzero; that result is retained, not whitelisted or retried.
SOFTWARE has the original two failures. Neither constitutes a network repair.

The demo configures the receiving host's simulated settings, while its child sends
the packets. Thus its assertion that phase2 must deliver zero unreliable packets
is not actually testing configured sender-side loss. Unreliable delivery has no
guarantee, so its observed count can vary; zero arrivals is not proof the setting
worked. This is an oracle flaw, not evidence that the content repairs fixed networking.

## Contract and existing repair

CNA01534c2e7f33d67191efe26c4434c874a244bfa3 (2026-10-16) predates and is an ancestor
of campaign baseline688f3f23e. It moves simulation to the sending machine, exempts
Reliable packets from loss, and matches XNA's normal latency distribution. Existing
native simulator tests also pin ordering, bounds, random-draw consumption and setters.

The local primary reference `../xna4-spec/Microsoft.Xna.Framework.Net/NetworkSession.xml`
documents outgoing-only simulation, Reliable exemption and a normally distributed
latency rather than a per-packet minimum. The genuine DLL's managed NetworkSimulator
agrees: meanL, deviationL/4, clamped to0..2L. Microsoft's earlier public
[XNA networking documentation](https://download.microsoft.com/download/D/F/6/DF6BE85E-8523-46DF-9DA6-730EABDDFDE4/XNA_Game_Studio_2.0_en-us.pdf)
also documents Reliable exemption and normally distributed latency; the current
decision is additionally verified against the actual4.0 DLL, not inferred solely
from that older document.

`NetworkSimulatorOracle.cs` executes the actual XNA4.0 managed simulator with a
scripted Random:27/27. It checks all eight flag combinations at complete and zero
loss, no random draws for Reliable/zero loss, clock conversion and five exact
normal/clamp cases (one Stopwatch-tick allowance only for floating-to-integer
truncation). It uses private reflection deliberately; it does **not** run a network
session, transport, Xbox LIVE or hardware. Native NetworkSimulatorTest7/7 passes2ms.

## Runtime-validated demo correction

The sender applies each phase's settings before sending; a separate recovery marker
returns that same sender to zero loss. Exact24 reliable deliveries, zero unreliable
deliveries during complete simulated loss, baseline reception and bandwidth checks
are preserved. No extra sleep, retry or deadline extension is added.

The invalid assertion that every packet waits at least the configured **mean** is
replaced by a one-sided statistical smoke check: with at least24 reliable samples,
sample mean >= L/2. For XNA's standard deviationL/4 this lies2*sqrt(24), about9.8
standard errors below the expected mean. Transport/frame queuing adds nonnegative
delay. This is explicitly statistical, not a deterministic minimum guaranteed by
XNA; exact sampling/bounds/order remain protected by native deterministic tests.
The assertion count is unchanged. This threshold has a distribution-based reason,
not an arbitrary wider tolerance chosen from a failed run.

A separate build-probe executable compiled with the actual frozen dependency
closure and Werror; no319-checkpoint binary was rebuilt. After the isolated serial
CTest completed, one two-process run on SOFTWARE and one on OPENGLES3 both passed
all six checks, without competing SystemLink sessions. The seven deterministic
simulator tests and four sender/receiver ENet integration tests then passed11/11
(164ms). Reports: `repair-network-corrected-driver.log`,
`repair-network-corrected-{SOFTWARE,OPENGLES3}.log`, and
`repair-network-sender-receiver-native.{log,xml}` under build/linux-full/reports.
The rebuilt aggregate runner subsequently passed all six checks on each renderer
(13.319s SOFTWARE,9.033s OPENGLES3), then marker3d1f833 removed the knownIssue.
Both full719 sweeps pass exact comparison, with no retry or030 whitelist. Final
isolated serial CTest accepts all732 enabled cases (48 unchanged skips),980.07s;
all319 frozen artifacts match before and after. ISSUE-030 is ALREADY_FIXED by the
pre-campaign commit above, not a new runtime repair attributed to this campaign.
See [complete checkpoint](content/SECTIONS_CHECKPOINT.md). No online-service or
Xbox hardware validation is claimed.
