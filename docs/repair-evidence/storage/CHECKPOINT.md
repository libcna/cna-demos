# Storage checkpoint — 2026-10-20

Status: FIXED_VALIDATED. Issues 056, 145 and 151 are closed.
All 155 historical IDs remain: 65 final, 90 pending (33 runtime findings, one
verified duplicate, 23 documentation corrections, seven pre-campaign fixes and
one demonstrated demo-expectation defect).

## Revisions and scope

CNA `5865ae938` repairs disposed StorageContainer behavior; `cbb77e3c8` repairs
TitleContainer and its C adapter. Sharp Runtime commits `1c0fc9b5`, `551ced42`,
`82b57541`, `3304ecd6` and `58c6cba1` repair EOF state, directory categories,
relative URI handling, invalid paths and native status-error preservation.
Demo marker commit `de6bcde` removes only the three validated restrictions.
Later XML/content changes are NOT in these frozen external binaries.

## Contract, native and C ABI validation

Genuine reference evidence includes EOF 13/13, storage 172/172, disposal 7/7,
title paths 30/30, and version-specific file-path validation 72/72 on both
Framework 4 and .NET 8. FileShare behavior differs between those platforms;
issue 152 remains open, with no inferred policy approval.

The final Sharp storage build is warning-free; its complete gate passes
18,144/18,144 across 41 executables, zero failures/skips. Native title/storage
focus passes 17/17. The exact 1,412 prior-consumer inventory gives 1,408 passes
and four unchanged skips; runtime suites give 238 passes and three unchanged
skips out of 241. Actual TitleContainer.cpp and FileStream.cpp plus 13 tests pass
ASan/UBSan/leak checks. Earlier actual-implementation sanitizer regressions also
pass for EOF (3 tests) and disposed storage (4 tests). Android title-path branch
syntax compilation passes; no Android runtime or NDK cross-build is claimed.

The rebuilt external C ABI passes strict C17/Werror RuntimeGameSmoke,
StorageSmoke and independent TitleCValidationProbe (13/13, previously 8/13).
`ldd` and SHA256 checks verify the actual external C ABI library, not the native
C++-only configuration. Both external library hashes remain unchanged after the
marker rebuild. All ten focused demos per renderer give nine passes and the
same issue-152 failure. Title's 10 checks, lifetime's 14 checks, and the actual
C save child's 38 checks are preserved.

## Full external regression

Both full 719-scenario runs have exactly 55 intended improvements versus the
campaign-HEAD baseline, or 57 versus Complete1128 including two pre-campaign
improvements. Strict comparisons of status, knownIssue, check counts and failed
assertion multisets have `differences: []` on BOTH renderers. No whitelist or
retry hides issue 030; its original assertions remain failures in this run.

| Renderer | PASS | Expected FAIL | SKIP | Manual/not run |
|---|---:|---:|---:|---:|
| SOFTWARE | 612 | 56 | 48 | 3 |
| OPENGLES3 | 651 | 58 | 7 | 3 |

Both 95-scenario math sweeps retain 93 passes and the two owner-reserved 131
failures, with exactly 48 math-related improvements. Metadata validation and
tool suites (7 + 4 + 5) pass. All six visual references are pixel-exact, maximum
channel difference zero; the GLES texture capture was also visually inspected.
No other renderer/platform validation is claimed for this batch.

After both smoke sweeps finished, full serial CTest ran on an isolated private
GPU display, with no overlapping campaign SystemLink suite. All 732 enabled tests
are accepted, zero unexpected failures, the same 48 skips and three disabled tests,
921.47 seconds. All three frozen hashes passed both before and after CTest. The
three repaired registered tests no longer have WILL_FAIL; assertions are retained.
The storage external freeze is now released for the next content repair batch.

Reports: `build/linux-full/reports/repair-storage-*`, including the frozen SHA256
manifest, strict comparisons, visual captures and full CTest log. Historical
failed probes/builds and platform limitations remain in [README.md](README.md).
No push or merge.
