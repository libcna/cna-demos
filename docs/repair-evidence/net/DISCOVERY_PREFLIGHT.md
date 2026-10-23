# ISSUE-018 / ISSUE-026: discovery investigation

**ISSUE-026 resolved:** CNA `5c60a2028f0688fa8b728109156bd6921dc4dbe2`.
ISSUE-018 remains under investigation; its notes below are not a closure claim.

## Current026 candidate (after075 was completed and published)

Native witness in TwoProcessLoopbackTest and net_two_process_harness builds without diagnostics.
Against unchanged production538069391, direct-port transport and host-first discovery pass;
search-first discovery fails: the same client's single second Find returns no matching host,
and the ready host subsequently times out waiting for a join. Reports repair-discovery-before
record2PASS/1FAIL (9.503s); this is a functional local session reproduction, not a crafted packet.
The readiness command carries only GO, with no port. The client performs no retry or restart.

The candidate makes the existing private SearchSocket available on all native platforms and
polls it while maintaining the shared query responder. Darwin's interface-broadcast fallback
stays platform-local. Wire format, search window, query destinations and session policy do not
change. Existing low-resource fallback behavior remains for separate018 review. Native rebuild
passes. Complete native networking passes551/551 in48.278s, with exact parity for every
preceding075 test and exactly two added passing discovery controls. Report repair-discovery-native
includes the existing museum migration and join-in-progress regressions. Full consumer rebuild
passes without diagnostics. All24 external C++/C network scenarios PASS on SOFTWARE and all24
PASS on OPENGLES3; exact comparison to075 preserves statuses, markers, check counts, messages
and every logged assertion's multiplicity. Native CApi_NetSmoke and GeneratedRoutesSmoke PASS.
The719-entry catalog is identical; validation has zero problems, and all275 frozen consumer
hashes match after the suite. Reports repair-discovery-* retain all stages. No demo oracle or
knownIssue marker changed (026 had no runtime marker). No new full719 sweep, full aggregate
native suite, visual, sanitizer, browser, external-service or other-platform runtime claim.
The last complete cross-subsystem checkpoint remains075 with its six explicit CTest residuals.
The026 freeze is released; the repair and permanent disposition are recorded with its SHA above.

## Historical preflight during075

Read-only review during the frozen ISSUE-075 regression. CNA base `6767bda37`, demos
`edc44ee`, with only the075 packet-copy candidate in progress. Do not count this as a
runtime reproduction, disposition or completed repair for018/026.

## ISSUE-026: search before a host starts

ENetDiscoveryService still creates its private ephemeral SearchSocket only under
`__APPLE__`. Linux FindSessions sends queries and receives unicast replies through the
shared discovery socket on61190. The historical bind-order failure therefore still has
its recorded source cause; the museum's host-migration fixes are a different issue.

The current native suite passes549/549, but does not establish this scenario:
TwoProcessLoopbackTest explicitly hands the host's bound game port to the client through
stdout, bypassing discovery. Its comment acknowledges the shared-port weakness. Existing
ENetDiscoveryService tests discover a registered host inside one process.

A regression must use independent processes and a deterministic handshake: complete a search
before creating the host, then announce host readiness and search again in the original client.
Use ordinary CNA discovery/host APIs and the existing process harness where practical. Do not
pass the actual game port to the client as its discovery result, restart the client to avoid
the defect, or use sleeps/retries to hide bind order. Preserve normal host-first discovery and
the existing codec/transport tests. No test traffic or port takeover has been performed here.

Planned native witness: parameterize the existing independent-process harness for host-first
and search-first startup. Give the host a unique test gamertag; the client matches that name in
the real public Find result, then joins that listing and runs the existing payload round trip.
In search-first mode the client completes one initial Find and announces readiness, then waits
for a stdin readiness command sent only after the host announces its bound port. The command
contains no port. The second Find is a single ordinary search, not a retry loop. The parent
retains watchdogs and cleans up only its own children. Keep the old explicit-port tests intact.
This witnesses delivery and successful use of the returned listing without fabricating it.

Isolation preflight passes: `unshare --user --map-current-user --keep-caps --net`, bring up
loopback and a dummy cna-test0 with192.0.2.1/24 plus the255.255.255.255 broadcast route, then
`setpriv --bounding-set=-all --inh-caps=-all --ambient-caps=-all`. The child remains UID1000
with all five capability sets zero. No CNA process or network socket was launched. The first
setup attempt omitted --keep-caps and was refused at `ip link`; it changed no host interface.
The namespace vanishes on exit. Run the actual network tests only after075 CTest has finished.

## ISSUE-018: public network failure type

ENetDiscoveryService's socket creation/bind failures and ENetHostHandle's host/client creation
still throw std::runtime_error. Review where public NetworkSession construction/Find translates
them before changing the internal abstraction. The framework exception types actually live in
Microsoft.Xna.Framework.GamerServices, not the Net namespace.

The C firewall already distinguishes NetworkNotAvailableException (INVALID_STATE/STATE) from
its NetworkException base (PLATFORM/PLATFORM), so correcting the native public exception may
also repair C diagnostics without a new C-specific special case. Verify the intended XNA
contract and existing native exception expectations; do not infer the right subtype solely
from the old demo's catch block.

The local official XNA4 XML documentation (`../xna4-decomp/dlls/` GamerServices.xml)
defines NetworkNotAvailableException for an unavailable network connection and its
NetworkException base for a communication failure. The decompiled Windows
NetworkSessionErrorHandler maps KernelReturnCode.NetworkNotAvailable to the former and
NetworkError/PacketQueueFull to the latter. This supports preserving the distinction;
it does not by itself prove that every ENet allocation or bind error has the same cause.
Review the specific public failure before choosing its subtype.

Existing harness role start-hosting-partial-failure lowers its own RLIMIT_NOFILE, then
verifies rollback and successful creation after restoration, but catches any std::exception.
It is useful lifecycle coverage, not an existing exception-contract assertion. A process-local
descriptor limit avoids interfering with other agents' sockets. The regression for the
historical exclusive-bind conflict still needs isolated networking if exercised literally.

Any bind-failure test must isolate its own resources and preserve other agents' sessions.
No source, native test, demo assertion, marker or permanent issue status was changed by this
preflight. Complete075's frozen checkpoint first.
