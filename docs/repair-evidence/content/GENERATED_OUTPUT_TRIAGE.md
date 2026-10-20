# ISSUE-047: separate generated-output root baseline

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

`generated_output_cli_tests.py <fixture-executable>` specifies five host scenarios:

- Directory publication, no-op with byte-identical manifest/artifact, rebuild of
  modified and deleted support files, then clean preserving input and unowned data.
- Changed generated filename collects only the obsolete owned report.
- Single-file compilation publishes its related report.
- Clean refuses an edited report without partially deleting other owned files.
- Invalid fixture input fails before any owned files are published.

The Python source passes an AST syntax check and contains five cases. The C++
fixture and all five runtime scenarios remain **NOT RUN**. Do not treat their
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
