# ISSUE-057: staging lifecycle repair and retained evidence

## Final repair and validation (2026-10-21)

ISSUE-057 is **RESOLVED / FIXED_VALIDATED** by CNA source commit
`83e611fe1e1196e63ccc708d2aada80e8addf512`. A stable parent OS lock excludes
initialization/teardown from scavenging while preserving independent live builds,
per-run leases and every original scanner diagnostic/safety rule. The public
pipeline documentation explains the persistent empty coordination file, safe
quiescent cleanup and the requirement to rebuild cooperating old tool binaries.

Three new CNA-native regressions pass. The complete native suite has10807 cases:
10706PASS/101unchangedSKIP, every10804 predecessor result unchanged plus three
new passes. Focus134 gives133PASS/one unchanged large-fileSKIP; unchanged context11
and real compiler-host6 cases pass. Canonical and external incremental builds have
zero diagnostics/errors, with one compiler worker and the shared `/rv` cache base.

The original unchanged1000-owner concurrent probe now emits zero diagnostics under
actual-header ASan/UBSan/leak instrumentation. Seven unprivileged permission controls
pass. Separate-process controls pass15 on Linux and14 available under MinGW32/Wine,
including2000 owners plus a scanner, lock exclusion/OS exit release, active-old
retention, abandoned-old collection and unsafe marker refusal. Wine cannot create
the native symlink fixture (`Function not implemented`): that control is explicitly
unavailable, not passed. No native Windows/MSVC, macOS, NFS or SMB runtime claim.

The historical sequential empty-parent fixture is retained byte-for-byte, not
claimed to pass a changed housekeeping contract. Its new permanent counterpart,
`ContentStagingSequentialLifecycleProbe.cpp`, preserves all original active/recent/
closed-scan checks and verifies exactly one empty regular singly-linked owned
coordination file after EACH of1000 lifecycles, with no extra entries. Gate81749
passes strict compilation and actual-header ASan/UBSan/leak execution, zero errors/
diagnostics,1000 exact cleanups; peak168,828,928 bytes,zero high/max/OOM. This is
an explicit contract adaptation with stricter exact-file checks, not arbitrary
leftover acceptance or a suppressed warning. No original fixture was edited.

External29991 completes all18 stages: SOFTWARE719=628PASS/40knownFAIL/48SKIP/
three manual, OPENGLES3719=667PASS/42knownFAIL/7SKIP/three manual. Every status,
knownIssue, check count and failed assertion is identical to047; all36 standalone
labels remain. Both95 math sweeps and six pixel-exact references pass. All735
registered CTests preserve684PASS/48SKIP/three disabled (732 enabled),714.32s.
Scope peak359,165,952 bytes,zero high/max/OOM. No runtime knownIssue marker existed
for057, so none is removed and the known-failure totals do not change:71 improvements
against the initial campaign baseline,73 against Complete1128, on each renderer.

Independent closure15171 passes:10807 native outcomes rechecked through XML,
352 unique current files verified including319 external artifacts, all original
and new fixtures, scanner-body equality, six exact visuals and all validation
comparisons; errors empty. Reports are under `build/linux-full/reports/repair-staging-lifecycle-*`,
especially `-closure-audit.json`, `-sequential-audit.json`, `-native-full-audit.json`,
`-final-gates-summary.json` and `-ctest-final-audit.json`. Instrumentation covers
the actual staging header and fixtures, not every linked CNA dependency.

## Retained candidate-validation chronology

The candidate/running statements below are historical steps superseded by the
completed checkpoint above, not live jobs or unfulfilled057 validation gates.

## Candidate repair and focused validation (not final closure)

The current uncommitted candidate coordinates initialization, scavenger inspection/
removal and teardown using one parent OS lock. Actual content builds retain their
independent per-run leases and are not serialized by the parent lock. A persistent
empty `.cna-staging.lock` file gives independent processes and threads the same
stable lock identity. Deleting/recreating it while a participant may be waiting
would invalidate that guarantee, so production never removes it. This is an
explicit housekeeping contract, not an ignored unexpected file. Private test
harnesses verify its exact empty regular singly-linked current-user-owned identity and
the absence of all other entries, then remove it only after their users exit.

The original scavenger body is unchanged ignoring whitespace; all name, ownership,
symlink, metadata, age, lease, bounded-scan and removal diagnostics remain.
Audit normalized SHA256:
`44f764c3e002252c570ebd1703ff3773881ff257e934a0bc272b2b001d206efa`.
An unsafe parent coordination file refuses reservation and scanning. A destructor
that cannot acquire coordination leaves its valid tree intact for later scavenging,
instead of exposing partial teardown. No PID-based suppression or arbitrary sleep.

Native v2 process fixture passes15 controls; MinGW32/Wine passes14 available ones.
Both exercise2000 independent owner lifecycles plus a concurrent scanner without
diagnostics/removals, aliases, same-process/independent-process OS exclusion,
OS release without C++ destructors, retained active old owners and collection after
exit, nonempty/hardlinked markers and exact cleanup. Linux additionally creates
and refuses a native symlink to an empty target. Wine's native creation reports
`Function not implemented` (exit77): that control is explicitly unavailable, not
passed. Earlier14-control runs are retained; a host-created link to a nonempty
target did not independently demonstrate native Win32 symlink refusal. No native
Windows/MSVC, macOS, NFS or SMB runtime coverage is claimed.

Build12564 passes12 tasks in18,585ms with zero diagnostics/errors. Native focus53714
passes133 cases plus one unchanged large-file skip, including three added regressions.
Unchanged context11/11 and actual compiler-host6/6 tests pass; their assertions
were not modified. Auxiliary54210 runs the original unchanged1000-owner probe
under actual-header ASan/UBSan/leak instrumentation:2030 scans,zero owner errors,
zero diagnostics. `parent_empty=0` is verified to mean exactly the new persistent
empty coordination file, not a leaked reservation. Seven unprivileged permission
controls pass with no effective capabilities. No source changes during the gates.

Full native60712 COMPLETE0:10807=10706PASS/101unchangedSKIP in1,224,981ms.
Every10804 predecessor outcome is unchanged plus three new PASS; independent XML
comparison and frozen source/artifact hashes pass. Peak1,117,966,336 bytes and
zero high/max/OOM events. External incremental build17134 COMPLETE0:10,073ms,
zero diagnostics/errors,one worker,peak1,605,029,888 bytes,zero high/max/OOM.
Focused external12011 COMPLETE0:146 cases,exact parity with047 for status,
knownIssue,check counts and failed assertions; all36 standalone labels retained.
Full external29991 has accepted both719 sweeps: SOFTWARE628PASS/40knownFAIL/
48SKIP/three manual; OPENGLES3667PASS/42knownFAIL/7SKIP/three manual. Exact047
status/knownIssue/check/failed-assertion comparisons have no differences, as do
the original-baseline comparisons. Both95 math sweeps and six pixel-exact visual
references pass. All319 external artifacts match after the sweeps and before
CTest.29991 now runs final serial CTest; final closure audit remains pending. No issue
closure or source commit yet. Reports are `build/linux-full/reports/repair-staging-lifecycle-*`.

## Refreshed baseline after ISSUE-047 (2026-10-21)

CNA `ee92c1eaa95ffad3aec1aa0c9d4c77c0e4fb8407`, demos
`1abd9ec6b14e44f6e2e77739e90a926b8dd4aec2`: both permanent probe sources remain
unchanged. Strict Clang C++23/O2/Wall/Wextra/Werror builds pass using shared ccache
and one compiler worker. One fixed concurrent run gives1000 owners/5640 scans,
zero owner errors/removals,3093 missing-owner warnings,64 unreadable/oversized,
22 incomplete reads and39 vanished/invalid-directory diagnostics. Its normal exit1
is retained. The paired1000-owner sequential control exits0 with zero diagnostics
or errors. Both initially empty private parents finish empty and are removed with
non-recursive empty-directory removal only. No production root is scanned.

Driver66458 COMPLETE0 accepts exactly that failing concurrent baseline and passing
sequential control, verifies unchanged source hashes and records four stages.
Peak141,725,696 bytes,zero high/max/OOM events; no heavy job remains live.
Reports: `build/linux-full/reports/repair-content-staging-after047-before-*`;
driver: `build-probe/repair-content-staging-after047-before.mjs`.
ISSUE-057 remains **CONFIRMED_OPEN**. No runtime fix, changed probe assertion,
sleep, diagnostic suppression or retry has been introduced.

## Earlier retained baseline

CNA `dbfe6b4c1` reproduced the historical warning race. The
unchanged staging header has SHA-256
`76992cf40699e95e8460973213749c8cb423cdda66cbd33a1342682fcc1ba1bd`.
The historical probes are now preserved as `ContentStagingConcurrencyProbe.cpp`
and `ContentStagingSequentialControl.cpp`; neither substitutes a staging
implementation. Both use a driver-created, initially empty private parent and
never scan the shared production staging root.

The single fixed 1,000-owner concurrent run completes with zero owner errors,
zero reported removals, an empty parent and 6,407 scans. It produces:

| Diagnostic | Count |
| --- | ---: |
| Missing/non-regular owner metadata | 3,241 |
| Unreadable/oversized metadata | 225 |
| Incomplete metadata read | 80 |
| Directory no longer validates | 24 |

The probe intentionally exits 1 to expose the reproduced warning. The paired
1,000-owner sequential control exits 0 with zero scan errors and diagnostics.
Both parents are empty afterwards; the driver removes only those empty private
directories. No workload was stopped, no retry or sleep was introduced, and no
production source changed.

The initial extra-strict GCC `-O2 -Wall -Wextra -Werror` compilation fails on
`maybe-uninitialized` in the existing optional `ExclusiveNewFile` path of
`CnaToolAtomicWrite.hpp`; its log is retained and no GCC runtime result is claimed.
Both probes compile warning-free using Clang with the same optimization/warning
requirements, shared ccache and one compiler. Reports are
`build/linux-full/reports/repair-content-staging-before-clang*` and
`repair-content-staging-sequential-before*`; the original failed GCC build is
`repair-content-staging-before-build.log`.

## Root-cause boundaries, not a committed repair

`ContentBuildStagingDirectory` publishes its final recognizable directory name
before creating the lease and atomically writing metadata. The scavenger reads
and validates metadata before checking the lease. Destruction closes the lease
before recursively removing the still-visible directory. A concurrent scan can
therefore observe legitimate initialization or teardown as malformed staging.

A repair must distinguish those transitions without suppressing diagnostics for
persistently malformed directories or weakening ownership, symlink, age, bounded
scan and exclusive-lease protections. Existing native controls in
`ContentPipelineCliTests.cpp` cover old unlocked removal, active/recent retention,
malformed names/metadata, symlinks and bounded cleanup; all must remain.
Process-local locking alone cannot cover separate compiler processes. Directory
publication/retirement, if used, must not overwrite an existing target or leave
unscavengable crash debris. No design or platform-runtime validation is claimed
yet. ISSUE-057 is **CONFIRMED_OPEN**, not fixed.

## Read-only synchronization design review

Reviewed while the ISSUE-047 native checkpoint is running; no staging source or
test expectation has changed. A process-local mutex cannot protect independent
compiler processes. Merely checking the lease before metadata still leaves the
directory-creation/lease-creation interval and teardown visible. Treating a live
PID or every missing metadata file as harmless would hide persistently malformed
state and is not an acceptable repair.

A shared parent lifecycle guard is a candidate, not an implemented design. It
would cover enumeration/validation/removal and reservation initialization/
destruction, but must not serialize the actual content builds. The constructor
must not recursively acquire an independently opened file lock when invoking
the scavenger. Linux documents that separate opens are independent lock owners,
including within one process; inherited descriptors retain the lock until their
last close. Any guard needs deliberate descriptor inheritance and crash-release
semantics. [Linux flock documentation](https://www.man7.org/linux/man-pages/man2/flock.2.html).

Do not assume a directory-only flock is a portable solution: Linux's NFS
emulation requires a writable descriptor for an exclusive lock. A stable regular
lock file is a possible alternative, but deleting/recreating that shared lock
can split participants across different inodes. Introducing a persistent staging
marker would change the existing empty-parent expectation: it must be explicitly
justified/documented and tested as one exact housekeeping file, never handled by
ignoring arbitrary leftovers. Existing cleanup, ownership and malformed-state
assertions must remain meaningful; no such oracle change has been made.

On Windows, a synchronous `LockFileEx` request without `LOCKFILE_FAIL_IMMEDIATELY`
can wait in the kernel instead of adding sleeps/retries; even a second handle in
the same process is excluded by an exclusive lock. [Microsoft LockFileEx
documentation](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-lockfileex).
A named kernel mutex adds namespace, case-sensitive naming and security-descriptor
considerations, so it is not automatically equivalent to locking the filesystem
resource. [Microsoft CreateMutexW documentation](https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createmutexw).

Before selecting a repair, exercise both independent processes and threads,
aliases of the same parent, owner termination, malformed/symlink/read-only
controls, bounded scans and exact cleanup on Linux and available MinGW/Wine.
No NFS/SMB/macOS/native-Windows runtime result is claimed by this design review.
