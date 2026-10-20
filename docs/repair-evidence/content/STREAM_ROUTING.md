# ISSUE-012 stream routing investigation

2026-10-20. Issue remains open; CNA repair committed `d7ce398d8`, full validation pending.
Pre-repair native binary revision: CNA `5d32ca58b` (status HEAD `56dca0b43`).
Permanent fixtures: `ContentStreamRoutingTests.cpp`, `ResourceContentColdOracle.cs`;
ordinary virtual-load/cache evidence also exists in `ContentOwnershipOracle.cs`.

## Contract and root causes

Genuine XNA cold ResourceContentManager:14/14, no Game or graphics service. The authored
Curve loads with exact values/loop modes, cached resource identity and one provider
lookup; Unload reopens/redecodes; a missing resource throws ContentLoadException and
does not damage the successful cache. Native Curve's by-value wrapper is not falsely
asserted to have managed reference identity.

Current CNA ReadAsset already registers its builtin family. Its registration was a
no-op without Game's declaration. Separately, ordinary Load, its SoundEffect move-only
specialization and erased external-reference loading bypassed virtual OpenStream.
These are two causes, not a missing duplicate registration call in ReadAsset.

## Before and focused candidate evidence

Original native four cases:onePASS/threeFAIL. Expanded eight cold/routing cases:
twoPASS/sixFAIL. Additional eight exception/fallback/SoundEffect/erased-reference
boundaries:twoPASS/sixFAIL. All strict builds use Wall/Wextra/Werror. An initial
expanded fixture build omitted Register's manager argument and failed before running
any test; its log is retained alongside the corrected baseline.

Cold-only candidate: sixPASS/two unchanged ordinary-routingFAIL, normal and actual-two-TU
ASan/UBSan/leak. Combined candidate:16/16 normal and actual-five-TU sanitizers. Expanded
20 controls add different-asset/foreign-manager exception boundaries, nested loading
and synchronized simultaneous managers. Normal20/20; the first expanded sanitizer run
also passes all20 behavioral assertions but exits1 for528 bytes of SDL TLS allocations
at foreign-thread exit. The installed SDL_thread.h explicitly requires SDL_CleanupTLS
for non-SDL-created threads. The fixture now performs that cleanup for its owned threads
on SDL3; no sanitizer suppression, disabled leak check, assertion change or retry policy.
Final normal and ASan/UBSan/leak runs both20/20 with no diagnostics. Linked native libraries
are not all instrumented; the five actual production TUs and fixture are instrumented.

## Candidate design and protected extensions

Standalone family registration does not declare/link unrelated readers. First explicit
factories retain priority. Lazy registration preserves withdrawn names even when a later
custom graph needs all families; explicit registration or a new Game restores them.

Every compiled-load path honors the virtual stream. CNA's existing lower-tier precedence
remains available only for the base implementation's own missing-XNB exception, identified
by exception identity in an owner/name-matched, scoped thread-local attempt. Custom,
translated, foreign-asset and foreign-manager exceptions do not authorize fallback;
null streams, invalid headers and I/O failures do not either. Nested calls restore the
outer attempt. The explicit stream-only option retains its stricter missing-file behavior.
No public exception subclass or API change is introduced.

Twenty native regressions are installed. The obsolete native assertion that no Game
means no builtin registration is corrected using the genuine14-control evidence, while
retaining family isolation and declaration controls. Canonical native gates now pass;
neither external012 witness nor its knownIssue marker has been changed. Global C API
RootDirectory contract blocker is separate and remains unresolved.

The first canonical build stops at the strict platform audit before compilation:
native cleanup calls do not belong in the content test layer. No budget/ratchet is
relaxed. The permanent native concurrency case instead uses the existing Headless
platform and ScopedCurrentPlatform seam, preserving all20 tests and every assertion.
Its actual-five-TU sanitizer run passes20/20 without diagnostics. The evidence fixture
retains separately validated default-SDL integration and its required foreign-thread
cleanup. Native nonproduction audit remains81 classified files/1050 references and
passes unchanged; the canonical build resumes with a new source hash manifest/log.

## Canonical native checkpoint

The resumed native build completes1205 printed steps with51 precisely matched
pre-existing warnings and no new diagnostics. Nine source hashes match; four
artifacts are frozen and checked before/after every gate. Canonical20 focus passes.
The1985-case content module gives1981PASS/four unchangedSKIP and preserves the exact
1965-case predecessor except the measured oracle rename and20 new regressions.
Pipeline559 gives558PASS/one unchangedSDK-fixtureSKIP in852.057s. Consumers1412
give1408PASS/four unchangedSKIP; runtime241 gives238PASS/three unchangedSKIP;
default XML gives4PASS/two unchangedfixtureSKIP and original XML fixtures6/6PASS.
All six link-footprint CTests pass in3.24s with no skips. Every exact audit has
errors[] and all four post-gate hashes match. SOFTWARE/OPENGLES3 demos validation
and full external regression remain pending; this is not issue closure.

External build initially shares the three-worker budget with native gates. Once
those finish, the single-worker Ninja is deliberately interrupted at563 completed
body tasks (566 printed steps), without any compiler failure, and resumed with
three workers. Both logs are preserved; the final audit must reconcile1040 unique
body tasks, the initial configuration prelude and exactly six repeated independent
asset-copy/content tasks. Nine source hashes remain unchanged throughout.

The complete two-segment external build now passes:566+484 printed steps,
1040 unique body tasks and exactly six repeated asset tasks, zero warnings,
nine source hashes unchanged. Initial146 focus passes its exact audit:67PASS/six
unchangedFAIL per renderer. Only the two012 witnesses improve; open_stream_override
retains4 checks and resource_content_manager rises2->4 because its existing success
branch now executes. The repair is committed after this proof. Only those two
knownIssue markers are removed; every assertion and resource cold-process isolation
is preserved. Ten-step marker rebuild is warning-free; marker-aware focus/full
regression are pending. No new global acceptance or issue closure is claimed.

Reports: `build/linux-full/reports/repair-content-stream-routing-*`, particularly expanded
before/cold-candidate/routing-candidate audits, retained `*-sanitizer-context.log`, final
`*-tls-cleanup-*` audits, and `repair-content-resource-cold-xna-*` reference evidence.
