# ISSUE-057: concurrent staging lifecycle baseline

Current CNA `dbfe6b4c1` still reproduces the historical warning race. The
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
