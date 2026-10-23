# Directory creation and enumeration — partial ISSUE-010 repair

## Baseline (2026-10-23)

Published deletion checkpoint: CNA4d384865ca0b586a4ed52929907589ea32a1656b,
Sharp4c7c69bd731b53bbb2fd87cb0f8da233278eb67d,
demos27fd84c809b5c9df8a2563828ffeae4c55172d31. Its complete frozen regressions are
recorded in [FILE_DELETION.md](FILE_DELETION.md); the overall gate has six existing failures.

The genuine XNA StorageOperationsOracle12/12 proves CreateDirectory must refuse an existing
file and a file used as parent with IOException, preserving contents; all four enumeration
overloads use DirectoryNotFoundException for a vanished container root.
Two new native CNA tests reproduce52PASS/2FAIL of54: existing files are silently accepted,
file parents and missing enumeration roots leak std::filesystem::filesystem_error.
A Unicode leaf-name/filtering control passes before the CNA production repair: the complete
native baseline with that control is53PASS/2FAIL of55.

Sharp Directory already supplies typed enumeration errors and refuses existing files, but
its native path calls need review before CNA forwards operations to it. Five new tests give
4PASS/1FAIL on Linux and4PASS/1FAIL with actual unchanged Windows-targeted code under Win64 Wine.
An embedded NUL creates or enumerates its prefix rather than throwing ArgumentException.
Unicode tests pass on this MinGW/Wine configuration before repair; do not claim a reproduced
Unicode failure there. Explicit existing NativePath/Utf8Of conversions preserve CNA's portable
UTF-8 path contract when forwarding through Directory, rather than relying on a narrow Windows
filesystem constructor's encoding. Native Windows/MSVC runtime validation is not claimed.

Actual .NET8.0.31/Unix DirectoryOperationsOracle12/12 independently validates NUL refusal and
prefix preservation, Unicode creation/listing, file refusal/preservation and idempotent creation.
No online reference or newer local runtime source is substituted for that measurement.

## Repair and completed validation

Sharp candidate is limited to CreateDirectory and enumeration: reject NUL/empty paths before
native access, use the existing UTF-8/native helper and return UTF-8 paths. Existing matching,
typed enumeration errors and other Directory operations are not rewritten. Focused full IO
754/754 passes, no compiler warnings. The same Windows-targeted five tests now pass5/5 under
Win64 Wine (15ms), strict C++23/-O1/Werror build, unchanged test source. Full dependency gate
passes18,228/18,228 across42 executables, zero failures/skips; full build warning-free,
boundary audit44/109 passes. Sharp2f644cf4761123275426247b4d582e3392b4a181 is published.
Native Windows/MSVC is still not runtime-tested.
CNA9c068f6324bf06cd43f3de4d035de54bce4fecfa is published on next and delegates
creation/enumeration to Directory, converts full paths to leaf names,
and retains its own existing GlobMatch and empty-pattern fast return. Native55/55 passes.
New strict C smoke assertions fail with exit4 against the old external library and pass
with exit0 against the repaired library, with the same strict C17/Werror executable. Actual
linkage is recorded. Full consumer rebuild passes (234 planned/231 printed steps), no compiler
diagnostics. Both SOFTWARE/OPENGLES3 storage7/7 and C storage2/2 pass; directory checks11->18.
Catalog719/zero problems,6 exact visual references and platform ratchet/nonproduction/hotpath
audits pass without budget changes. All275 binaries are frozen in
repair-storage-directory-full-artifacts.sha256; do not rebuild before the regression run ends.
Full SOFTWARE719 IDs:643PASS/25FAIL/48SKIP/3NOTRUN, same statuses/markers/failed-assertion multiset
as the deletion checkpoint (51 failures). Only directory checks11->18, all passing.
Full OPENGLES3719 IDs:685PASS/24FAIL/7SKIP/3NOTRUN, same statuses/markers/failed-assertion
multiset (50 failures); only the same directory check-count increase. All275 hashes match
after both complete sweeps. Final serial CTest56448 completes692.56s:
735 registered/732enabled,678PASS/48SKIP/6FAIL/3disabled. Exact ID/status comparison with the
deletion checkpoint has zero differences; all275 artifact hashes also match after CTest.
Reports repair-storage-directory-ctest.*. The binary freeze is released. The overall gate
remains red with the same C content/resource-lifecycle failures and stale knownIssue unexpected
passes listed in FILE_DELETION.md. No full native CNA corpus pass is claimed.
No issue is closed and no knownIssue is removed.

Reports: build/linux-full/reports/repair-storage-directory-*.
All compilation reuses existing build directories, one job, shared ccache where configured,
MemoryHigh4GiB/MemoryMax6GiB/no swap; cross-compiler TMPDIR is Sharp build-tmp.
Keep failed baseline logs. This evidence commit accompanies the published CNA repair. ISSUE-010's content
guard and remaining filesystem error paths still require their final audit; no full closure.
