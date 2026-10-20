# ISSUE-109: model encoder / decoder validation symmetry

## Current baseline and local candidate

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
