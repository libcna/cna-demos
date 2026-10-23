# Storage path regression review — 2026-10-23

`StoragePathErrorsOracle.cs` executes genuine XNA's public path methods against a
fresh isolated directory. Its constructor-free setup does not validate storage
selection or native container construction. The retained run
`build/linux-full/reports/repair-storage-path-reference-v1-xna.log` passed all
29 checks; its audit records successful compilation/execution and no OOM events.

The matching seven-case native fixture originally gave five failures and two
passes (`repair-storage-path-before-v1-focused.log`). Production repairs
`17c66a9d3` and `ac6be8f03` now satisfy those cases. The inherited fixture was
reviewed and committed as CNA `4dba781c3`, with all 55 current storage module
cases passing (`repair-inherited-before-Storage.json`).

Assertions preserve exact exception types and argument names, disposed-state
precedence, empty patterns without filesystem access, nonempty-directory
contents, missing-directory errors and UTF-8 directory deletion. No assertion
was removed or relaxed. This preserves regression coverage for already-landed
repairs; it does not close every part of historical ISSUE-010.
