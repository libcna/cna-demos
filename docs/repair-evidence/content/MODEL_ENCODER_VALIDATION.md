# ISSUE-109: model encoder / decoder validation symmetry

## Final validated repair

Resolved by CNA commit `68e0acb67732efdb8a0b737ab6f43181444dee65` on2026-10-20.
The localized encoder repair and native regressions are committed; the decoder,
schema and valid serialization layout remain unchanged. All five historical
invalid-input families now fail at the encoder with ContentLoadException.

Native final gate5396 exits0: focused54/54 and full10798=10697PASS/101unchangedSKIP
in1,148,296ms. Independent XML comparison preserves every10791 predecessor
outcome and admits exactly the seven new passing regressions. Two duplicated
child-process log records are not counted as extra tests; XML has10798 unique cases.
The private-display, owner-UID, zero-capability, broadcast-capable scope peaks at
2,598,830,080 bytes with zero high/max/OOM events. The full configured build passes
with138 retained legacy/vendor warnings, not a warning-free result.

Actual encoder/decoder production TUs and both native test TUs pass61/61 under
ASan/UBSan/leak checks; linked dependencies are not fully instrumented. The
completed external18-stage gate retains both719 renderer outcomes,95 math demos
per renderer, six zero-pixel references,146 focused scenarios and CTest684PASS/
48unchangedSKIP/three disabled. The strengthened model demo retains29 controls
and the same five invalid inputs; its obsolete restriction was removed only
after passes. There is no knownIssue/expected-failure delta for this already
passing diagnostic demo.

Independent closure audit58861 exits0 and rechecks330 current file hashes,
including319 external artifacts, four native binaries and all four instrumented
translation units. Report: `repair-model-encoder-closure-audit.json`, errors empty.
Earlier failed runs remain preserved below; they were not converted to expected
failures. All following baseline/candidate/running-state descriptions are
historical checkpoints, superseded by this final result.

## Historical baseline and local candidate

The public `EncodeModelToCnb` contract requires `ContentLoadException` for
internally inconsistent models, including out-of-range indices. This is a
CNA-specific format contract, not an XNA-reference claim.

Seven new native cases extend `CnbModelCodecTests.cpp` without changing its 28
existing cases. Against the current stable libraries all 28 original cases pass
and all seven additions fail. The valid controls within those additions pass:
root parent `-1`, valid hierarchy, valid 16/32-bit indices, all alpha modes 0..2,
coordinate sets 0/1 in every slot, and an empty vertex/index buffer. Invalid
controls cover negative sentinels down to INT32_MIN, both index widths including
nonzero high bytes and UINT32_MAX, alpha modes 3/UINT32_MAX, coordinate sets
2/255 in every slot, and nonempty indices into a zero-vertex buffer. Refusal
checks require the exact public exception type and verify input data is not
mutated.

The uncommitted candidate changes only `CnbModelCodec.cpp`: reject invalid parent
sentinels; validate every decoded little-endian index after the existing
byte-count/width checks; validate material ordinals before writing them. The
decoder, schema, writer layout and valid serialization path are unchanged.
There is no decoder round trip inside the encoder and no added GPU dependency.

Actual production TU plus all native codec tests: **35/35 PASS** both normally
and with ASan/UBSan/leak detection. Linked dependencies and GoogleTest are normal,
not fully instrumented. Both builds use C++23/Wall/Wextra/Werror and emit no
warnings. Exact before/after comparison confirms the 28 previous outcomes and
only seven expected improvements, with identical test-source hashes. All three
earlier native checkpoint binary hashes were preserved during these isolated
probes.

Reports: `build/linux-full/reports/repair-model-encoder-{before,after,sanitizer}*`
and `repair-model-encoder-focused-comparison.json`.

## Canonical build and focused downstream validation

Native incremental build succeeds with542 printed tasks, zero warnings and
unchanged production/test source hashes. Canonical codec focus is35/35; a new
full10798-case native gate is running against four frozen binaries and both
source files, requiring all10791 previous outcomes plus seven new passes.

Canonical external build succeeds with229 printed tasks and zero warnings.
Both73 content scenarios match the previous complete XML checkpoint exactly:
69PASS/four unchangedFAIL per renderer. The actual model demo passes29/29 on
SOFTWARE and OPENGLES3. Its same five malformed inputs must now throw exact
ContentLoadException from the encoder before returning a container, instead of
confirming the historical wrong behavior. A source-transform audit proves that
all29 controls and five inputs are retained, with only that stronger assertion,
its helper name and explicit exception include changed.

Only after those passes, the now-obsolete109 restriction sentence was removed;
all other restrictions remain. The metadata-only rebuild succeeds without
warnings. A post-metadata146 focus and then full both719/math/visual/CTest gate
are running. There is no109 knownIssue status transition: this was already a
passing diagnostic demo. The full comparison must preserve all previous demo
outcomes/check counts/failing assertions, not invent an expected improvement.

The earlier XML/content checkpoint is complete, with five other findings closed;
it does not validate this new candidate. ISSUE109 remains open until its own
native/external full audits complete. Production/test files are not yet
committed; final closure must record their eventual repair commit.

## Full native gate exposed a fixture defect and discovery contention

The first native full run (95074) completed with **10,798 cases: 10,690 PASS,
101 unchanged SKIP, seven FAIL**. All four binary and two source hashes matched.
This is a failed gate, retained as `repair-model-encoder-native-full-audit.json`,
not a completed repair.

`CnbHardeningTest.AnIndexAddressingAVertexThePartDoesNotHaveIsRefused` constructed
its malformed decoder input through the encoder. The new correct encoder refusal
therefore escaped before the decoder check. Review also found that the neighboring
32-bit test could silently pass from that earlier encoder exception, without
reaching the decoder. Both fixtures now encode a valid model, replace the index
chunk through `CnbWriter` (including valid container checksums), parse it outside
the decoder refusal assertion, and independently assert the encoder refusal.
The 16-bit decoder diagnostic checks and 32-bit high-byte malformed input remain.
The decoder implementation and all tolerances are unchanged. This adds a third
uncommitted CNA test source; it does not change the external tested runtime.

Six `ENetDiscoveryServiceTest` cases failed in the same full run. The native XML
places that suite at `2026-10-20T19:44:59.439`; the concurrent SOFTWARE net report
finished at `19:45:01.537` local time. Both use UDP discovery port 61190. The
partial-train test received connect port 58650 instead of its own 60342, direct
evidence of a different host answering. The unchanged binary then passed all
**13/13 discovery cases** in a fresh private Linux user/network namespace with
loopback enabled (74752). No process was stopped, no retry loop or sleep was
added, and no discovery implementation or assertion changed. Reports:
`repair-model-encoder-discovery-isolated.{log,xml}`.

The next full native gate will run in that private network namespace as well as
the usual private graphics display, to prevent collision with the concurrent
external suite. It must still match all 10,791 predecessor outcomes plus seven
new passes. This is environmental isolation, not physical-LAN validation.
The initial fixture rebuild failed on a missing required `Parse` filename;
that compiler error is retained in `repair-model-encoder-hardening-native-build.log`
and the explicit filename is corrected in the candidate.

The external SOFTWARE719 sweep is complete with627PASS/41FAIL/48SKIP/three
manual cases and exact local/original comparisons. OPENGLES3/CTest8261 is still
running on its original319 frozen artifacts and three unchanged source hashes.
ISSUE109 remains open; no successful final native gate is claimed.

The corrected canonical native build (61320) now completes without warnings.
Private network/display gate86497 passes its54-case codec/hardening focus and is
running the full10798 comparison. Its freeze covers four binaries and all three
changed native sources. A separate61-case sanitizer build52903 additionally
instruments the actual model decoder and includes every test from both source
files; results are pending, not yet a sanitizer pass.

Both external719 sweeps now complete with exact original and local comparisons:
SOFTWARE627/41/48/3,OPENGLES3 666/43/7/3. Final serialCTest8261 remains running.

Build-policy audit: although the canonical build invocation exports
`CCACHE_BASEDIR=/rv`, the existing native CMake cache stores a compiler launcher
that overrides it with `/rv/data/development/github.com/libcna`. The shared cache
directory is `/rv/cnaccache`; the actual normalization base was not `/rv`.
Correct `CNA_CCACHE_BASEDIR` and both persisted C/CXX launchers explicitly before
the next build after the active artifact freezes. The isolated probe compilers
invoke ccache directly with the requested `/rv` environment.

Extended sanitizer52903 now **COMPLETE0:61/61 PASS**. The actual encoder and
decoder translation units and both test files are instrumented with ASan/UBSan,
with leak detection enabled; other linked dependencies are normal. Strict
C++23/Wall/Wextra/Werror compilation emits no warnings. The audit confirms four
unchanged frozen native artifacts and three unchanged source hashes, zero
sanitizer diagnostics and zero failed/skipped cases. Reports:
`repair-model-encoder-hardening-sanitizer-{audit.json,build.log,run.log}`.
The two full regression jobs86497/8261 are still running.

## Correct the namespace credentials without changing the permission test

While86497 is still running, its
`ContentPipelineCliTest.MultiOutputFailureLeavesTheOldManifestAndRecoversSafely`
has failed: mapping the invoking user to root retained namespace capabilities,
so the compiler could write the deliberately non-writable fixture directory.
This is an unsuitable test environment, not a reason to change the test.
That full run remains running and its eventual results must be retained; it
cannot be used as a passing checkpoint.

A focused unchanged-binary run46590 uses `unshare --user --map-current-user
--keep-caps --net`, enables only that namespace's loopback, then executes through
`setpriv --bounding-set=-all --inh-caps=-all --ambient-caps=-all`. Its recorded
UID/GID are1000/1000 and all five capability sets are zero. The original publication
permission test plus all13 discovery tests pass **14/14**, without source,
assertion, timeout or retry changes. Evidence:
`repair-model-encoder-network-user-isolated.{log,xml}`.

After86497 actually terminates, run
`build-probe/repair-model-encoder-usernet-native-gates.mjs` under this corrected
namespace and the usual private graphics display. The new driver refuses a UID
different from the repository owner or any retained capability, checks all four
binary/three source hashes against86497, and still requires all10798 exact
outcomes. Do not start simultaneous native full runs sharing the genuine XNA
oracle build directories. No process has been stopped.

## External final checkpoint completed after resume

External8261 is **COMPLETE0**. All18 driver stages exit0. CTest has735 registered,
732 enabled:684PASS,48 unchangedSKIP,three disabled,964.43s. The audit compares
all735 outcomes directly with the completed XML checkpoint as well as the earlier
effective-parameters checkpoint. All319 frozen artifact hashes and three source
hashes match before/after validation.

Both719 renderer sweeps match the XML checkpoint exactly, including check counts,
knownIssue metadata and failing-assertion multisets: SOFTWARE627PASS/41FAIL/48SKIP/
three manual,OPENGLES3 666/43/7/3. Original-baseline comparisons still have exactly
70 status improvements and no unexplained delta. Both95 math comparisons, all146
post-metadata content controls (model29/29 per renderer), catalog/metadata and
three Python test suites pass. All six visual references differ by zero pixels
at zero tolerance. This does not claim a new expected-failure reduction for109,
whose formerly diagnostic demo was already PASS.

Independent summary: `repair-model-encoder-external-checkpoint-audit.json`,
errors empty. External artifact freeze is released, but no new heavy job starts
while the surviving native full job86497 is active after OOM. That run already has
the documented namespace-capability failure, so ISSUE109 remains open.

Before the next final native run, after86497 actually ends, correct the persisted
native ccache base/launchers and rebuild with one worker. The external CMake cache
already has `/rv`; only the native cache needs correction. Record any resulting
native binary-hash changes rather than pretending old frozen binaries still
match. Then run the complete unprivileged usernet gate with unchanged source
hashes and all10798 expected outcomes. Do not omit the permission case.

## Completed invalid-environment gate and corrected network control

Native86497 completed with10,798 cases:10,688PASS,105SKIP,fiveFAIL. Its complete
audit is retained, including all four additional skips. Four permission-sensitive
cases skip under the mapped-root credentials, and the original publication case
fails because those credentials bypass its removed write permission. The other
four failures are real two-process discovery/migration tests: the isolated
loopback-only network had no route for limited broadcast255.255.255.255. The
earlier single-process discovery control did not exercise that missing capability.
This failed run is not a passing regression checkpoint.

Corrected unchanged-binary control83742 passes all26 cases:13 discovery,
eight real two-process cases, and all five permission-sensitive cases. The private
namespace preserves UID/GID1000, drops all capability sets before executing tests,
and adds a dummy interface192.0.2.1/24 with an explicit limited-broadcast route.
No host network configuration is changed. No production/test source, assertion,
timeout or retry is changed. The audit verifies all four original native binaries
and three source hashes. Reports:
`repair-model-encoder-broadcast-usernet-focused.{log,xml}`,
`repair-model-encoder-broadcast-usernet-focused-audit.json`.

The focused process ran in its own systemd scope with verified MemoryHigh4GiB,
MemoryMax6GiB and MemorySwapMax0. After both original full jobs had terminated,
cache-policy build61683 started serially with one compiler worker under those
same verified limits in scope`cna-repair-model-cache-policy`. These limits apply
to the launched workload, not to the Codex application itself. The final usernet
driver now refuses a missing private broadcast route as well as wrong credentials.
Await the build audit before starting the final full gate.

The prepared final-native and generated-output-host drivers now also inspect
their own cgroup-v2 `memory.max`/`memory.swap.max` before launching any workload.
They refuse an absent/unbounded limit, a limit above6GiB, or nonzero swap.
Their actual guard prefixes were evaluated separately: both accept the private
`cna-repair-memory-guard-check.scope` with memory.max6442450944/swap.max0;
both refuse the ordinary unbounded scope with the intended diagnostic. These
four guard checks launch neither a compiler nor CNA tests. Both complete driver
files pass Node syntax checking. Their future audit records include the actual
cgroup and limits, rather than assuming the launch command enforced them.

The ongoing cache-policy build has emitted16 warnings from vendored Draco PLY
reader/writer headers (`-Wdeprecated`, implicit `this` capture under C++20).
No vendor source was edited. Keep these diagnostics in the complete build report;
the earlier warning-free incremental model build did not recompile this vendor
code and must not be confused with this broader configuration-triggered rebuild.

The live scope sample at2026-10-20T18:48:02.981Z records memory.peak4297031680,
memory.high4294967296, memory.max6442450944 and swap.max0. Its memory.events has
high307, max0, oom0, oom_kill0 and oom_group_kill0: the soft threshold was exercised
without reaching the hard maximum or killing a process. This is an interim sample,
not a claim about the eventual whole build. Retained data:
`repair-model-encoder-native-cache-memory-samples.jsonl`.

Later compilation of `XnaMaterialContentTests.cpp` adds two `-Wunused-result`
warnings at418/420 for ignored `nodiscard` getters, bringing the running total
to18. `git diff 688f3f23e -- <that test file>` is empty and both lines blame to
d2336a91cd, establishing that these are unchanged pre-campaign test statements.
They are retained, not suppressed or conflated with the16 Draco diagnostics.

The subsequent64-warning snapshot consists of16 Draco diagnostics and48 ignored
nodiscard-return diagnostics in tests. Six affected test files are unchanged
against688f3f23e. BoundingBoxTests.cpp and CurveKeyCollectionTests.cpp have later
campaign changes elsewhere, but the warning lines blame to75e94ed66 andcd211420e;
both commits are verified ancestors of688f3f23e. Thus all48 diagnostic call sites
predate this campaign and none belongs to the new model encoder/test changes.
The full exact diagnostic list remains in the build log and final build audit;
this is an interim count and does not change the test oracle or warning flags.

The reusable read-only provenance audit now records an86-diagnostic snapshot:
zero errors,70 owned-test warning call sites predating688f3f23e, zero newer owned
call sites, and16 vendor diagnostics. It retains every diagnostic, source hash,
baseline comparison and relevant blame ancestry in
`repair-model-encoder-native-cache-warning-provenance-interim.json`.
The checked driver `build-probe/repair-native-warning-provenance.mjs` can audit
the completed log under a fresh report prefix; it does not compile or change flags.
Call-site age alone does not prove every declaration/compiler option was identical;
the report explicitly records that limitation. This remains an interim snapshot.

The prepared final-native driver now records cgroup memory.current/peak/events
at start and after each stage, and rejects a new oom_kill/oom_group_kill event even
if a child exit status otherwise appeared successful. Its guard/snapshot prefix
was evaluated alone in cna-repair-memory-audit-check.scope:6GiB maximum,zero swap,
15,441,920-byte initial peak and all event counters zero. No CNA suite/compiler
was launched by that check; the whole driver passes Node syntax checking.

## Canonical cache-policy rebuild complete; final native gate running

Job61683 completed with configure/build exit0. The complete configured native
build took6,024,361ms with one compiler worker and persisted shared ccache base
`/rv`. Its audit records four resulting binary hashes and verifies all three
candidate source/test hashes unchanged, errors empty. It retains138 diagnostics.
The final read-only provenance audit classifies122 owned-test call sites as
predating688f3f23e and16 as vendor; no compiler errors/newer owned call sites.
This is not a warning-free build, nor proof that every historical declaration or
compiler option was identical. Reports:repair-model-encoder-native-cache-build-audit.json
andrepair-model-encoder-native-cache-warning-provenance-final.json.

The corrected full native gate is now job5396 in a new memory-limited scope,
using private graphics plus the validated unprivileged broadcast network setup.
Its actual guard checks accepted the memory limit, owner UID, zero capabilities,
broadcast route, completed build audit and frozen hashes. Focused54/54 PASS,
exit0, audit errors empty. The full10798-case run has started but is NOT COMPLETE;
expected10697PASS/101unchangedSKIP. No ISSUE-109 closure is claimed yet.
