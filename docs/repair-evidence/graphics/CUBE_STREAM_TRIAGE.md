# ISSUE-054 cube stream consumption

**Closed2026-10-22, FIXED_VALIDATED.** Final external3868 terminated2 with
parityAccepted:true:132=123PASS/8existing082FAIL/1SKIP, no identity/status/check/
marker/assertion deltas. All eight extra C++/C DDS runs PASS after final relinking:
C++5 original checks, C19 child checks per renderer, no054 marker. No source drift;
peak169512960 bytes/all memory events0. Existing082 failures remain unaccepted.
Product commit b6ab302cc06b028dedfc451838e97cc31bd0599a, assertions preserved.
The pending-language sections below are chronological evidence, superseded by
this closure and the actual terminal reports. All156 issue IDs remain inventoried;
this closes one code defect, not the full campaign.

## Validated product repair

CNA commit `b6ab302cc06b028dedfc451838e97cc31bd0599a` contains only the
stream-consumption fix, public ownership/boundary clarification,14 native tests
and handoff. The separate inherited011 TextureCube ownership hunks were excluded
with an index-only patch; no source was stashed or overwritten. Nothing pushed.

Native14642/v3 completed0: incremental fixture build4420ms/no new diagnostics,
focused14PASS1756ms, whole graphics3040=2950PASS/90SKIP39707ms, all154 selected
DDS/cube native tests PASS2599ms. Exact old3026 graphics identities/outcomes and
old selected DDS/cube outcomes retained; exactly14 new passes. No drift; peak
2062221312 bytes/all events0. The original broader build's four existing warnings
remain documented below. This is not a new whole11041 native run.

External63082 completed2 with parityAccepted:true:132 prior cases=
123PASS/8existingFAIL/1SKIP, exact status/check/marker/assertion-label parity.
Eight existing082 failures remain unaccepted, not a green suite. The original
DDS demo separately passes5 checks each on SOFTWARE/OPENGLES3/OPENGL33/SDL_GPU.
No source drift; peak169390080/events0. C API49361 passes the original
c_api.graphics.texture_volume_data on allfour:19 actual child checks each
(the wrapper JSON reports0), including in-memory DDS cube creation. No drift;
peak141025280/events0. No C-specific adapter workaround or behavior change.

No renderer/pixel algorithm or ownership representation changed; exact decoded
face/mip readback is verified. No new screenshot/reference or sanitizer run is
claimed. Full campaign gates remain due after all actionable issues.
Metadata64127 now verifies the sole054 marker/restriction removal, original
assertions on four renderers, registry and six enabled CTests; collect before closure.

### Final consumer and metadata validation

Consumer64127 built successfully113644ms/no diagnostics. Its deliberately strict
artifact gate stopped before tests because cna-content and libcna_c_api.so were
also relinked, not only cna-demos. Build log names allthree links; the entire source
manifest is unchanged apart from the exact approved demo metadata edit. This is
not a product/test failure. Peak4298334208 bytes, memory.high411 throttling events,
memory.max/oom/oom_kill0; the6GiB hard cap was never hit. No duplicate build.

Resumed metadata4477 verifies those exactthree regenerated artifacts against the
successful build and checks the catalog snapshot saved before rebuilding. Exactly
the054 knownIssue and stale restriction sentence changed; all719 IDs/other metadata
and all behavior assertions remain intact. Registry PASS; originalDDS5 checks PASS
on four renderers139/261/212/260ms. Six enabled CTests PASS/no skips7176ms:
SOFTWARE dds_streams/compressed_blocks/typed_payloads and CLI catalog/list/validate.
No drift; peak176869376 bytes/events0. Final external3868 repeats the prior132
outcomes and both C++/C DDS scenarios against the newly linked artifacts; results
pending, no final054 closure yet.

## Current reproduction and candidate (2026-10-22)

CNA6fd2d636d / demos9d2cdd0: unchanged original five-check demo reproduced
identical3PASS/2FAIL on SOFTWARE, OPENGLES3, OPENGL33 and SDL_GPU. Both failures
are the original short-read completion and prefixed truncation refusal checks.
Session87281 is terminal (handle missing, scope inactive/dead); complete audit
`build/linux-full/reports/repair-cube-stream-before-audit.json` has no source drift,
peak151216128 bytes and all memory events0. No repeated expensive baseline.

Native/evidence `TextureCubeStreamTests.cpp` adds14 cases: four complete payloads
(prefix0/16, maximum chunk1024/7), eight one-byte truncations (same combinations,
truthful length or EOF before reported length), read exception and empty guard.
All successful loads check all18 face/mip images exactly; all checks retain caller
stream ownership and position. No broad tolerance or retry. Compile33689 failed
two dangling-else warnings around gtest macros; explicit braces fixed the fixture
without modifying assertions. Both logs are retained. Corrected31610 strict
compile4870ms0/no diagnostics; baseline SW81ms/GLES455ms/GL33424ms/SDL1922ms:
each5PASS/9FAIL, all14 present, no skips/drift; peak474386432/events0.

The [Stream.Read contract](https://learn.microsoft.com/en-us/dotnet/api/system.io.stream.read?view=net-8.0)
permits positive short reads before EOF and reports the actual byte count.
Runtime Stream.hpp and BinaryReader.cpp implement that contract. The cube loader
is an extension, not an XNA4 DDS API; no fresh XNA runtime oracle is claimed.
Attempt to retrieve upstream FNA TextureCube source returned a cache miss; this
is not evidence of FNA behavior. Existing native tests require FormatException
for invalid cube data and preserve the empty ArgumentException guard.

Candidate reuses BinaryReader(&stream,true).ReadBytes(len), passing buf.size()
to DecodeDdsCube. The reader constructor validates readability; its destructor
with leaveOpen=true never closes the caller, including during exception unwind.
Runtime already handles current position, short reads and early EOF; no lower-layer
patch or copied loop is needed. The decoder and upload loops remain unchanged.
Session20639 is focused after-validation; collect before further heavy work.
No issue closure, repair commit or marker removal yet; native broad/external gates due.

## Original source audit (retained)

### Validation corrections retained (2026-10-22)

Focused20639 completed: candidate product build87233ms0/no diagnostics; strict
fixture compile2708ms0;14PASS/no skips each renderer and original5 checks PASS
each. Peak1375842304 bytes/events0, no drift. Native84067 built successfully
(197438ms), but its diagnostic gate stopped before tests on four existing
nodiscard warnings in unchanged ModelContentTypeReaderTests.cpp:481/482 and
CnjStockEffectTests.cpp:295/332, all EXPECT_THROW expressions. Review preserved
in repair-cube-stream-native-v2-diagnostic-review.json with source hashes verified
against HEAD; no warning suppression or warning-free native build claim.

The resumed1753 focused run correctly failed12/14 native cases: the new fixture's
namespace-scope palette copied dynamically initialized Color constants before
their translation unit initialized them, yielding zero colors and failing even
the last-byte255 fixture guard. This is a fixture initialization bug, not a product
regression. Moved the same palette into a function-local static initialized on
first use. All14 assertions, exact colors and guard remain unchanged. Original
demo already uses runtime-local colors and was not affected or modified.
Failed logs/XML are retained; no retries or altered tolerances.

Corrected focused43263: no-op product build180ms; strict compile2650ms;14PASS each
SW78ms/GLES435ms/GL33440ms/SDL1395ms. Original5 checks PASS each
130/229/223/255ms. No drift/events, peak501108736 bytes. Native-v3 now runs the
corrected fixture plus full graphics and selected DDS/cube paths. No054 closure yet.

2026-10-22, CNA30c872915 with the separate027 candidate and inherited011/082 work.
Read-only while native027/69993 is running. No054 runtime result, source repair,
test, commit, marker removal or closure is claimed by this audit.

The current TextureCube.cpp DDSFromStreamEXT still allocates total Length,
performs exactly one Read and passes total Length to DecodeDdsCube, ignoring the
returned count. The original demos/graphics/texture_cube/dds_streams.cpp retains
five checks: complete and prefixed complete images, seven-byte short reads, and
one-byte truncation at positions0/16. It verifies all18 face/mip images and keeps
both historical failures. Run it against the current libraries after027 validation.

The responsible Runtime operation already exists: BinaryReader::ReadBytes(count)
in sharp-runtime/modules/io/src/System/IO/BinaryReader.cpp loops over positive
short reads, limits allocation to remaining bytes when seekable, and trims at EOF.
Its public constructor accepts Stream* and leaveOpen. Reusing it with leaveOpen=true
can fix the caller without copying a partial-read loop or changing the decoder.
Preserve the current empty-stream argument guard and inspect current-position/EOF
and exception timing before implementing. Pass the returned vector's actual size
to the decoder; do not manufacture initialized padding. Verify stream ownership
and position as well as decoded pixels and truncation exception types.

Texture2D's own decoder already accumulates short reads and subtracts Position;
it additionally has an XNA-specific rewind-at-exact-EOF contract. Do not import
that separate image-loader behavior into the cube extension without evidence.
The cube extension currently documents only decoding from a DDS input stream.

Important staging boundary: TextureCube.hpp/.cpp already contain inherited011
ownership changes. Any054 repair must stage only its own exact hunks, preserving
and excluding011 changes, just as the003 Texture2D commit did. Native regression
tests should cover the original5 controls plus short-read prefix, truthful and
early EOF, and caller stream remaining open. No unavailable hardware is needed
for SOFTWARE/GLES readback reproduction.
