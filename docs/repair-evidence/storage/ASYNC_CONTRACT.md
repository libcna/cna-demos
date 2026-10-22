# StorageDevice Begin/End contracts — museum finding

This is a focused part of the museum's storage-contract finding, not closure of
CNA-DEMOS-ISSUE-010. Empty filesystem paths, non-empty-directory errors and the
separate ContentManager finding remain outside this repair.

## Reference and root cause

`StorageDeviceAsyncOracle.cs` exercises the real Microsoft XNA 4.0 Storage assembly
in the existing Framework 4 Wine reference environment. It opens no successful
container and creates no application storage directory. Both the original run and
the strengthened exact-ParamName run pass **46/46** checks. The assembly identity
and loaded GAC path are printed in the retained reports.

The shipped `StorageDevice` IL validates player before size, permits players 0–3
and sentinel 255, and does not reject a negative directory count. Matching End
calls are one-shot even when they fail; a wrong result kind is not consumed.
Container End verifies the originating device and captured player before checking
the display name. Synchronous callbacks may call End themselves.

CNA's FNA-style result objects lacked consumption and owner state; selector
arguments were unchecked. Repeated End constructed additional objects, wrong
owners were accepted, and sentinel 255 selected `Player256` instead of shared
`AllPlayers` storage. The repair retains the synchronous API and adds the missing
checks at their original XNA ordering points. The empty-name check precedes root
resolution, avoiding filesystem access for that invalid argument.

Two unusual XNA diagnostics are intentional: an empty title's ArgumentNullException
uses `You must pass in a non-null title name.` as ParamName, and a wrong owner's
ArgumentException uses `result` as Message and the matching-Begin resource as
ParamName. The oracle verifies those properties; they were not replaced with more
idiomatic but incompatible diagnostics.

## Validation

Base CNA: `fb713de84` (haptic audit follow-up approved separately).
Repair CNA: **`3798787bbe05ddd79559f5313d8690971f0ea33e`**.

- Native before: **8 FAIL / 1 PASS** in nine new permanent regressions; all **18**
  prior storage tests pass. Report prefix `repair-storage-async-before-v2`.
- Genuine reference: **46/46 PASS**, including the strengthened ParamName checks.
  Prefix `repair-storage-async-reference-v2`.
- Native candidate: **9/9 focused**, **27/27 full storage** pass; build passes.
  Prefix `repair-storage-async-after-v1`; peak memory 485,470,208 bytes, no OOM,
  no source drift during validation.
- External consumer build passes without diagnostics. Nine scenarios × two renderers
  × before/after give **32 PASS / four unchanged ISSUE-152 failures**; no unexpected
  status/check-count/knownIssue/exit changes. The selector's six new checks increase
  its count from 6 to 12 on each renderer. C children retain 38 save-game and 35
  member checks. Prefix `repair-storage-async-external-v1`; peak 4,301,287,424 bytes,
  no OOM or source drift. The memory-high threshold throttled the single-job build.
- Actual storage translation units and the nine permanent regressions pass
  **ASan/UBSan/LSan 9/9**, without suppressions. Prefix
  `repair-storage-async-sanitizer-v3`; peak 451,665,920 bytes, no memory-limit events
  or source drift. SharpRuntime dependencies are linked from the existing native
  archives; this is not whole-library instrumentation.
- Both `cna_demos.cli.catalog_json` and `cna_demos.cli.validate` pass with
  `--no-tests=error`, using the consumer build's own shared library.

Retained harness failures: external-v1 initially selected zero catalog tests with
an incorrect regex; that invocation is not coverage. Sanitizer-v1 linked both the
instrumented storage sources and `libcna.so`, causing duplicate static-string
destructors and a shutdown double-free after nine passing assertions. Linking only
the actual storage sources plus their native runtime archives fixed the harness.
Sanitizer-v2 then passed, but its catalog command inherited the wrong multi-renderer
DSO and failed a CRTEffect symbol lookup. Version 3 uses the consumer DSO for that
command and passes both gates. No product code or test oracle was changed to hide
these failures.

The selector demo adds six exact-type assertions; it removes no existing assertion
or knownIssue marker. The C API's separately documented argument policy is unchanged.
No hardware, non-Linux runtime, visual, or full-CNA sanitizer coverage is claimed.
