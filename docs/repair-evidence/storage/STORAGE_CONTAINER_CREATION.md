# StorageContainer construction — partial ISSUE-010 repair

2026-10-23. Baseline CNA `15d5d6f3d`, Sharp Runtime `2f644cf4`, demos `92ee315`. Repair: CNA
`c30925496` with Sharp Runtime `530e217f` (both local, not yet pushed).

## Genuine XNA reference

`StorageContainerCreationOracle.cs` calls the real `StorageDevice.EndOpenContainer`, so the real
`StorageContainer` constructor resolves `Environment.GetFolderPath(Personal)`, the `SavedGames`
and title folders and the player folder itself; nothing is injected. The shared Wine prefix maps
`Documents` to the owner's real `~/Documents`, so the runner (`StorageContainerCreationRunner.mjs`,
run as `build-probe/repair-storage-ctor-reference.mjs`) built a 5.7 MB overlay prefix: copied
registry files, symlinks to the shared `drive_c` content, and a real empty `Documents` fixture.
Neither the shared registry nor `~/Documents` was written; the oracle refuses to run unless
`Personal` is the fixture. The overlay was removed afterwards (symlinks first, then the remaining
real files).

Results (`build/linux-full/reports/repair-storage-ctor-reference-v1-*`): compile clean, create
mode 20/20, denied mode 3/3, peak 721,780,736 bytes, no memory events.

- `BeginOpenContainer` completes and invokes its callback without filesystem access.
- `EndOpenContainer` constructs the container. A file occupying the container name, the player
  folder, the title folder or `SavedGames` itself is an exact `System.IO.IOException` with no
  inner exception, e.g. `Cannot create "...\SavedGames" because a file or directory with the same
  name already exists.`
- The failed End consumed its result: a second End is `InvalidOperationException`.
- The blocking file is preserved; a later open succeeds once it is gone (nothing is cached).
- A runner-made unwritable parent is an exact `UnauthorizedAccessException` and nothing is created.
  This maps a Unix permission through Wine, not a Windows ACL.

The decompiled constructor agrees: `FinishCreation` calls `DirectoryInfo.Create()` on the whole
chain with no handler, and `StorageDeviceNotConnectedException` is never thrown on Windows.

## Lower-layer .NET reference

`DirectoryCreationErrorsOracle.cs` on actual .NET 8.0.31/Unix: 5 PASS / 1 FAIL of 6
(`repair-storage-ctor-direrrors-net8*.log`). File at the leaf is `IOException`; an unwritable
parent is `UnauthorizedAccessException`; a component longer than NAME_MAX is
`PathTooLongException`. The one failure is a real platform difference, retained rather than
edited: .NET on Unix reports a file *ancestor* as `DirectoryNotFoundException`, whereas XNA/.NET
Framework 4 (above) and .NET on Windows report `IOException`.

## Defect

CNA resolved and *created* the storage root in `EndOpenContainer` before construction, wrapping a
failure in `StorageDeviceNotConnectedException`, and the constructor called the throwing
`fs::exists`/`fs::create_directories`, leaking `std::filesystem::filesystem_error`. With a file at
the player folder, `fs::exists` was true and a container was opened on top of a file. Sharp's
`Directory::CreateDirectory` reported denial as `IOException`.

Seven new CNA tests (`StorageContainerCreationTests.cpp`): before the change six ran, 1 PASS /
5 FAIL (`repair-storage-ctor-before-Storage.log`, 56/61 overall); the seventh, container
`CreateDirectory` under an unwritable directory, was added with the Sharp change, whose own two
new tests were 5 PASS / 2 FAIL before it.

## Repair

- Sharp Runtime `530e217f`: `Directory::CreateDirectory` maps EACCES/EPERM to
  `UnauthorizedAccessException` and ENAMETOOLONG to `PathTooLongException`; every other failure,
  including a file anywhere in the chain, stays an exact `IOException`.
- CNA: `EndOpenContainer` only resolves the root (`ResolveStorageRoot`, no creation); the
  constructor creates the whole chain through `System::IO::Directory::CreateDirectory`. The browser
  storage-readiness refusal is unchanged. `GetStorageRootEXT`, `SetAppNameEXT`'s isolated-storage
  resolver and the C root routes keep reporting an unusable root as a disconnected device, so the
  museum B11 extension contract and its `show_selector` assertions are untouched.
- One existing CNA assertion that expected `StorageDeviceNotConnectedException` from
  `EndOpenContainer` contradicted the genuine reference and now expects `IOException`; its other
  four assertions are unchanged.

## Validation

- Sharp: warning-free full build; IO 756/756; full gate 18,230/18,230 across 42 executables,
  zero failures/skips, XNA sample fixtures and the private Yacht SOAP fixture; boundaries 44/109;
  MinGW-w64 strict C++23 syntax check of `Directory.cpp`. No Windows runtime is claimed.
- CNA `cmake-build-multi`: `CnaStorageTests` 62/62, also in three shuffled orders; C API
  `StorageSmoke`, `ModuleProbe`/`ModuleLinkClosure` (and XML-free) for storage 4/4; build
  warning-free apart from the known Draco CMP0148 configure note.
- CNA `build-asan` (address, undefined, float-cast-overflow, leak detection; 141 instrumented
  translation units including Sharp `Directory.cpp`): `CnaStorageTests` 62/62, no reports.
  Third-party code is not claimed instrumented.
- Demos `build/linux-full` targeted rebuild (`cna-demos` and both C storage programs, 277 steps,
  no diagnostics). Through the private runner, storage 7/7 and C storage 2/2 on SOFTWARE and on
  OPENGLES3, with statuses, check counts and knownIssue values identical to the directory
  checkpoint. CNA platform audits pass at unchanged budgets.

No full 719-ID sweep or serial CTest was rerun for this batch; the other demo binaries in the tree
were not relinked. ISSUE-010 stays open for the residual storage paths below.

## Residual ISSUE-010 audit (next)

- `StorageDevice::DeleteContainer` uses the throwing `fs::remove_all` (a denied subtree leaks
  `std::filesystem::filesystem_error`), resolves through the creating root, and removes the whole
  named container where XNA deletes only this device's player folder.
- `StorageContainer::FileExists`/`DirectoryExists` use the throwing `fs::is_regular_file`/
  `fs::is_directory`; XNA's `File.Exists`/`Directory.Exists` answer false instead of throwing.
