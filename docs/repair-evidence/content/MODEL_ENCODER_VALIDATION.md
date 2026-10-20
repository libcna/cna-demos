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
