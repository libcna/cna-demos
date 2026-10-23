# Deferred application storage root — museum B11

2026-10-23. CNA repair **fffa552ad4b4567c772f2f293302b8b103a5b16f** requires
Sharp Runtime **31c7c08f1987a284ce1b84bf1c779ce236a6e1c6**; both were published,
dependency first. Demo baseline: 1295899bbaa2d657348a115b831f31b928d2df6c.

The owner-requested museum finding is distinct from the XNA selector contract:
`SetAppNameEXT` is a CNA extension and must configure identity without requiring a
writable filesystem. CNA previously created the root in this setter in order to
configure Sharp Runtime's fixed isolated-storage override. A lazy lower-layer
resolver now preserves per-application isolation even when System.IO is accessed
before CNA storage. The first real storage access reports an unavailable device.

The selector demo previously expected that exception from the setter itself.
Its original exception-type, message, inner filesystem cause and serialization
assertions are retained, now triggered by `GetStorageRootEXT`. One additional
assertion requires the setter to complete without accessing the unavailable root.
This deliberate extension-contract correction is not a relaxed error oracle or an
XNA change. Check count grows from 12 to 13; no knownIssue/restriction is changed.

Evidence under `build/linux-full/reports/repair-museum-storage-*`:

- CNA baseline storage: 29 PASS / 5 FAIL in 34. All five failures are in the
  inherited uncommitted StorageContainerPathErrorsTests.cpp, preserved untouched.
- Five new CNA tests all fail before the runtime fix, all pass after. Full after:
  34 PASS / the same 5 FAIL in 39. XML comparison changes no previous outcome.
- Sharp Runtime full build warning-free; complete suite 18,201/18,201 across 42
  executables, no failure/skip, with authentic XNA fixtures and private Yacht SOAP.
  Storage6/6 and IO.IsolatedStorage63/63. The initial run missing the fixture
  environment stopped on five skips; it is retained and not counted as a pass.
- Actual StoragePaths.cpp plus its six tests: ASan/UBSan/LSan6/6, no suppressions.
  This is not whole-CNA instrumentation or a platform-independent validation claim.
- Existing demos build/linux-full rebuilt successfully, one compiler, shared ccache,
  6GiB memory cap/no swap. Before correction, the selector is an unexpected failure
  on SOFTWARE, demonstrating the old timing expectation. After correction, all
  seven storage demos on SOFTWARE and OPENGLES3 give 6 PASS / 1 unchanged expected
  ISSUE-152 FileShare failure on each. Selector13checks; root5checks. Other counts,
  failure message and knownIssue values remain unchanged.
- Catalog JSON, validation and JSON parsing: 3/3 CTests pass. All CNA SDL/configure
  boundary audits pass without increased budgets.

No Windows/Android/browser storage runtime or full CNA regression pass is claimed.
No historical CNA-DEMOS ID is closed by this museum-only subfinding. ISSUE-152 and
the remaining museum StorageContainer error-path defects remain actionable.

Continuation authority/state: CNA `CNA_MUSEUM_REPAIR_HANDOFF.md`. Next batch is
StorageContainer argument/IO exception mappings; existing genuine reference evidence
`StoragePathErrorsOracle.cs` has 29/29 passing public-call checks against XNA4.
