# Video acceptance — C API inventory follow-up

2026-10-21. Reviewed metadata corrections now pass the seven original inventory,
policy and generated-document gates. This is **not** a ready-to-release C ABI or
video runtime closure. The failed five-test run remains in
`build/linux-full/reports/repair-video-xna-postreview-ctest.{log,xml}`:
CompatibilityMatrix, RouteTestCoverage and GeneratedRoutesCurrent pass;
CoverageMatrix and ReleaseGate fail. No test, release criterion, parser rule,
declaration discovery policy or expected-failure marker is relaxed.

## Accepted shared-worktree metadata checkpoint

Native62083 completed first:10,885PASS/101sameSKIP, exact old10,982 outcomes plus
four loop witnesses. Inventory29275 then passed coverage generation and five of
six gates, failing only stale LIMITATIONS.md. After its original generator ran,
33140 passed six of seven gates and exposed stale RELEASE_GATE.md. Both failed
logs/XML remain. Original generators updated only derived descriptions/counts;
release_gate.json, limitations.json, generators and test criteria are unchanged.

Final22151 (`repair-video-inventory-v3-*`) passes all seven in26,436ms and checks
the actual decoder-dependent fixture selector in both TRUE/FALSE modes. This
does not claim a decoder-OFF runtime test. Native inputs remained unchanged;
peak965,607,424 bytes, all memory events zero. The snapshot now reports8,122
symbols,7,011 implemented,15 partial,643 planned,453 not applicable. In particular,
the existing release criterion still honestly says **not ready** for643 planned
symbols; no additional declaration was approved or hidden to pass the gate.

All three generated summaries (COVERAGE, LIMITATIONS, RELEASE_GATE) include
inherited uncommitted ownership declarations. Preserve them and the SoundEffect
pin hunk for that coherent checkpoint; do not sweep them into the video commit.
Runtime video sanitizer/external/C ABI acceptance is still independent and pending.

## Removed content trait

Committed CNA `3d254dfa795f6dd64c921a467b9774e0509c1149` removed both declarations
of `Microsoft::Xna::Framework::Content::Detail::IsDisposableSharedPtr` from
ContentReader.hpp. `git show` confirms their removal; the current actual Doxygen
inventory has no matching declaration. The old `content-reader-detail-traits`
mapping describes a compile-time trait with no C operation and still approves
`CPP-2BFF02F2062A` and `CPP-4EDC13684D0F`. Removing that orphan mapping removes no
existing declaration or runtime test. Its historical content remains in Git.
This correction belongs to that earlier content-dispatch change, not the new
video runtime repair.

## Existing SoundEffect move declarations

Inherited, uncommitted ISSUE-011 changes defaulted inline move operations to
out-of-line declarations with the parameter name `other`. The operation names,
parameter types and noexcept contract are unchanged. Their declaration-derived
inventory IDs therefore change:

| Operation | Previous pin | Current pin |
| --- | --- | --- |
| SoundEffect move constructor | CPP-69D8C13F9A4B | CPP-BC0E945EB845 |
| SoundEffect move assignment | CPP-2646628D6D91 | CPP-57743BD68FD1 |

The retained pre-repair build/c-api/COVERAGE_FULL.md identifies the old signatures;
the actual Doxygen diagnostic reports the new declarations. The existing
`sound-effect-move-operations` rule still applies: C owns stable handles and
offers no move operation. Its pattern, status, tests and the two SoundEffectInstance
pins are unchanged; it gains no additional operations. Only the two SoundEffect
pins are replaced. No content/audio implementation or inherited test is edited.

**Commit boundary:** these two pin changes depend on uncommitted ownership work.
Do not sweep that work into the video commit, or commit these pins as if their
declarations already existed in HEAD. Preserve this mapping hunk for the coherent
ownership checkpoint. Likewise, do not claim the shared worktree is clean.

## Video contract descriptions

The `media-video` and `media-video-player` rules still match exactly their original
declarations and retain every approved symbol, status and task. Only descriptions
and test-evidence text are corrected for the owner's XNA decisions: fresh-frame
refusal, atomic Volume range refusal, Stop retention, URI resolution and genuine
ArgumentException/INVALID_ARGUMENT for deferred missing files. Existing invalid
codec/native Windows timing limitations remain explicit. The mapping text does
not itself prove runtime acceptance; that requires the actual video gates.

## Earlier pending validation plan (superseded by accepted checkpoint above)

Continuation review at18:04 CEST: native62083 is authoritatively live and has
advanced from successful-build differential cases to XnaErrorParityTest. No
second heavy job has started. The prepared extra ABI driver now requires the
future accepted loop external audit and the actual six-gate inventory audit,
rather than the failed84138 checkpoint. It pins inventory inputs independently
of the later intentionally rebuilt sanitizer/external libraries. Syntax check
passes; the ABI driver is still UNRUN. Run inventory first after62083, then
sanitizer53, external112 and the additional ABI checks serially.

The unchanged lightweight policy suite `python3 tools/c-api/test_coverage_scope.py`
passes33/33 in0.800s against the reviewed mapping. It is not Doxygen inventory or
release validation. Mapping SHA256 at that check:
`f57621dadb5d98bc9ca93799a8a00ca923068e20eadb2e2956fd8ffef0229618`;
unchanged policy-suite SHA256:
`9eb34f5a18d4d35d5cb55ea0388abb2f3bc518843a3a1e732328e7263e1e84c2`.

After live native62083 finishes, rerun the actual inventory. Review every new
diagnostic separately; do not use blanket approval or expand patterns to absorb
unknown declarations. Regenerate the derived coverage summary with the existing
generator only after declaration validation passes, then rerun all five original
C API gates. Keep the original failed log/XML. Regeneration based on inherited
dirty declarations is shared-worktree evidence, not a clean-HEAD claim, and must
not be accidentally committed without the matching ownership checkpoint.
