# ISSUE-057: concurrent staging lifecycle baseline

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
