# ISSUE-150: retire the declaration-only runtime facade

Resolved by CNA `379bac6d3f76f452ffce7c7760149b2d1df30b95` on 2026-10-23.
Final disposition: `UNSUPPORTED_CONTRACT_CORRECTED`.

## Contract and decision

The permanent issue explicitly permits either implementing the five documented methods or
removing the nonfunctional facade from the public include tree. CNA `079a2fbe2` still has
`CNA/Misc.hpp`, but no definitions for `Runtime::Initialize`, `Shutdown`, `IsGraphicsEnabled`,
`IsAudioEnabled` or `IsInputEnabled`. A fresh link of the original
`tools/repro_runtime_facade.cpp` fails on all five symbols. The original source is retained.

No production source includes the header or uses its four `RuntimeOptions` flags. This is a
CNA-only facade, not an XNA type. The smallest correct repair retires the header and options;
it does not invent another subsystem-initialization model. `Game`, `GraphicsDeviceManager`,
the C game lifecycle routes, and the unrelated `CNA::Runtime` CMake target remain supported.
There is no replacement promising the old five-method contract. Consumers must remove the
obsolete include. No implemented method or exported symbol is removed.

The C inventory drops exactly its eleven obsolete declarations and their mapping/limitation
record. The historical CBIND-037E5 diagnosis is retained with a retirement update. Public C
documentation names the supported lifecycle APIs and explains the source-level removal.

## Regression protection and measurements

`CApi_RetiredRuntimeFacade` replaces the old absence-of-definition guard. It checks both that
the retired header is absent and that the runtime archive defines none of the five methods.
It permits the unrelated `CNA::Runtime::Testing` namespace. Running the new guard before
removing the header fails; running it afterwards against the real archive passes.

- Native `CnaRuntimeTests`: 248 cases, 244 PASS and the same four SKIP. Exact per-case status
  comparison with the ISSUE-019 checkpoint is unchanged.
- Native and complete cna-demos incremental builds pass. All 275 consumer binaries are
  SHA-256 identical to the last full ISSUE-019 checkpoint, including libcna and libcna_c_api.
  No implementation changed; the consumer build only rebuilt the generated shared-library
  anchor and relinked dependents.
- Catalog validation: 719 demos, zero problems.
- Fresh complete `game` category: SOFTWARE 25 PASS / 7 SKIP / 1 NOT RUN; OPENGLES3
  32 PASS / 1 NOT RUN. Status, marker and assertion counts match the prior checkpoint.
- Fresh complete `c_api.game` category on each renderer: 12 PASS / 2 unchanged known FAIL
  (ISSUE-050 and ISSUE-051). Status, marker, assertion counts and messages are unchanged.
- Clean-source inventory: 466 headers, 8093 declarations, 7009 implemented, 15 partial,
  626 planned, 443 not applicable. Shared worktree: 8102 declarations and 635 planned,
  reflecting inherited uncommitted work. The committed generated documents use clean source.
- Clean-source coverage/limitations/release checks and all 33 scope-model tests pass.
  The C API release verdict remains NOT READY; no outstanding binding is hidden.

The first renderer harness stopped because it requested nonexistent `c_api.runtime` after
the SOFTWARE Game category passed. The corrected catalog prefix is `c_api.game`; the failed
harness log is retained. The first eight-test native CTest selection passed all six audit/
retirement checks but could not run two smoke executables that had not yet been built in that
tree. Building both exposed an independent stale ISSUE-019 fixture assumption:
`RuntimeGameSmoke` reset the clock in every Update, then expected two fixed steps to draw.
The genuine XNA clock evidence already proves why this is invalid. Separate CNA commit
`8a21c594d` exercises reset once inside Update and consumes that pending reset before the
independent SuppressDraw checks. Reset success, callback reentry refusal and exact draw counts
remain asserted. Both native C runtime smoke tests then pass on OPENGLES3 (10.57s) and SOFTWARE
(0.35s). The initial failing log and the repaired runs are retained; no runtime change was made
to accommodate the old fixture. This gives six passing audit/retirement gates plus both smoke
tests on both renderers.

Reports are under `build/linux-full/reports/repair-runtime-facade-*`. The original five-symbol
failure is `repair-runtime-facade-before.log`. No historical reproducer was changed into a
successful consumer, and no knownIssue marker or behavior assertion was removed.

The last full renderer, pixel-reference and 735-registered-test external checkpoint remains
ISSUE-019, on byte-identical binaries. This batch does not claim another full smoke/visual run,
sanitizer run, or hardware/platform validation beyond Linux. Its six previously recorded
external CTest residuals remain open. This issue has no runtime demo whose FAIL should become
PASS: its resolution is removal of a nonfunctional public contract.
