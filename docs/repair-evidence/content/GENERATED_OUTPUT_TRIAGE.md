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
