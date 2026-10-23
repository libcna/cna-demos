# Residual storage paths — ISSUE-010 completion

2026-10-23. Baseline: CNA `c30925496` (container construction), Sharp Runtime `530e217f`,
demos `965efdb`.

## Genuine references

`StorageResidualPathsOracle.cs`, run by `StorageResidualPathsRunner.mjs` exactly like the
construction oracle (real `StorageDevice`/`StorageContainer` code in an isolated Wine overlay
profile; `~/Documents` and the shared registry untouched; overlay removed afterwards). Reports
`build/linux-full/reports/repair-storage-residual-reference-v{1,2}-*`.

Create mode, 11/11 in both runs:

- `DeleteContainer` on the shared device removes `<name>\AllPlayers` only; Player1's folder and
  the named folder stay. A player device removes only its own folder.
- A missing container is a no-op and creates nothing; a file at the container name is a no-op and
  is kept. An empty name does not throw and removes `<title>\AllPlayers` (`Path.Combine(x, "")`).
- `FileExists`/`DirectoryExists` beneath a file answer false.

Denied mode (the runner removes Unix search permission from `AllPlayers\locked`, write permission
from `AllPlayers\inner` and `AllPlayers\ro`; Wine maps them, these are not Windows ACLs):

- Beneath the unsearchable directory, `FileExists`/`DirectoryExists` answer false and `OpenFile`,
  `CreateFile`, `DeleteFile`, `CreateDirectory`, `DeleteDirectory` all throw an exact
  `UnauthorizedAccessException`.
- `DeleteContainer` with an undeletable file throws `IOException("Directory is not empty.")` and
  leaves the file. v1 expected `UnauthorizedAccessException` and recorded this as its one FAIL
  (7/8); it is retained, not edited away. Wine defers the unlink of a file in an unwritable
  directory, so .NET Framework's recursive delete sees only the non-empty directory.
- v2 adds `DeleteDirectory` in an unwritable (searchable) parent: Wine reports success while the
  directory remains (9/10). Both are Wine deferred-deletion artifacts, so these two cases have no
  genuine measurement here.

`DirectoryDeletionErrorsOracle.cs` on actual .NET 8.0.31/Unix, 4/5
(`repair-storage-residual-direrrors-net8*.log`): a recursive delete with an undeletable file is
`UnauthorizedAccessException` and keeps the file (also what .NET Framework's source does on Windows,
where the file's ACCESS_DENIED wins); `File.Exists`/`Directory.Exists` beneath an unsearchable
directory are false. The FAIL is retained: .NET 8 reports `Directory.Delete` beneath an unsearchable
directory as `IOException`, while XNA (Framework 4 queries attributes first) reports
`UnauthorizedAccessException`; CNA follows XNA.

## Defects

Six new CNA tests (`StorageResidualPathsTests.cpp`): 1 PASS / 5 FAIL before
(`repair-storage-residual-before-Storage.log`, 63/68 overall).

- `DeleteContainer` removed the whole named folder: every player's saves, and a *file* named like
  the container (data loss). It created the storage root just to resolve it, and the throwing
  `fs::remove_all` leaked `std::filesystem::filesystem_error`.
- `StorageContainer` reported a component it could not canonicalize (no search permission) as an
  escape, so every operation beneath it threw `ArgumentException` and `FileExists`/
  `DirectoryExists` threw instead of answering false.
- Found while auditing: with no `XDG_DATA_HOME`/`LOCALAPPDATA`/`HOME` and a deleted working
  directory, root resolution leaked `filesystem_error`; a seventh test failed before.

## Repair (CNA `162e07b8a`)

- Containment (`StorageContainer::ResolveBelow`) still refuses lexical and symlink escapes. When
  canonicalizing fails with a filesystem error, the lexically contained path is used: the operation
  cannot traverse that component either and reports the filesystem's own error.
- Exists queries use non-throwing status and answer false on any failure.
- `DeleteContainer` mirrors XNA's internal `StorageContainer.DeleteContainer`: only this device's
  player folder, only when it is a directory, nothing created; removal failures map to
  `UnauthorizedAccessException`/`IOException` like `DeleteDirectory`. Absolute and escaping names
  stay refused (deliberate CNA hardening). An empty name follows XNA in C++; the C ABI keeps its
  documented refusal of empty relative paths (`CApi_StorageSmoke` caught the difference).
- An unresolvable root is a `StorageDeviceNotConnectedException`, like other unusable roots.
- Three older CNA tests encoded the whole-folder semantics or the empty/"." refusal and contradict
  the reference; they now assert XNA's scope and keep their safety assertion that the storage root
  itself survives.
- Demos: `storage_device.storage_root` asserted that a shared `DeleteContainer` removes every
  player's files; it now checks XNA's per-device scope and cleans up (5 -> 6 checks).
  `path_rules` and `directories` caught `std::exception` because of this issue; they now require
  `System::ArgumentException` and `System::IO::IOException`.

## Validation

- CNA `cmake-build-multi`: storage 69/69, also in three shuffled orders; ASan/UBSan/
  float-cast-overflow/leaks 69/69 with no reports; C `StorageSmoke`, `ContentSmoke`,
  `ContentReaderSmoke`, `MediaSmoke` and the storage link probes 7/7 through the private runner;
  C API routes current, route coverage 3205/3205, limitations current, coverage inventory
  refreshed (its hash was already stale after `c30925496`'s header change; counts unchanged).
- Storage-dependent module executables from the source directory: GamerServices 736 + 1
  environment skip, Net 565, Runtime 244 + 4, Platform 524 + 13, Media 355; no failures.
- Demos: storage 7/7 and C storage 2/2 on SOFTWARE and OPENGLES3; catalog 719 IDs, 0 problems;
  catalog CTests 3/3.

## Complete checkpoint

The demos tree was rebuilt against `162e07b8a` (with the three corrected demos) before a complete
sweep of all 719 IDs on each renderer through the private runner (`repair-storage-closure-full-*`):
SOFTWARE 653 PASS / 15 FAIL / 48 SKIP / 3 NOT RUN, OPENGLES3 695 / 14 / 7 / 3. Against the last
complete checkpoint (`repair-packet-range-full`, 647/21/48/3 and 689/20/7/3, before the later
lifetime, gamer and occlusion repairs) `tools/compare_checkpoints.py` reports only rows those
committed repairs turned FAIL to PASS or whose knownIssue markers they removed, the intended
`storage_root` check count 5 -> 6, and failed assertions 39 -> 31 / 38 -> 30. No status or
assertion regressed and no new failure appeared.

Serial CTest of the same tree (`repair-storage-closure-ctest.{log,xml}`, 878.91 s): 735
registered, 732 enabled, 684 PASS / 48 SKIP / **0 FAIL** / 3 disabled. Against the directory
checkpoint's 678/48/6 FAIL/3, exactly six tests changed, all FAIL to PASS (C content manager,
C resource lifecycle, the three content-manager ownership demos and the occlusion query -- the
repairs and marker removals committed since). ISSUE-010 is closed with CNA `fe2eb8f33`.

Also found during this audit and fixed separately (CNA `a24c75b2e`, not a CNA-DEMOS ID): a
dangling symlink inside a container passed the containment check, so creating a file through it
wrote outside the container.

