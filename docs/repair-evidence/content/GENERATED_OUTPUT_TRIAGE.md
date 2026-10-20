# ISSUE-047: generated-output repair and retained evidence

## Final repair and validation (2026-10-21)

ISSUE-047 is **RESOLVED / FIXED_VALIDATED**. CNA source commit
`a56383319578256459c95ff990fd6b7826fa2226` records generated-output provenance and
nested paths. Its parents `23a34f986219d8ad61c24f31ce902b0ceeacf5cb` and
`32cb1a4ed31b7c0c5a1b58c4341faf0df481e37c` repair old-cache output reporting and
Windows atomic self-copy, respectively. Six added native regressions protect
these repairs; the original11 context/manifest and six real-host tests are unchanged
and pass, including publication, no-op, tamper/deletion repair and ownership-safe clean.

Full native10804:10703PASS/101unchangedSKIP; independent XML preserves every10798
predecessor outcome plus six passes. External63473 completes all18 stages:
SOFTWARE628PASS/40knownFAIL/48SKIP/3manual, OPENGLES3667PASS/42knownFAIL/7SKIP/3manual.
Only047 improves against109; unrelated statuses/checks/failed assertions and all36
standalone internal labels are unchanged. Both original-baseline comparisons find
71 expected improvements, zero unexpected differences. Math95 per renderer,six
pixel-exact references and CTest684PASS/48sameSKIP/three disabled (763.03s) pass.
Independent closure59379 verifies341 current files, including319 external artifacts,
and all evidence comparisons; errors empty. Final external scope peak1,132,859,392
bytes,zero high/max/OOM events. No unresolved full-regression gate remains for047.

Actual-helper ASan/UBSan/leak fixture5/5 and identical Linux/MinGW32-Wine copy
fixtures5/5 pass. This is not native Windows/MSVC, genuine-XNA execution or
instrumentation of every CNA dependency. Static XNA evidence is identified below.
No broad all-platform/all-renderer claim is made. The knownIssue047 marker and
previously resolved048 restriction are removed without changing the standalone
runtime assertions. The manifest demo explicitly adopts10/generatedOutput:false,
retains version8 rejection and adds9, preserving all14 checks. No failing gate
below is deleted, retried away or reclassified as success.

Reports live under `build/linux-full/reports/repair-generated-output-*`, especially
`-closure-audit.json`, `-native-full-audit.json`, `-final-gates-summary.json`,
`-ctest-final-audit.json` and both `-full-*-local-audit.json` comparisons.

## Retained candidate-validation chronology

The candidate/running statements below describe the progression to the completed
checkpoint above, not current jobs or remaining validation requirements.

The uncommitted candidate has focused and full native validation, with external
CTest validation still running: registration
accepts canonical configured-output files without input dependencies; the XNA
bridge preserves output-relative paths; manifest version10 persists a generated
provenance bit and fingerprints it separately from input bytes. Initial build20174
failed after102/687 tasks: canonical cache recovery succeeded, but BuildContent
reparsed the saved incompatible previous manifest and failed output reporting.
That log/audit is retained; no manual cache deletion bypasses this defect.
BuildContent now still requires a valid current manifest, while warning and using
empty previous state for an incompatible/corrupt old cache. A fifth new native
regression covers old/corrupt/future manifests, rebuilt-output reporting, no deletion
authority from invalid state and a subsequent no-op. Recovery52660 completed0,
with only two prior warnings. Focus56969 passed all11 context and six host cases
unchanged. Native115 gave113PASS/one unchangedSKIP/oneFAIL: the round-trip test
still explicitly expected version9. All new native cases passed. That version
assertion now pins10, also checks generatedOutput:false, and the old-version
rejection test adds9. No assertion is removed or relaxed.

The actual atomic-copy helper was independently probed with identical C++ source
on Linux and MinGW32/Wine: before, Linux5PASS but Wine2PASS/3FAIL (same-file,
lexical-alias and empty self-copy all Access denied). Its source reader remained
open when destination replacement ran. The local fix closes/checks that reader
before publication, preserving read-error refusal and atomic replacement. After,
both platforms5/5 PASS, strict warning-free builds, no temporary leaks, identical
probe hash. Native Windows/MSVC are not claimed. Reports are
repair-atomic-deployment-copy-{before,after}-audit.json; source is
AtomicDeploymentCopy.cpp. A sixth native regression protects the helper path.

Final candidate build76628 completed0:20 tasks,zero diagnostics. Focus13383 then
completed0:131 native cases give130PASS/one unchangedSKIP, including all six new
regressions; every previous selected native outcome is unchanged. The unchanged
11 context/manifest and six real compiler-host cases all pass. Strict fixture
builds are warning-free; frozen source/artifact hashes match. Peak1,212,870,656
bytes,zero high/max/OOM events; private temporary parents cleaned. Reports:
`repair-generated-output-native-final-build-audit.json` and
`repair-generated-output-final-focused-audit.json`.

External incremental build94535 completed0:347 tasks,609,391ms,one expected
incompatible-cache recovery warning,no errors,all frozen hashes exact. Peak
4,299,345,920 bytes,high2539,max/oom/oom_kill0. Initial demo focus76950 stopped
after16 SOFTWARE cases:15PASS/oneFAIL, the manifest demo's explicit version9
expectation. Original audit/logs retained. The deliberate format10 change now
has an explicit10/generatedOutput:false assertion in that demo, retaining old
version8 refusal and adding9; all14 checks and unrelated assertions remain.
No behavior assertion is relaxed. Follow-up build/focus use format-v10 prefixes.
Sanitizer57508 COMPLETE0: identical atomic-copy probe5/5 PASS under
ASan/UBSan/leak detection; actual helper instrumented,strict build clean,
source hashes exact,fixture tree cleaned,peak272,592,896 bytes,zero OOM.
Format-v10 build94348 COMPLETE0:11,205ms,zero diagnostics/errors,frozen hashes
exact. Format-v10 focus85104 COMPLETE0:146 cases,70PASS/three unchanged knownFAIL
per renderer; only047 improves. Manifest demo retains14 checks. Independent
comparison confirms all36 standalone internal labels unchanged on both renderers.
Peak2,929,029,120 bytes,zero high/max/OOM. The047 marker and stale048 restriction
are now removed locally,without changing standalone runtime code. Metadata build
6798 COMPLETE0:8,742ms,zero diagnostics/errors. Metadata focus96907 COMPLETE0:
146 cases,36 internal labels preserved per renderer,hashes exact;peak
4,295,487,488 bytes,high1450,max/oom/oom_kill0. Native56713 COMPLETE0:
10804=10703PASS/101sameSKIP,1,281,742ms; independent XML preserves every prior
outcome plus six new passes,source/artifact hashes exact. Private display/network,
owner UID/all capabilities zero verified. Peak2,546,384,896 bytes,zero high/max/OOM.
Current63473 runs full external alone in <=6GiB/private-Xwayland scope. Fourteen
stages pass,including both math95,six exact visuals and both full719 sweeps:
SOFTWARE628PASS/40knownFAIL/48SKIP/3manual; OPENGLES3667PASS/42knownFAIL/7SKIP/3manual.
Only047 improves against109. All unrelated statuses/check counts/failed assertions
and all36 internal standalone check labels are preserved. Original-baseline audits
each report71 expected improvements and zero unexpected differences. CTest runs;
the completed-stage memory peak is1,121,337,344 bytes,zero high/max/OOM events.
All319 artifacts and candidate sources remain frozen. Full external and final
closure audit still required; no issue closure or commit.
The historical before-repair failures below remain preserved, not reclassified.
Validation drivers: repair-generated-output-demo-focused.mjs
compares all146 content runs against109 and permits only047's improvement;
repair-generated-output-native-full.mjs preserves all10798 predecessor native
outcomes plus six new passes and requires owner UID/zero capabilities/private
broadcast networking; repair-generated-output-full-gates.mjs performs math,
six visual references, both719 sweeps and735 registered CTests with source/artifact
freezes. repair-atomic-deployment-copy-sanitizer.mjs instruments the actual helper
and unchanged five-case fixture with ASan/UBSan/leak detection.
Initial demo focus failed as recorded; sanitizer57508 and format-v10 focus85104
passed,as did final metadata focus96907 and full native56713. External full63473
is running. Never overlap its heavy workload. The additional closure audit
driver is syntax-checked only and must run after the complete external checkpoint.

## Retained host baseline (before this candidate)

The real compiler fixture was compiled and run against the canonical binaries
of the validated CNA `68e0acb67` repair checkpoint. Six Python scenarios yielded
**five FAIL/one PASS**. Each of the five valid generated-output scenarios fails
at the original outside-source-root authorization guard; the invalid-input
control passes. Nested-output ownership, publication, incremental repair and
cleanup assertions after that guard remain **not reached**, not validated.
No ISSUE-047 production fix or closure is claimed.

`GeneratedOutputCompiler.cpp` registers actual importer/processor/writer objects
with the real compiler host. The strict C++23/Wall/Wextra/Werror build is clean.
`generated_output_cli_tests.py` retains its normal failing exit1; the baseline
auditor accepts only the exact five failure names, five original diagnostics,
one passing control and unchanged canonical artifact/source hashes. No assertion,
timeout, expected value or production code changed between baseline attempts.

Initial job51013 correctly retained the five failures but its auditor incorrectly
required the private TMPDIR parent to be completely empty. CNA deliberately
retains `cna_content_staging_v1`; the destructor removes each reservation, not
the shared root. Inspection found only that owned non-symlink directory, empty.
The corrected auditor removes only this exact owned empty directory with rmdir;
any reservation, symlink or other leftover still fails. Initial audit/logs remain.
The initial empty directories were subsequently removed with nonrecursive rmdir.

Corrected job92605 exits0: strict build0, Python1, exact5FAIL/1PASS, no audit
errors, four canonical binaries/three ISSUE-109 sources unchanged, private parent
cleaned. It used the existing build-probe, shared ccache/base, one compiler and
MemoryHigh4GiB/Max6GiB/Swap0; no build occurred in the temporary fixture tree.
Reports: `repair-content-generated-output-host-before-corrected-*`, including
the complete audit; initial reports retain the `...-host-before-*` prefix.
Fixture SHA-256: `586983c9e7cce8ff834ba77222fe6334a13aa64ac0e69c679a7c4af629e59d1f`;
Python SHA-256: `d9c8d09c8f33bcbff7908a0141aab5fb93cf8269321b4202e005b7f020fa234a`.

Next implement output-root provenance and path preservation through canonical
registration, XNA bridge and manifest/fingerprint flow, retaining all input-root
capability and authored-file guards. Re-run these same six host cases and the
11-case context/manifest suite; they must become passes before closure.
The older preparation/running-state descriptions below are historical evidence.

## Historical context baseline

Current CNA `69afba262` still reproduces the finding. The isolated native
`ContentGeneratedOutputTests.cpp` runs the actual XNA bridge and canonical
processor context against the frozen libraries used by the ongoing ISSUE-109
regression gate. No production implementation is replaced.

Seven cases give **five passes and two failures**. Both legitimate output-root
registrations fail with the original `std::invalid_argument` identifying an
unauthorized external source: the ordinary separate output directory and its
equivalent canonical spelling. The source-contained deployment control passes.
Unrelated outside files, an output-prefix lookalike, an escaping symlink and
outside access with an empty output-directory setting are all refused.

The C++23/Wall/Wextra/Werror build is warning-free. The test process intentionally
returns 1; its driver returns 0 only after verifying the exact two failures,
five passes, four unchanged native binary hashes and two unchanged ISSUE-109
source hashes. All private fixture directories are gone. The symlink control
was actually run on Linux; no Windows filesystem validation is claimed.

Reports are `build/linux-full/reports/repair-content-generated-output-before*`.
These CPU context tests supplement, rather than replace, the existing real
registered-processor demo. They do not yet prove publication, cleanup, cache
invalidation, genuine XNA execution or a repair.

## Root cause and repair boundary

`XnaBridgeProcessorContext::AddOutputFile` forwards to
`ContentProcessorContext::AddDeploymentFile`, which accepts source-contained
files or an exact dependency resolved through a named external input capability.
It does not authorize a file generated in the explicitly configured output root.

Allowing that root in this one check is insufficient:

- `MakeContentBuildManifestEntry` derives deployment provenance relative to the
  primary or named external **source** root.
- Manifest validation requires every deployment source to have a corresponding
  byte-hashed input dependency.
- Direct input fingerprint refresh has source-root capabilities, not an output
  root. Treating generated output as an input would give incorrect build/cache
  semantics.
- Both direct publication and project staging consume deployment input
  snapshots; existing publication copies those files to the output destination.

A correct local extension needs explicit generated-output provenance through
registration, manifest/hash checks, publication, incremental verification and
cleanup. It must retain existing input-backed media deployments, exact named
external capabilities, canonical containment and authored-source protections.
Compiled additional outputs are not a substitute: those require CNB/XNB schema
identity and cannot represent an arbitrary processor-written text report.

No production change or API/schema design is committed here. ISSUE-047 remains
**CONFIRMED_OPEN**. Next: minimize host-level manifest/publication/clean scenarios
and select the smallest correct output representation before implementing it.

## Manifest baseline and proposed minimal representation

The same permanent source now contains11 cases. The extended baseline52851
gives **eight PASS/three FAIL**: the original two bridge failures plus a direct
manifest failure for a valid processor result whose report is under the separate
output root. Constructing that result directly deliberately isolates the manifest
from the earlier registration failure. The compiled primary artifact is a real
CNB Curve, not arbitrary bytes. `MakeContentBuildManifestEntry` refuses the report
as escaping the source root, confirming the second root-cause boundary.

The new source-backed deployment control passes its actual byte-change
fingerprint and missing-input checks. Outside-root support and a deployment
destination overwriting authored source are refused, with authored bytes intact.
All seven earlier cases retain their original assertions. The new generated
case also specifies the required manifest round trip, root-relative identities,
no physical-path leak, no invented input dependency, unchanged direct fingerprint
when generated output is modified/deleted, and changed fingerprint when authored
input changes. Those post-registration assertions are not yet reached in the
failing baseline; no pass is claimed for them.

The first strict compile caught two omitted aggregate initializers for
`ContentDependency::sourceRoot`. Explicit empty aliases fixed the compile; the
failed log is retained. Corrected strict build is warning-free, all four frozen
native binaries/three frozen sources are unchanged, and private fixtures are
gone. Reports: `repair-content-generated-output-manifest-before-corrected*`;
initial compiler log: `repair-content-generated-output-manifest-before-build.log`.

Source inspection suggests a narrow representation without a new XNA API:

1. Preserve primary/named-input provenance and existing source-backed deployments.
   Only a canonical file under an explicitly nonempty configured output root
   gains generated-output authority; unrelated paths remain refused.
2. Distinguish output-root provenance in the persisted deployment record. Keep
   its source path relative to the output root and its source-root alias empty.
   Do not add a synthetic input dependency or external capability.
3. Version the manifest change, include provenance in the definition fingerprint,
   and validate generated records separately from input-backed records.
4. Retain output digest verification, publication snapshots, ownership-safe clean
   and authored-source protection. Existing publication/clean loops already
   enumerate deployment outputs, but require actual host-level tests before
   claiming the new route works end to end.

This is a proposed implementation boundary, not an implemented or validated
repair. The locally installed XNA SDK XML has an empty AddOutputFile summary,
so it supplies no additional evidence for path handling; the baseline expectation
is the explicit CNA public contract and real processor use case. No genuine-XNA
runtime claim is made.

## Prepared real-host fixture (not yet compiled or run)

`GeneratedOutputCompiler.cpp` registers a real importer, processor and Curve
writer with `RunContentCompiler`. The processor writes the supplied report payload
under the host's configured output directory, closes it, then registers it through
`XnaBridgeProcessorContext::AddOutputFile`. It neither replaces the host publisher
nor mocks the manifest/cache/clean implementation.

`generated_output_cli_tests.py <fixture-executable>` specifies six host scenarios:

- Directory publication, no-op with byte-identical manifest/artifact, rebuild of
  modified and deleted support files, then clean preserving input and unowned data.
- Changed generated filename collects only the obsolete owned report.
- Single-file compilation publishes its related report.
- A report in a nested output directory retains its path and is cleaned in place,
  without creating a flattened basename copy.
- Clean refuses an edited report without partially deleting other owned files.
- Invalid fixture input fails before any owned files are published.

The Python source passes an AST syntax check and contains six cases. The C++
fixture and all six runtime scenarios remain **NOT RUN**. Do not treat their
expected assertions as evidence yet or infer that a source-contained host layout
is supported: existing host guards deliberately prohibit self-discovering and
authored-source-overwriting layouts. The earlier 11-case context/manifest baseline
remains the measured baseline.

After the current canonical build/final ISSUE-109 gate, compile this fixture
strictly in the existing `build-probe` using the include/definition flags and link
libraries of the existing `cna_custom_content_compiler_example` target. Use the
shared ccache, one compiler worker and a bounded memory scope; do not overlap the
ongoing canonical build. Retain the original failing host run before production
repairs. These new fixture files are currently uncommitted preparation.

Prepared driver: `build-probe/repair-content-generated-output-host-before.mjs`
(syntax checked, NOT RUN). It requires a successful canonical cache-policy build
audit before compiling, reuses the actual custom-compiler target configuration,
uses streaming SHA-256 checks, and verifies unchanged canonical binaries/sources.
Its baseline expects five original root-authorization failures and the one invalid
input control passing; unexpected diagnostics, failures or leaked fixture
directories make the driver fail. That expectation does not suppress the Python
tests' failing exit status or establish a baseline until execution.

## Genuine XNA static evidence: preserve the recorded output path

The locally supplied XNA4 pipeline assembly was inspected with `monodis`, not
executed. Assembly SHA-256:
`1ea481da62eb9e66bad8589df83f173725e62d22362801e1264cd02347721c69`.
Source: the existing SDK References/Windows/x86 directory selected by CNA's
`tools/xna-pipeline-oracle/run-oracle.sh`; no assembly or proprietary IL is
committed to either source repository.

The concrete `XnaContentProcessorContext.AddOutputFile` (RVA0x22718) forwards its
complete filename to `BuildCoordinator.AddExtraOutput` (0x1147c). That method
records the result of the coordinator's relative-path conversion, with duplicate
checks. `BuildCoordinatorSettings.GetRelativePath` (0x1162c) requires an absolute
path and removes the configured root prefix when applicable; it does not reduce
the path to a basename. `GetAbsolutePath` (0x11600) recombines that identity with
the root. `BuildCoordinator.GetOutputFiles` (0x111b8) uses those complete paths
for extra outputs. `BuildItemCollection.DeleteStaleOutputFiles` (0x11e40) likewise
uses the recorded path when deleting obsolete owned outputs.

This establishes a static compatibility basis for the nested-output scenario:
preserving `Reports/route.txt` is necessary, not merely accepting registration
then copying/owning only `route.txt`. The current CNA bridge's unconditional
`path.filename()` would lose that identity even after canonical registration and
manifest authorization were repaired. Keep existing source-backed deployment
semantics and explicit input capabilities; this evidence does not authorize
arbitrary writes/deletions outside the configured CNA roots. The observed XNA
absolute-filename requirement is recorded, not claimed runtime-tested or repaired.

The eight-method extract is retained only in ignored reports:
`repair-generated-output-xna-reference.il` and `...-audit.json`, errors empty,
extract SHA-256`14f6d23436e2f3df591a38789c821b5ba6e9a725dc58e313e2266a192ad0547d`.
This is genuine-reference IL analysis, **not a genuine-XNA execution pass**.
