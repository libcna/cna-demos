# Lifecycle checkpoint — 2026-10-20

Closes141–144 as FIXED_VALIDATED. All155 historical IDs remain:62 final/93 pending
(30 runtime findings,one verified duplicate,23 documentation corrections,seven
pre-campaign fixes,one demonstrated demo-expectation correction).

Runtime031693a9f (components/hooks/collections) andb086814f9 (launch parameters);
demo oracles/markers61d344e. Later storage5865ae938, Sharp1c0fc9b5/551ced42/82b57541
and the uncommitted Title145 work are NOT in the frozen external binaries.

## Native and focused contracts

Genuine component34, live Game8, collection mutation3 and drawable disposal6 checks
pass. Virtual property hooks raise events from the base hook; explicit managed
component disposal repeats its callback/event and removes membership first.
Drawable disposal invokes UnloadContent even before initialization. Collection
Clear iterates live indices including null entries; Game disposal retains its
original snapshot. Owned components remain alive across the entire virtual hook
or disposal, nested removal, exceptions and re-registration. Public layout/vtable
is unchanged. Launch parsing preserves empty values, skips only actual argv0,
keeps the first parsed duplicate and rejects direct duplicate Add atomically.

Final native focus77/77 (4.770s); actual five-production-TU ASan/UBSan/leak58/58
(1.771s); exact1,412 prior consumers1,408PASS/four unchanged skips (256.700s).
All33 runtime-source suites234total231PASS/three unchanged capability skips
(15.440s). Actual process argv8/8 after five original failures.

Actual rebuilt C ABI RuntimeComponentsSmoke, RuntimeGameSmoke and independent
disposal probe pass; the latter11/11 after four original failures. ldd verifies
the external C ABI library. Both external builds are warning-free. All20 focused
scenarios per renderer give18PASS/two unchanged050/051 failures. The four repaired
demos retain5/8/11/8 checks. Adjacent genuine-backed oracles retain4/20/15 source
assertions; none were deleted. Four markers were removed only after focused passes.

## Full external regression, including a retained strict-comparison failure

Both95-demo math sweeps have93PASS/two unchanged reserved131 failures, exact48
cumulative math-related changes. Both full719-ID sweeps have52 cumulative intended
status improvements over campaign HEAD (54 versus Complete1128, including two
pre-campaign improvements):

| Renderer | PASS | Expected FAIL | SKIP | Manual/not run |
|---|---:|---:|---:|---:|
| SOFTWARE |609|59|48|3|
| OPENGLES3 |648|61|7|3|

SOFTWARE strict comparison has `differences: []`. OPENGLES3 strict comparison
**failed** on one failed-assertion multiset delta in the still-open030 demo,
`net.network_session.simulated_conditions`: its unreliable-outage assertion
incidentally passes with0/24 received, while latency still fails. The comparator
and driver retain exit1; they were not relaxed or rerun until green. All statuses,
knownIssue values, check counts and other failed-assertion multisets match.

The delta was investigated, not hidden. The unchanged demo configures simulation
on the receiver, although the current backend applies it on the sender; unreliable
delivery has no guarantee. Thus zero observed packets is not evidence of a repaired
outage oracle. Full GLES received12/24,1/24,0/24 unreliable packets by phase, while
all reliable phases remained24/24. The independently scheduled serial CTest on the
same frozen executable/libraries received24/24 in all three phases and retained
both original failed assertions. No additional retry was needed for this evidence.
The simulator source blob is identical at baseline and frozen runtime
(`edbf121a7b57c012f45f952817a159b4e12c9f47`); the demo source is also identical
(`bccf709fb0c4db690732040ac097e759ca5d9d1e`).030 stays open for a separately
genuine-backed oracle repair; no network runtime change or closure is claimed.
This checkpoint is therefore NOT described as an exact-zero-delta GLES sweep.

Metadata719/0, tool suites7+4+5 pass. All six SOFTWARE/OPENGLES3 visual references
are pixel-exact, zero differing pixels/channel delta0; the GLES texture capture
was also inspected. No untested renderer/platform claim.

After both full smokes, isolated serial CTest accepts all732 enabled tests:
zero unexpected failures,48 unchanged skips,three disabled,911.46s. The four
repaired demos no longer have WILL_FAIL; unrelated known failures remain explicit.
No campaign SystemLink runs overlapped. SHA256 rechecks confirm the executable,
actual C++ library and actual C ABI library stayed unchanged throughout.

Raw reports: `build/linux-full/reports/repair-lifecycle-*`; the complete CTest
output is also in `build/linux-full/Testing/Temporary/LastTest.log` until its next
run. The network evidence is preserved in this checkpoint and original smoke logs.
The external lifecycle freeze is released after closure. No push or merge.
