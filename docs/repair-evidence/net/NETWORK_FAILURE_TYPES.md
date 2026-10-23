# ISSUE-018: unavailable System Link transport reports the XNA exception

**Resolved:** CNA `494e24d09de742b639d46cc5982a363e338c610d`.

## Contract and scope

Baseline CNA `49b0884f0`, demos `f43caae`. The official XNA4 GamerServices XML in
`../xna4-decomp/dlls/Microsoft.Xna.Framework.GamerServices.xml` defines
NetworkNotAvailableException for an unavailable network connection and NetworkException
for a communication failure. The Windows assembly's decompiled
`NetworkSessionErrorHandler.ThrowExceptionFromResult` maps NetworkNotAvailable to the
derived exception, and NetworkError/PacketQueueFull to the base. This repair concerns
failure to initialize or acquire the required transport sockets, not arbitrary exceptions
from callbacks, codec parsing, address resolution or an online service.

CNA's public Create/Find and Begin/End paths already unwind failed construction/search and
clear their active operation. The low-level ENet allocation and discovery bind/create paths
instead throw std::runtime_error; the private search socket silently falls back to the
unreliable shared port. The C boundary already has the correct distinct mappings:
NetworkNotAvailableException -> INVALID_STATE/STATE, NetworkException -> PLATFORM/PLATFORM.
No C-specific conversion is needed.

## Reproduction before production changes

The permanent native harness has seven new process tests. A child duplicates stdout under
its own bounded RLIMIT_NOFILE until no descriptor is available, then frees exactly the
socket headroom required by the case. This avoids assumptions about inherited file descriptors
and touches no other process's limits or sockets. Cases cover host and client creation,
discovery creation, Find, private reply creation, EndCreate and EndFind. Each restores its
resources and proves a subsequent public Create succeeds, with no registered failed session.
The two Begin/End cases also check that Begin returned and invoked its callback once.

On unchanged production, all seven fail their exception contract (0 PASS / 7 FAIL, 0.656s).
Six report the exact expected failure point using the wrong exception type; the private-reply
case reports no failure at all. Recovery controls complete. Reports repair-net-errors-before.*.
The standalone bind role also recreates the historical exclusive UDP61190 conflict and fails
only the type check; releasing its own blocking socket allows Create again. It runs alone in
a disposable network namespace, so it cannot occupy another agent's discovery port. This role
is retained in the native harness; the seven allocation cases are registered GTests.

## Candidate

ENetLibrary, CreateHost/CreateClient and required discovery socket creation/bind failures now
throw NetworkNotAvailableException at the original point with the existing diagnostic. The
private search socket refuses acquisition/configuration failure instead of returning a false
empty search through a shared port. There is no catch-all conversion, callback timing change,
wire change, retry, sleep, C adapter change or vendor edit. Native build passes without diagnostics.
The original bind case now passes. Complete native networking passes558/558 in48.369s,
preserving all551 preceding outcomes and adding seven passing exception/recovery cases.
CApi_NetSmoke, GeneratedRoutesSmoke and BoundaryDetail all pass (0.11s); the last verifies the
existing distinct native-to-C mappings without adapter changes.

Before rebuilding the consumer, all275 preceding026 artifact hashes were verified. The
unchanged original `net.packet.send_and_receive` demo, with its discovery port deliberately
occupied inside a private namespace, reports FAILED on both SOFTWARE and OPENGLES3:
`exception: Failed to bind discovery UDP socket.` It has zero checks/empty knownIssue and exit1.
The consumer RUNPATH names its own build/linux-full/CNA tree, not the repaired native tree.
This captures the original external failure against the preceding binaries. Only the stale
source comment is updated; the catch, assertions, skip policy and metadata are unchanged.
Consumer rebuild passes without diagnostics and275 artifacts are frozen. The first corrected
SOFTWARE negative-path capture is SKIPPED/exit77 with exactly
`no network: Failed to bind discovery UDP socket.` The driver initially stopped on77 under
set -e. The resumed driver preserves that capture, explicitly checks its status/message/exit,
and requires the same negative-path behavior on GLES plus all ordinary network scenarios to
PASS. This is the original demo's existing declared missing-network policy; no assertion,
knownIssue marker, tolerance or expected result was changed. The resumed validation passes:
the forced unavailable-network case uses that same exact SKIPPED/exit77 contract on both
renderers, while all24 ordinary C++/C network scenarios PASS on SOFTWARE and all24 PASS on
OPENGLES3. Every preceding026 status, marker, check count, result message and logged assertion
multiplicity is unchanged in those ordinary runs. Catalog719 is identical, with zero validation
problems. All275 frozen artifact hashes match; freeze released. No knownIssue was removed
because018 had none. Only the stale explanatory comment changed in the original demo.

All validation is complete; the CNA repair SHA is recorded in the permanent inventory. The next
independent group is achievement collection/indexer semantics (017/077/078), after checking
the genuine XNA read-only and dictionary contracts. No achievement implementation was changed.

Linux runtime only. ENet startup failure on platforms where initialization can fail is source
reviewed, not a claimed Windows runtime pass. No external service, new visual or sanitizer claim.
The latest full cross-subsystem checkpoint remains ISSUE-075; ISSUE-026 adds a validated network
checkpoint preserving all24 external C++/C network scenarios on each renderer.
