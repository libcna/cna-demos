# Prepared storage/runtime evidence (056, 151, 152; not repaired)

No CNA storage or Sharp Runtime production change is included here. The current
dependency is Sharp Runtime `f2aecfef`. These probes prepare the next coherent
storage batch while the preceding geometry checkpoint remains frozen.

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
