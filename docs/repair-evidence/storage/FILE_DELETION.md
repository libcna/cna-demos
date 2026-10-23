# File deletion preservation and exceptions — partial ISSUE-010 repair

## Reference and reproduction (2026-10-23)

`StorageOperationsOracle.cs` executes the genuine XNA4 Storage assembly under Framework4/Wine:
12/12 checks pass. Constructor-free private root injection is used only for filesystem method
semantics, not platform storage selection/mounting. It verifies exact exceptions and preservation
for directory/file confusion, missing parents, missing leaves and vanished enumeration roots.
`FileDeletionOracle.cs` independently executes actual .NET8.0.31 on Linux:7/7 pass, including
the exact directory exception and a NUL-containing path with an existing prefix file.

The new Sharp native deletion tests give1PASS/4FAIL before repair:

- Directory deletion is refused, but with plain IOException instead of UnauthorizedAccessException.
- A nonexistent parent is silently accepted instead of DirectoryNotFoundException.
- A path containing NUL deletes its existing prefix file.
- A Unix directory symlink is followed by the precheck and incorrectly refused.

CNA independently gives50PASS/2FAIL of52 before its facade repair: DeleteFile deletes an empty
directory, leaks filesystem_error for a nonempty directory, and accepts a missing parent.
All these failed logs are retained. The C exception barrier also has an independent regression:
BoundaryDetailTest returns31 before the new UnauthorizedAccessException-to-IO mapping.

## Repair

Published Sharp Runtime `d051307b3316903a692846acf78200d74339bbef` uses file-only unlink or
DeleteFileW, native UTF-8 conversion, pre-call NUL rejection and typed native errors. This avoids
the directory-check/remove replacement race. No API signature/layout change. A Unix directory
symlink is unlinked without deleting its target.

CNA `4d384865ca0b586a4ed52929907589ea32a1656b` is published on next and forwards DeleteFile
to that layer after its existing disposal/argument/containment guards.
The shared C barrier maps UnauthorizedAccessException to CNA_RESULT_IO / CNA_ERROR_CATEGORY_IO,
not INTERNAL. Native C smoke adds directory preservation, missing-parent and missing-leaf controls.
The demo retains all7 previous directory checks and adds4 exact deletion/preservation checks.
No knownIssue marker is removed: ISSUE-010 still contains other storage and content defects.

## Completed deletion-batch validation

- Sharp full warning-free build,18,223/18,223 across42 executables,zero failures/skips; IO749.
  The first full IO candidate gives748/749 because an older test expected the wrong IOException.
  Its exact expected type is corrected from genuine references, retaining preservation checks.
- Strict MinGW/Emscripten syntax checks and module-boundary audit44/109 pass. Native Windows/browser
  runtime is not claimed. No new sanitizer run is claimed for this non-lifetime repair.
- Actual Windows-targeted File/FileStream code and unchanged tests pass16/16 under Win64 Wine,
  including all4 Windows-applicable FileDeletion tests. Strict C++23/-O1 build is warning-free;
  the first -O0 attempt's MinGW type_info duplicate-symbol link error remains recorded.
  Source inputs and reproduction: Sharp docs/WindowsFileIoValidation.md, commit
  4c7c69bd731b53bbb2fd87cb0f8da233278eb67d. This is not native Windows or Windows CNA validation.
- CNA storage52/52 pass after forwarding (18ms). C exception-barrier regression exits0 after
  mapping correction, versus31 before. Complete demo consumer rebuild finishes warning-free
  (469 planned/466 printed steps). Both SOFTWARE and OPENGLES3 storage7/7 and C API storage2/2
  pass; directory demo checks7 ->11, every other focused status/check count unchanged.
- Strict C17/Wall/Wextra/Wpedantic/Werror native StorageSmoke passes against the rebuilt C ABI;
  linkage is recorded. Catalog719/zero problems, all6 pixel-exact visuals, all6 platform audits
  with unchanged budgets, generated routes3197/57, route coverage3205/3205 and8 prose counts pass.
- All275 binary hashes remain frozen and verified. Comparable complete SOFTWARE and OPENGLES3
  sweeps finished serially in session14464 with prefix repair-storage-filedelete-full-v2 and
  explicit SDL_AUDIO_DRIVER=dummy, matching the checked CMake baseline setting. Each covers719 IDs.
  SOFTWARE643PASS/25FAIL/48SKIP/3NOTRUN and OPENGLES3685PASS/24FAIL/7SKIP/3NOTRUN exactly retain
  the FileShare checkpoint's statuses, markers and failing-assertion messages/multiplicities
  (51 and50 failures respectively). The only change is directory checks7 ->11, all passing.
  No failing assertion disappeared and no existing check was weakened.
- Full serial CTest finished in726.18s:735 registered,732 enabled,678PASS/48SKIP/6FAIL,
  plus3 disabled. Exact test-ID/status comparison against repair-fileshare-ctest has zero
  differences; all275 binary hashes still match after completion. Reports:
  repair-storage-filedelete-ctest.*. The six residual failures are the same C content-manager
  and graphics-resource-lifecycle failures and unexpected passes for read_asset_ownership,
  several_managers, unload_disposes and occlusion visible_pixels. The overall gate remains red;
  this batch introduces no regression and does not claim a full native CNA corpus pass.
  A daemon restart interrupted the first launch before any scope/process/log existed;
  the single executed run was session94083. The artifact freeze is now released.

The first full-sweep invocation omitted that audio setting. Its partial SOFTWARE run exposes
no-audio skips/failures, including an SDL_OpenAudioDeviceStream abort in the C VideoPlayer child;
it is not accepted as a comparable regression result. The exact owned shell863182/private scope
was stopped after identifying the mismatch; session73692 exits143 and its queued OPENGLES3 never
started. All partial category logs remain under repair-storage-filedelete-full-parts/SOFTWARE.
No test/product edit or rebuild preceded the corrected v2 run; all275 binary hashes still match.
The hardware-unavailable audio paths remain diagnostic evidence, not hidden or reclassified
into knownIssue. They require separate reference-environment triage rather than attribution to
this file-deletion change from an unmatched audio configuration.
- Builds use existing Sharp build/, CNA cmake-build-multi/, demos build/linux-full/ and existing
  build-probe/ for reference assemblies; one compiler at a time, shared ccache,6GiB cap.

Reports: `build/linux-full/reports/repair-storage-filedelete-*` and
`repair-storage-operations-net4*`. This is not full ISSUE-010 or campaign closure.

## Next checks

Preserve inherited ownership/occlusion work and the existing six full-CTest failures recorded by
[FILE_SHARE.md](FILE_SHARE.md). The CNA deletion batch is published; this evidence commit
records its corresponding permanent inventory progress without closing ISSUE-010.
CreateDirectory(existing file) and missing-root enumeration remain separate actionable cases
already demonstrated by the same genuine XNA oracle; do not lose them when closing this batch.
