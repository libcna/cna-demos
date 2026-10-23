# FileShare repair — CNA-DEMOS-ISSUE-152

## Published product repairs

- Sharp Runtime `c6984ecef98aa6a04634e76145763545a90c68fe`: one native owned handle,
  platform sharing, buffering, lock-before-truncate, non-destructive write-only opens,
  handle-based length/resize, append boundaries and write-error reporting.
- CNA `21b4cd64cd122353431dafb5793a19ac568956fd`: forward explicit sharing, use exclusive
  short StorageContainer overloads/CreateFile, choose write-only access for Append.
- Sharp Runtime `f51bf9acf8374088a425024b40ef1c7b697c5ba5`: strengthen the child-process
  regression with direct kernel flock, so an inherited process-local reservation cannot fake it.
- CNA `169470ee7550471844d01a0789dd5e25770544b6`: correct the C storage contract and add
  strict C smoke controls for conflict errors, failed handles, preservation and defaults.

The root cause is fixed at the lower layer, not with a demo-only lock or retries.
The original FileShare demo still executes exactly the same eight assertions. Its
knownIssue marker is removed only after SOFTWARE and OPENGLES3 each pass8/8.

## Native reference contract

The owner selected native .NET platform behavior. The original144-pair measurements
and sources remain in [README.md](README.md): Windows119refused/25allowed, Linux
.NET8 63refused/81allowed. Both must refuse conflicting FileShare.None opens.

`FileShareFlagsOracle.cs` adds all16 legal sharing masks, including Delete/Inheritable,
and append-boundary/overwrite controls. Actual results:

| Reference | Pairs | Allowed | Refused | Policy mismatches | Append failures |
| --- | ---: | ---: | ---: | ---: | ---: |
| .NET Framework4.0.30319.1, Win32NT under Wine |2304|400|1904|0|0|
| .NET8.0.31, Linux |2304|2025|279|0|0|

In the measured Unix policy only the exact None value takes an exclusive lock;
Inheritable alone is shared. Windows applies read/write/delete masks and inheritance
independently. Wine reference execution is not native Windows product validation.
The first Framework compiler invocation used an unsuitable forward-slash path and
did not compile the source; its failed log remains. The corrected `-v2` run used
the same unchanged source with proper Windows paths.

## Product validation completed

- Baseline Sharp IO727PASS plus3 new failures. After repair and added controls:
  full warning-free build and **18,218/18,218 tests in42 executables**, zero skips.
  IO744 includes144 native access/share pairs, alias/hardlink/symlink exclusion,
  separate-process exclusion, failed-open cleanup, preservation before truncation,
  write-only permissions, write failures and EOF/read/write/resize transitions.
- First candidate740/741 exposed stale read-ahead after resize; that assertion was
  retained and the implementation fixed. Supporting archive code is not entirely
  instrumented, but the actual FileStream.cpp and23 focused tests pass ASan/UBSan/LSan.
- MinGW/Emscripten final-source strict syntax checks pass. Windows C++ runtime and
  browser runtime are not claimed. Browser VFS flock is a successful no-op in this
  Emscripten implementation; this repair does not invent mandatory locking there.
- CNA native storage before46/50, after50/50. Four new tests cover the facade's
  forwarding matrix, defaults, pre-truncation refusal and Append access.
- Full existing demo target and all its required artifacts rebuilt without warnings.
  Both SOFTWARE and OPENGLES3 storage go6PASS/1FAIL ->7PASS, with no changed check counts
  or unrelated outcomes. FileShare remains8checks. The initial post-repair runs exit1
  only because the stale knownIssue reports an unexpected PASS; those reports remain.
- C API storage2/2PASS on each renderer, preserving35 and38 internal child assertions.
  An additional strict C17/Wall/Wextra/Wpedantic/Werror StorageSmoke compile/run verifies
  IO conflict results, invalid failure handles, preserved file length, shared-reader
  success and exclusive/Append defaults against the actual rebuilt libcna_c_api.so.
- Sharp module boundaries44/109 and all6 CNA platform/SDL/hot-path audits pass without
  changing any audit budgets.

**ABI:** FileStream sizeof576 ->24 on Linux x86_64, alignment8 unchanged. Before/after
and copy/move/nothrow-destructor pins are native tests. All consumers must be rebuilt,
including derived IsolatedStorageFileStream. See Sharp's migration note. No existing
public signature, vtable or exception specification changed.

## Completed wider regression (2026-10-23)

All275 frozen demo/C API/CNA binary hashes match after both full sweeps and CTest.
Each sweep accounts for all719 IDs exactly once:

| Renderer | PASS | FAIL | SKIP | NOT RUN |
| --- | ---: | ---: | ---: | ---: |
| SOFTWARE |643|25|48|3|
| OPENGLES3 |685|24|7|3|

Against the preceding complete `repair-sprite-binding-full` checkpoint there are no
added failing assertions or PASS-to-FAIL changes. Failed assertions decrease64 ->51
on SOFTWARE and66 ->50 on OPENGLES3. Other improvements include intervening published
repairs and an inherited, uncommitted occlusion candidate; they are not credited to152.
The narrow storage comparison changes only FileShare from FAIL to PASS, still8 checks.
All6 visual references are pixel-exact. Tool unit tests pass16/16.

Full serial CTest completes in695.99s: **735 registered,3 disabled,732 enabled;
678 PASS,48 SKIP,6 FAIL**. The global gate is **not green**. Its six failures are:

- `c_api.content.content_manager` and `c_api.graphics.resource_lifecycle`: pre-existing
  unmarked assertion failures, also in the preceding complete checkpoint.
- `content.content_manager.read_asset_ownership`, `content.content_manager.several_managers`,
  `content.content_manager.unload_disposes`, `graphics.occlusion_query.visible_pixels`:
  unexpected passes with retained010/011/113 markers. The shared worktree contains another
  batch's uncommitted ownership/occlusion changes; this repair neither accepts nor commits them.

Historical Complete1128 had SOFTWARE555PASS/113FAIL and OPENGLES3594PASS/115FAIL,
with unchanged skip/manual totals. `c_api.graphics.resource_lifecycle` is still a historical
PASS-to-FAIL regression, already present before this batch; it remains actionable and visible.
No FileShare test is suppressed or relaxed to obtain the improvement.

**Disposition: FIXED_PLATFORM_VALIDATION_PENDING.** Linux implementation, CNA facade and
both available renderer-labelled reproductions are validated. Windows product runtime remains
pending despite strict MinGW compilation and genuine Windows reference runs. Browser VFS
advisory locking is explicitly best-effort, not a claim of native exclusion. This disposition
does not certify the entire CNA corpus or close the campaign.

Reports: `build/linux-full/reports/repair-fileshare-*`. Builds used existing Sharp
`build/`, CNA `cmake-build-multi/`, demos `build/linux-full/`, and existing build-probe
directories for focused executables/oracles. One compiler at a time; shared ccache;
C++ builds use4GiB soft/6GiB hard/no-swap scopes. No consumer repo or pin was edited.
