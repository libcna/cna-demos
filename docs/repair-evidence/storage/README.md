# Storage/runtime repair evidence (056, 145, 151, 152)

## Current checkpoint (2026-10-20)

056/145/151 are FIXED_VALIDATED; see [CHECKPOINT.md](CHECKPOINT.md). CNA repairs5865ae938
(disposed storage) and cbb77e3c8 (title paths/C adapter), Sharp repairs1c0fc9b5
(EOF),551ced42 (directories),82b57541 (relative URI),3304ecd6 (invalid paths),
58c6cba1 (native path-status errors) are committed. All historical sections below
describe the measured pre-repair state unless explicitly labeled otherwise.

Sharp's final warning-free full build and full component gate pass18,144/18,144
across41 executables, zero failures/skips. Native title/storage focus17/17 passes;
exact native consumers1408PASS/four unchanged skips and runtime241total238PASS/
three unchanged skips pass. Actual TitleContainer.cpp plus FileStream.cpp and13
regression tests pass ASan/UBSan/leak checks. Android branch syntax-only compilation
passes; this is not an NDK cross-build or Android runtime claim. Reports:
`repair-title-status-native-*`, `repair-title-nul-final-sanitizer*`,
`repair-title-android-branch-syntax.log`.

Actual rebuilt external C API passes strict C17/Wall/Wextra/Wpedantic/Werror
RuntimeGameSmoke, StorageSmoke and the independent TitleCValidationProbe13/13
(previous8PASS/five FAIL). The independent probe checks argument categories, buffer
preservation, directory errors and cleanup; the existing runtime smoke retains its
missing-path and successful-byte controls. ldd binds
the actual external C ABI DSO; both libraries' hashes match before/after execution.
Reports `repair-storage-c-after-*`, `repair-storage-c-libraries*.log` and manifest.
The full external demo build now passes warning-free. Ten focused scenarios on each
of SOFTWARE/OPENGLES3 give9PASS/one unchanged152FAIL; title10/lifetime14 checks and
the actual C save child's38 checks remain unchanged. Both DSOs still match the C ABI
validation manifest. Only then removed056/145/151 markers; marker rebuild passed.
Both full renderer sweeps now have exact55 improvements/differences[] versus the
campaign baseline; all six visuals are pixel-exact. Final isolated serial CTest
accepts all732 enabled tests with zero unexpected failures,48same skips/three
disabled,921.47s. All three frozen hashes pass before/after; external freeze released.
FileShare152 remains open; the platform-policy question has no answer yet.

### Invalid paths and error preservation

TitlePathsOracle now passes30/30 genuine checks, adding embedded NUL with existing
and missing prefixes. FileStream formerly opened the prefix of a NUL-containing
name and could truncate the wrong file. FilePathValidationOracle verifies exact
exceptions, argument precedence and unchanged seed bytes. Its explicit Framework4
mode and default .NET8.0.31 mode each pass72/72. They deliberately differ for long
Append/ReadWrite input: Framework4 normalizes the path before its append-access
guard; modern .NET rejects the arguments first. The initial71/72 Framework4 result
and diagnostic rerun remain retained, not overwritten or claimed as universal parity.
Sharp keeps the modern .NET argument order; XNA TitleContainer uses Open/Read.
Reports `repair-file-path-{validation,length,length-detail,versioned}-xna*`,
`repair-file-path-net8*` and `repair-title-nul-xna*`.

Sharp rejects empty input before mode/access checks, and embedded NUL after those
checks but before filesystem effects. Native missing errors remain missing errors;
filename-too-long, denied and other status failures retain their correct exception
categories instead of becoming FileNotFoundException. New regressions fail before
each repair. CNA opens through that corrected FileStream without masking errors in
an existence check; Android only falls back for genuine missing-file categories.
The old Framework4 URI length boundary stays in CNA's XNA layer, not modern Sharp.

Retained intermediate failures include the native length regression (16/17), its
broader runtime gate (237PASS/three skips/one FAIL of241), and an expanded sanitizer
link failure missing File helpers. The latter is not a sanitizer runtime failure;
the corrected build links the actual Sharp archives and all13 tests pass. No test
assertion or tolerance was weakened. The following sections retain the original
investigation chronologically.

## EOF is a state-transition defect, not only a getter result (056)

`FileStreamEofOracle.cs` runs against the existing .NET Framework4 environment
used by genuine XNA:13/13pass (`repair-file-stream-eof-xna*`). It checks short and
repeated EOF reads, exact bytes/guards, position, relative seek, explicit rewind,
and a write immediately after EOF followed by length/data verification.

`FileStreamEofProbe.cpp` runs the same13 checks directly against unchanged Sharp
Runtime exposed by the native CNA build. Seven fail: position after short/repeated
EOF, relative seek, position/length/data after the write, and final position.
The short-read bytes, guards and explicit rewind remain correct. Strict C++23
Wall/Wextra/Wpedantic/Werror build passes. Reports:
`repair-file-stream-eof-native-before{,-build}.log`.

Source confirms fstream.read leaves fail/eof state set; tellg then returns-1,
and a subsequent write can be silently lost. Repair the stream state at the
responsible layer, preserving real I/O errors. Do not clamp the position getter
or emulate it in the CNA C adapter. Each probe creates a dedicated new fixture,
refuses an existing path and removes only its own fixture afterward.

## Genuine Storage contracts and historical-suggestion correction (151/152)

`StorageContractsOracle.cs` passes172/172 genuine XNA checks
(`repair-storage-contracts-xna*`). It uses FormatterServices plus private-field
injection solely to give real public StorageContainer methods an isolated root.
This does **not** validate device selection or container construction. It verifies:

- all144 combinations of two read/write accesses and None/Read/Write/ReadWrite
  sharing policies, including the symmetric compatibility requirement;
- all15 properties/file/directory/list/open doors after Dispose, three invalid
  arguments showing disposed-check precedence, one-shot event identity/state,
  repeated Dispose, and unchanged filesystem contents;
- shorter overload defaults and Append access, separately from explicit sharing.

The permanent ISSUE-152 historical suggestion says to keep shorter overloads'
ReadWrite sharing default. Fresh genuine evidence disproves that as an XNA
contract: StorageContainer's two- and three-argument OpenFile are **exclusive**
(FileShare.None), and its two-argument Append selects **write-only** access.
The current CNA public header does not document an intentional contrary default;
only its implementation currently supplies ReadWrite access/sharing. By contrast,
the three-argument **FileStream constructor** defaults to FileShare.Read. These
are distinct APIs and must not be conflated when implementing the missing lower-
layer share constructor. Preserve the historical issue body and explain the
correction in its eventual resolution; do not follow its suggested fix blindly.

Sharp Runtime's StandingApprovals.md has been read completely. Any eventual
private-layout change requires its measured layout/migration/full-rebuild gates;
all Sharp compilation/test work is limited to two jobs. No dependency edit,
platform-wide FileShare implementation claim or storage closure is made here.

## Fresh native reproduction

`StorageContractsProbe.cpp` uses CNA's real Begin/End selector and container-open
path under a new isolated XDG root. It builds warning-free with C++23 and
Wall/Wextra/Wpedantic/Werror. Its170 checks match the genuine probe except for two
direct FileStream sharing checks: the required four-argument constructor does not
yet exist in Sharp Runtime, so those two cannot be expressed on its current API.

Current native result:29PASS/141FAIL. All119 incompatible sharing combinations
are incorrectly admitted;25 compatible combinations pass. Both shorter-overload
exclusivity checks fail; two-argument Append throws because CNA supplies ReadWrite
access. All18 post-disposal access/precedence checks fail, and the rejected-operation
filesystem invariant fails: disposed calls actually create/delete probe files.
Live properties/filesystem and one-shot disposal state/event controls pass.
Reports: `repair-storage-contracts-native-corrected-before{,-build}.log`.

The first probe did not catch the unexpected Append exception and terminated;
`repair-storage-contracts-native-before.log` is retained, as is its isolated fixture.
The corrected probe reports that exception as a failed assertion and continues all
independent checks; its own named fixtures are removed without recursive deletion.
No product modification, suppression or storage closure is claimed.

Cleanup-only follow-up retains the same170checks/141failures
(`repair-storage-contracts-native-cleanup{,-build}.log`). A standalone process must
not reset SetAppNameEXT after deleting its fixture: that reset creates a fresh
default `game` directory. The probe now exits without that unnecessary reset,
and its entire newly created fixture is verified absent afterward. The two empty
directories left by the earlier reset were removed non-recursively; the original
aborted run's evidence fixture remains retained.

## Sharing must reject before mutation; write-only opens must preserve data

`FileShareEdgesOracle.cs` initially added15/15 genuine .NET Framework4 checks, including
Create/Truncate rejection without changing existing bytes, normalized path aliases,
an independent child process before/after Close, idempotent release and failed-
constructor cleanup. The child really opens the file; no simulated lock or retry is
used (`repair-file-share-edges-xna{,-build}.log`). This is Wine/.NET4 validation,
not a claim of runtime testing on a physical Windows host.

The expanded oracle now passes21/21 (`repair-file-open-edges-xna*`, warning-free):
File.Create defaults to exclusive sharing, File.OpenRead permits readers but not
writers on Windows, and opening an existing directory in each of Read/Write/
ReadWrite reports UnauthorizedAccessException. The directory result identifies
the lower-layer prerequisite for Title145's InvalidOperationException translation;
do not emulate FileStream's missing directory check only in the CNA caller.
StorageContainer.CreateFile delegates to File.Create in genuine XNA, so its default
must also stay exclusive when adding the FileStream sharing overload.

Four of these controls also expose a related lower-layer data-loss defect before
any repair: `FileStreamWriteOnlyProbe.cpp` fails4/4 against unchanged Sharp Runtime
for FileMode.Open and OpenOrCreate with FileAccess.Write. Opening the existing file
truncates it before any write; overwriting one byte therefore destroys the remaining
bytes. The fstream `out`-only mode implicitly truncates, unlike the public file mode.
The strict C++23/Wall/Wextra/Wpedantic/Werror build passes, and its exact new fixture
is removed afterward (`repair-file-stream-write-only-before{,-build}.log`).
Preserve these controls in the152 lower-layer file-opening repair; adding a sharing
check after a truncating open would still corrupt data on a rejected operation.

The inspected local modern .NET Unix SafeFileHandle implementation explicitly uses
advisory flock: exclusive for None, shared for other policies, with documented
weaker granularity than Windows. That platform limitation is evidence, not proof
that all144 genuine Windows access/share combinations have been implemented on Unix.
Do not claim cross-process Windows parity from a process-local reservation table.

## Title paths (145): one historical subdefect was already repaired

`TitlePathsOracle.cs` passes20/20 against genuine XNA4; the same public-native
checks in `TitlePathsProbe.cpp` give6PASS/14FAIL. Both builds use warnings as errors.
Four exact data/capability checks retain title-root resolution after changing cwd,
backslashes and internal dot/parent normalization. The two missing-file cases
(including a missing parent directory) already produce FileNotFoundException on
current CNA, unlike the historical report. The complete Color smoke likewise has
only two failures in the existing ten-check title demo: absolute and empty names.

Remaining contracts include exact empty/rooted/drive-relative/escaping-parent and
invalid-character categories, plus InvalidOperationException when the requested
title entry is a directory. Source inspection finds public OpenStream skips XNA's
cleaned-path validation and I/O exception translation. Its existing native absolute-
path acceptance test and public relative-or-absolute comment contradict genuine
XNA; neither contains an explicit intentional-extension rationale. Preserve the
case-insensitive title lookup and platform asset loading when repairing this path.
The C title-stream route and achievement catalog/picture callers are the only
production callers found; no content-wide rewrite is justified.

Reports: `repair-title-paths-xna{,-build}.log`,
`repair-title-paths-native-before{,-build}.log`. Both new fixture roots were removed;
no runtime source or demo assertion is changed by these probes.

Four native `TitleContainerPathTests.cpp` regressions now independently retain the
same contracts. The strict standalone C++23 build is warning-free; two positive/
missing-file controls pass and two argument/directory groups fail on unchanged
CNA (217ms, `repair-title-native-tests-before*`). Each test restores both cwd and
TitleLocation and removes only its uniquely named fixture. These tests are not
yet integrated into a rebuilt CnaTests executable or included in a repair commit.
The C title-read adapter additionally catches every std::exception as I/O and
still documents absolute names: its argument-category mapping must be reviewed
with the underlying145 repair, not hidden by changing demo expectations.

`TitlePathNormalizationOracle.cs` additionally records32 genuine cleaned paths,
absolute classifications and public open outcomes (`repair-title-normalization-xna*`,
warning-free build). This is a diagnostic measurement, not32 asserted passes.
XNA cleans before validating: `bad*/../seed.bin` can become an admissible seed;
`child//../seed.bin` collapses the empty component, not `child`, and consequently
looks for `child/seed.bin`. A blanket `lexically_normal()` is not equivalent.
Nonempty input cleaned to the title directory raises InvalidOperationException,
not the initial empty-input ArgumentNullException. Windows-specific trailing-space,
NUL/newline and case results are recorded separately from portable lexical rules;
do not infer native Unix filesystem parity from a Windows open outcome. The probe
removed its unique fixture and did not change production code.

The normalization witness now adds a fifth native test. The three successful
clean-before-validation spellings and two exact missing-file outcomes are retained;
the expanded unchanged-production run gives2PASS/three expectedFAIL,117ms
(`repair-title-native-tests-expanded-before*`). In particular, accepting the
two empty/dot-component cases is itself a wrong behavior, not a tolerance issue.

### Directory prerequisite and relative-URI diagnostic (2026-10-20)

The sharing-edge witness now has36/36 genuine checks. Its18 directory cases cover
every FileMode/FileAccess pair: invalid combinations throw ArgumentException first;
valid CreateNew throws IOException, other valid directory opens throw
UnauthorizedAccessException. The native pre-fix matrix fails. Sharp Runtime repair
551ced42 preserves this precedence without changing public layout or implementing
FileShare. Its full build is warning-free and all18,141 tests across41 executables
pass, with no failures/skips. Reports: `repair-file-open-directory-matrix-xna*` and
Sharp `build-probe/cna-repair-directory-*`. CNA145 is not closed by this prerequisite.

`RelativeUriOracle.cs` records36 genuine constructor outcomes, not36 asserted passes.
Empty and whitespace-only references are accepted unchanged by Relative and
RelativeOrAbsolute, rejected by Absolute. NUL/newline in relative strings are
accepted by URI validation itself. The old Framework4 length boundary is65520
UTF-16 code units; do not assume the current .NET snapshot has the same limit.
Reports: `repair-relative-uri-xna{,-build}.log`. This reveals why directly inserting
the current Sharp Uri guard into TitleContainer would incorrectly reject a cleaned
empty relative path. Sharp repair82b57541 now passes the full18,142-test gate across
41 executables, zero failures/skips, with a warning-free full build. Its new matrix
fails against the exact pre-fix Uri.cpp compiled directly from committed551ced42;
initial harness compiler errors are separately retained, not claimed as runtime evidence.

The expanded `TitlePathsOracle.cs` passes28/28 genuine checks, adding cleaned-empty
paths and ASCII/astral UTF-16 length boundaries. Reports: `repair-title-expanded-xna*`.
The first native title repair passes all15 title/storage-focused tests, then a new
sixth path test exposes the missing old Framework4 length bound:5PASS/1FAIL,
`repair-title-length-before*`. This bound belongs in CNA's XNA compatibility layer,
not modern Sharp Uri. The original five path tests and all existing checks stay.

`TitlePathNormalizationOracle.cs` now records inner exception types and actual
loaded assembly identity. The GAC XNA4 DLL has SHA256
`8b7aa4c4939f243b21432747281cc8aacdcda56191a16d9eaa036b4136cf0da4`.
Its OpenStream IL_0076–0084 recognizes only FileNotFoundException and
DirectoryNotFoundException as missing-file categories; ArgumentException is not
included, despite the available decompiled Windows source listing it. Thus an
underlying path ArgumentException is wrapped as InvalidOperationException, agreeing
with the NUL/newline Windows measurements. Follow the actual assembly, not that
decompilation discrepancy. Reports: `repair-title-normalization-{inner,assembly}-xna*`,
`repair-title-genuine-il.log`, `repair-title-genuine-assembly.sha256`. These remain
platform-specific file-input observations, not a claim of identical Unix filenames.

## Native151 regression coverage prepared before its repair

`StorageContainerLifetimeTests.cpp` adds three native tests: live identity and
one-shot disposal controls, all15 post-disposal public doors with filesystem
invariants, and disposed-before-invalid-argument precedence. The standalone
strict C++23/Wall/Wextra/Wpedantic/Werror build passes; unchanged storage production
gives1PASS/two expectedFAIL (5ms; `repair-storage-lifetime-native-before*`).
Fixtures use unique container names through real public acquisition and remove
only their own named files/directories. These tests have not yet been rebuilt
into CnaTests; no runtime151 repair or external checkpoint includes them yet.

An additional genuine `StorageDisposingOracle.cs` passes7/7: inside the event,
IsDisposed is already true, FileExists/listing throw ObjectDisposedException,
backing files remain, and a previously opened stream still reads during and after
disposal. This does not claim device-selection/constructor coverage (root injected).
The matching fourth native test fails before repair, alongside the two earlier
disposed-access failures; the live control still passes. The isolated151 candidate
passes all4 (2ms). Reports `repair-storage-disposing-xna*`,
`repair-storage-lifetime-native-event-*`, `repair-storage-lifetime-event-candidate*`.

This also disproves an existing native C `StorageSmoke.c` expectation that its
FileExists query succeeds inside Disposing. Correct that oracle with the151 repair:
require INVALID_STATE and untouched output, and separately prove the backing file
still exists via a fresh live container. The generic C exception barrier already
maps ObjectDisposedException through InvalidOperationException to INVALID_STATE;
do not add a redundant adapter guard or change the canonical event ordering.

## Unchanged Sharp Runtime baseline preparation

Reused the existing Debug `build/`, capped at two jobs. Its incremental baseline
build completes without warnings/errors (11 reported link steps). The initial
full component gate reaches XML Serialization with zero failures but five skips:
its default old XNA sample-root location is absent. The original sample files do
exist under `/rv/tmp/samples`, so the supported `XNA_SAMPLES_ROOT` is now directed
to a private two-symlink layout in Sharp Runtime `build-probe/cna-repair-xna-samples`.
No sample or test is modified, no skip is permitted, and the first failing gate
log is retained as `build-probe/cna-repair-storage-baseline-components-missing-fixtures.log`.
The unchanged full gate now passes **18,137/18,137 across41 executables, zero
failures and zero skips**, using those fixtures and the original Yacht SOAP server
on a private loopback port. The fixture process stopped and its temporary executable/
config were removed by the existing helper. Current Sharp revision remains
`f2aecfefeac171b331787e5cdc4849507363ef0e`, clean; no production edit or new build tree.
Reports: Sharp `build/gs-007b-components-live.log` and
`build-probe/cna-repair-storage-baseline-components-corrected-driver.log`.

Only after that complete unchanged baseline, three native `FileStreamEofTests.cpp`
regressions were added in Sharp Runtime. All three fail against unchanged production:
short/repeated EOF preserves position and guards, relative seek uses the real end,
and both Write/WriteByte after EOF persist bytes. The incremental IO-only build is
warning-free. Reports: Sharp `build-probe/cna-repair-eof-before{,-build}.log`.
No EOF fix has been applied yet; these are intentionally failing pre-repair tests.

`FileStreamLayoutProbe.cpp` records the pre-repair Linux x86_64 layout: FileStream
size576/alignment8, Stream size8/alignment8, FileStream neither copyable nor movable.
No layout change is authorized by this measurement alone: any eventual native-handle
implementation must also preserve ownership traits, add before/after pins, document
full-consumer rebuilding and pass the dependency's full gate.

The Win64 MinGW cross-build also succeeds without warnings. Its compiler class
layout records FileStream size536/alignment8 and Stream size8/alignment8
(`repair-file-stream-layout-win64-static*`). The attempted executable run was
rejected with Bad EXE format by the existing32-bit XNA Wine prefix; that failure
is retained in `repair-file-stream-layout-win64.log`. This is compile/layout
evidence only, not Win64 runtime coverage; no additional Wine prefix was created.

## Isolated candidates while lifecycle binaries rebuild

Production sources remain unchanged during the lifecycle external build. A copied
FileStream.cpp candidate clears only benign EOF/fail state after returning the
actual short-read count, preserving badbit and non-EOF failures. It passes3/3 new
tests and all723 current IO tests (720 baseline +3), zero skips/failures,2.233s.
Strict candidate build passes. The first harness linked only CNA's exported subset
and lacked File::ReadAllText/WriteAllText; the next tried a nonexistent Sharp compile
database. Both failures are retained. The corrected harness uses current exported
header paths and the actual Sharp IO target's complete static dependency/object list.
Reports `repair-eof-candidate-*`. This is candidate evidence, not a production fix
or an18,140-test post-fix full dependency gate.

A copied StorageContainer.cpp candidate uses one shared internal disposed guard
before property/path/list validation or filesystem effects. It leaves IsDisposed,
GetTypeName and repeated Dispose available. All three new native tests pass(3ms),
without changing public layout/API or bypassing actual container acquisition.
The same candidate also passes all17 native storage tests (14 existing containment/
exception controls +3 new lifecycle regressions,10ms). Reports
`repair-storage-lifetime-candidate*`; no151 closure is claimed.

## Measured FileShare platform split; no expectation relaxation

The identical `FileSharePlatformOracle.cs` direct144-pair matrix now runs in both
reference environments. Framework4.0.30319.1/Win32NT opens25 and refuses119,
zero Windows-contract mismatches. .NET8.0.31/Unix opens81 and refuses63:56 cases
differ from Windows, while every exclusive/None conflict is still refused. The
Unix measurement deliberately exits1 for those mismatches; it is not relabeled
as Windows parity. This agrees with the inspected modern Unix implementation's
coarse advisory exclusive/shared locking. Original XNA expectations and all demo
assertions remain unchanged. The production sharing implementation is not chosen
or applied by this measurement; it must distinguish actual platform semantics
from the existing defect that ignores sharing altogether.

Reports: demos `repair-file-share-platform-xna*`; Sharp Runtime
`build-probe/cna-repair-file-share-net8-corrected*`. Installed .NET8 SDK references
were used offline, one build job. The initial standalone project's late intermediate-
path property emitted MSB3539; the corrected early SDK-import configuration builds
with warnings treated as errors,zero warnings/errors. Original warning/run logs
remain retained. Both platforms' exact new fixture files were removed.
