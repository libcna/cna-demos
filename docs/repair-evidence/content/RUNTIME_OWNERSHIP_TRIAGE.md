# Next content-runtime batch: source audit, not repair closure

## Latest dispatch candidate (2026-10-20,09:32 UTC)

Permanent `ContentValueDispatchTests.cpp` adds nine native counterparts of the
genuine18 value controls. Before:fivePASS/fourFAIL; combined with the previous19
reference controls,28=tenPASS/eighteenFAIL, every prior outcome unchanged.
Reports `repair-content-dispatch-values-native-before-braced{,-build}.log/.xml`
and `repair-content-dispatch-values-audit.json` retain this exact baseline.

The first production candidate restores the erased/shared callback, giving
12PASS/16FAIL with precisely those two improvements. The expanded candidate
centralizes typed invocation (including raw reads), checks requested type before
identity/ownership, rejects replacement existing references and boxed existing
values, distinguishes null references from existing instances, and dispatches
explicit reference readers through the serialized index. A common internal
serialized-reference trait replaces the collection-only classification and also
recognizes CNA's existing by-value wrappers of XNA reference types. It does not
use Sharp's stub reflection or redesign those wrappers' ownership/identity.

Actual ContentReader.cpp plus both regression TUs now pass28/28, exact comparison
errors[] (`repair-content-dispatch-complete-split-policy-*`). First compilation
had incorrectly applied the probe's extra Wextra/Werror to the production TU;
pre-existing unused renderer parameters stopped it. That log remains. The helper
now uses the actual production compile_commands flags for that TU and retains
Wall/Wextra/Werror for both new test TUs; this successful build emits no warnings.
No canonical build/full regression or087 closure is claimed yet. Sanitizers are
now also pass28/28 (actual ContentReader TU plus new tests, ASan/UBSan/leak checks,
prefix `repair-content-dispatch-complete-sanitizer-split-policy`, errors[]);
this is not a claim that all linked libraries were sanitized.
Manager fallback011 and by-value resource ownership remain
independently unresolved. Existing by-value reference wrappers retain their
C++ value-carrier semantics, not a fabricated managed ReferenceEquals identity.

The old native test `ReadRawObjectReadsIntoAnExistingInstance` used a value struct
and expected success. Genuine18 demonstrates the exact opposite for all four
existing-value overloads. Its replacement expects InvalidOperationException,
unchanged caller value and full payload consumption; direct reader bridge tests
remain unchanged. This is an evidence-based oracle correction, not a relaxed gate.

Date:2026-10-20. Current registry and sections SOFTWARE sweeps still reproduce
the documented runtime failures. No production change for these IDs is included
in the pending048 build.

## Concrete current-source findings

- 011: ContentManager::Unload only clears loadedAssets. ContentReader's typed
  RecordDisposable calls an explicit caller recorder, but the manager fallback
  remains a TODO. These are independently actionable from shared-table dispatch.
  Value-handle Texture2D is an additional lifetime concern: copies currently own
  independent wrappers around a shared renderer, and Dispose clears only that
  wrapper. Recording and disposing another copy is not sufficient to invalidate
  the returned handle. Do not close011 on a shared_ptr-only fix.
- 087: InnerReadObjectAny returns the result of ReadUntyped without recording it;
  ReadSharedResources uses exactly that route. The typed reader knows its carrier
  before erasure, so preserve disposable capability at that boundary instead of
  guessing a std::any payload type or adding demo-specific type names.
- 012: ordinary Load still requires the protected loadsThroughOpenStream opt-in.
  The cold ResourceContentManager failure needs an updated root-cause explanation:
  ReadAsset already calls BuiltinContent<T>::Register, but a family invokes
  RegisterDeclaredXnbReaders, which does nothing unless Game previously declared
  the built-ins. Existing family-closure tests explicitly establish Game's declared
  state and therefore cannot prove a cold standalone manager works. Preserve the
  per-type link boundary and explicit-removal semantics when repairing this.
- 043: CreateReaderForTargetTypeEXT still sends the associated target name directly
  to a factory table keyed by reader name. Distinct target/reader identities remain
  necessary in a minimized regression, unlike the existing same-name control.
  CNA explicitly documents fresh unique_ptr instances and a native test requires
  them, whereas genuine XNA caches readers process-wide. Do not silently change
  that documented C++ ownership surface as part of a target-name mapping repair.
  A factory's actual key is known when CreateRegisteredReader creates the table
  reader; any association must also respect factory removal, clear operations,
  constructor callbacks and lock order. Do not eagerly instantiate every factory
  merely to discover its target.
- 045: LoadXnbAsset invokes ParseXnbHeader outside an exception-wrapping boundary.
  Genuine XNA's stream/header and object-body boundaries wrap IOException in
  ContentLoadException; do not indiscriminately wrap caller callback exceptions.
- 039: both RootDirectory setters assign directly. Genuine XNA refuses assignment
  with any loaded asset, even assignment of the same root value.

## Reference preparation

`ContentOwnershipOracle.cs` now **passes35/35 against genuine XNA4.0**. It uses
the real XNA ContentManager and ContentReader with an authored in-memory XNB and
public custom type readers. Executed checks cover inline/shared callback order,
alias fixups, default ownership, partial-load retention, disposal-failure cleanup,
stream closure, virtual OpenStream routing, cache/root transitions and truncated
header/body exception boundaries. No GPU, online service or replacement manager
implementation is involved; only the public stream source is overridden. The
actual SDK framework DLL is used in the existing private Wine reference prefix;
the C#4/x86 warning-as-error build and run both exit0. Reports:
`build/linux-full/reports/repair-content-ownership-xna{-build,}.log`.

The local shipped-XNA decompilation additionally warns against an attractive but
incorrect assumption: RecordDisposableObject appends to its list without identity
deduplication. Unload calls each recorded disposable in order, stops on the first
exception, and clears both lists in finally. The genuine oracle now confirms this,
including three disposal calls when a custom reader returns one identity for
three separate materializations. A shared alias fixup itself adds no callback.
Caller recorder exceptions remain InvalidOperationException and do not transfer
ownership back to the manager; truncated header/body failures instead become
ContentLoadException. Source paths are under
`../xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework/`.

The local FNA checkout at1358793096d556388be798d24dc3728b2abb10ff differs in
these exact places: its RecordDisposable suppresses duplicate identities,
RootDirectory is an unrestricted auto-property, and Unload lacks XNA's finally
cleanup. CNA's owner policy says measured XNA wins. A future implementation must
record these specific departures from FNA in the required CNA divergence log;
do not substitute FNA's convenient deduplication for the measured35-check result.

Next: add corresponding native before/after tests. Keep011,012,039,
043,045 and087 open until their own repairs and full relevant gates finish.

## ISSUE-039 native baseline

Three native cases are now prepared in ContentManagerXnbTests.cpp and were linked
against the frozen048 dependency closure, without rebuilding checkpoint binaries.
Result:twoPASS/oneFAIL,3ms. Both lvalue and rvalue RootDirectory setters wrongly
accept same-root and different-root assignment while an asset is cached, and mutate
the root. Positive controls confirm cache preservation and reload after Unload;
failed Load and uncached ReadAsset correctly leave root assignment available.

The first Werror compilation exposed six pre-existing ignored-nodiscard results in
the reused fixture. Exception assertions now explicitly discard the value, and the
initial cached-load control is strengthened to assert its exact value. No exception
assertion is removed or relaxed. The corrected compilation passes Werror; the
original compile failure remains in `repair-content-root-native-before-build.log`.
Actual baseline results: `repair-content-root-werror-native-before.log`.
No039 production repair or post-fix pass is claimed yet.

## ISSUE-045 boundary and bounds baseline; candidate039/045 repair

Expanded genuine oracle passes89/89 (strict C#4/x86): all header lengths0..9
through Load and ReadAsset, stream closure, root mutation after failed loads, and
reader IOException versus InvalidOperationException are verified. The original
35-check run remains archived; the new report is `repair-content-envelope-xna.log`.
Four native envelope cases fail as expected in10ms against frozen048 dependencies:
short typed/raw/untyped headers leak EndOfStreamException, and the body boundary
also leaks IOException. User InvalidOperationException is already preserved.

An additional actual-TU ASan baseline confirms a **four-byte heap-buffer-overread**
in ContentManagerUntyped.cpp:154 when an external XNB declares a ten-byte LZX
container. The typed path checks the compressed14-byte prefix; the erased path
does not and constructs a four-byte MemoryStream at the end of the input vector.
The retained report `repair-content-envelope-sanitizer-before.log` identifies the
read immediately after the ten-byte allocation. This is a related045 parser-boundary
subfinding, not a fabricated runtime pass or a new historical issue ID.

The uncommitted candidate moves existing header/range checks into one private
ContentManager::ReadXnbHeader shared by both paths, adds IOException wrapping at
the manager header and typed/erased ContentReader asset boundaries, and preserves
the low-level ParseXnbHeader EndOfStream contract and non-I/O user exceptions.
Both RootDirectory setters reject a nonempty cache before assigning, matching XNA
rather than FNA's unrestricted property. The required divergence log is updated.

Actual ContentManager.cpp plus ContentManagerUntyped.cpp and the whole manager
fixture pass ASan/UBSan/leak **23/23 in29ms**, exit0, after the candidate. Eight new
native cases cover039/045; unchanged positive controls also pass. Report:
`repair-content-envelope-sanitizer-after.log`. The candidate has NOT yet entered
the canonical native build or external demos: both048 checkpoint binary sets
remain frozen. Neither039 nor045 is closed; complete native/external regression
is still required. Other ownership findings011/012/043/087 remain open.

Pre-build regression review identified an older SoundEffect asset test expecting
raw EndOfStreamException at ReadAsset. Its exact expected type is corrected to the
genuine-backed ContentLoadException; the truncated100MiB declaration and allocation
guard are unchanged. The whole-container and SoundEffect property fuzz tests no
longer accept leaked EOF from asset loading, making those gates stricter. Low-level
header/decompressor tests retain their own contracts. After the048 native consumer
and hash audit passed, canonical runtime build20911 started (1194 steps,parallel2).
No compiled source is to be edited during that build.

Additional011 source constraint: GraphicsResource's existing optional shared
identity is used by reference-valued state properties, but ordinary texture copies
deliberately remain independent wrappers. Its disposal broadcast currently marks
aliases disposed without invoking each texture wrapper's native-resource release.
Simply sharing that identity would therefore be insufficient to prove immediate
resource release at ContentManager::Unload. Preserve ordinary copy semantics and
validate both invalidation and actual ownership release; no graphics edits yet.

## Genuine dispatch contract:39/39

`ContentDispatchOracle.cs` now passes39/39 against genuine XNA4, with a strict
C#4/x86 warning-as-error build. Reports `repair-content-dispatch-xna{-build,}.log`.
Nine typed, erased, explicit-reader and raw-object modes preserve exact payload
and identity. Every newly materialized IDisposable is recorded, including raw
reads; an existing instance is not recorded. All four existing-instance overloads
reject a reader that returns a replacement, with exact InvalidOperationException
and no disposal callback. A null serialized type index returns null, not the
provided existing instance. A reader-produced null is accepted and not recorded.

This prevents an incorrect087 repair that merely records every erased return:
existing-instance, raw, null and identity rules must be handled together at the
reader invocation boundary. FNA's raw path bypasses recording, unlike this measured
XNA contract. These are reference measurements, not a claim that CNA already passes;
no011/087 implementation or closure is included in the039/045 build underway.

Two native regression sources are staged here, outside the canonical source glob
while039/045 builds: `ContentDispatchTests.cpp` (17 cases matching the39-check
dispatch oracle plus shared-resource and caller-exception controls) and
`ReaderTargetLookupTests.cpp` (eight cases for distinct target/factory identities,
fresh instances, lazy construction and registry withdrawal/replacement callbacks).
They are prepared, not yet compiled or executed. No test result is claimed.
The target-lookup tests deliberately preserve CNA's documented fresh unique_ptr
contract rather than importing XNA's process-global reader ownership model.
Dispatch uses matching factory/target names to isolate043 from its own failures.

`ContentOwnershipTests.cpp` adds seven further prepared, uncompiled native cases
from the measured ownership contract: default Unload/Dispose, cache identity,
caller ownership/order, per-materialization duplicate recording, partial-read
retention, failed-Unload cleanup and a throwing recorder. The test manager uses
the existing OpenStream opt-in solely to isolate011 from012; it is not a proposed
production workaround. These shared_ptr cases cannot close the separate value-
handle Texture2D ownership requirement. All32 prepared cases can be compiled in
one bounded probe once a worker and stable native dependency closure are available.

## Canonical039/045 build and next-batch baseline

The complete native configuration builds successfully (1194 scheduled steps).
All52 emitted warnings match the original152-warning baseline by normalized file
and diagnostic; no new warning. Four artifacts are frozen in
`repair-content-runtime-native-frozen.sha256`. The complete manager fixture passes
23/23; all1918 tests in172 content-module source files are running. The initial
focus filter selected only the21 XNB tests and omitted the two constructor tests;
all21 passed, but the inventory guard correctly failed. The corrected filter adds
those two tests, retains the expected23 count and preserves the initial log.
Reports: `repair-content-runtime-{manager-focus,manager-complete-focus}.*` and
`repair-content-runtime-{gates-driver,complete-gates-driver}.log`.

The next-batch32-case probe now compiles with Wall/Wextra/Werror and runs in8ms:
eightPASS/24 expectedFAIL. Dispatch17 gives fivePASS/twelveFAIL; lookup8 gives
onePASS/sevenFAIL; ownership7 gives twoPASS/fiveFAIL. Failures reproduce the missing
callbacks, wrong existing/null/explicit dispatch semantics, broken factory-key
resolution and absent manager disposal. Positive raw-existing/caller ownership
and exception controls remain passes. Native039/045 is already present in the
linked libraries; these are before results for011/043/087, not for039/045.
Reports: `repair-content-dispatch-lookup-werror-native-before{,-build}.log`.

The initial combined-source build failed Werror on generated GoogleTest classes
outside their anonymous-namespace fixture. Moving those test classes inside the
same namespace fixes linkage without suppressing warnings or changing assertions.
The initial `repair-content-dispatch-lookup-native-before-build.log` is retained.

Additional genuine `ContentValueDispatchOracle.cs` passes18/18, strict C#4/x86:
explicit readers bypass the dispatch index for value targets, unlike reference
targets; IDisposable structs are never recorded, including erased reads. All four
existing-value overloads throw InvalidOperationException because boxed identity
cannot be retained. This is measured XNA behavior, not a claim that CNA passes it.
Reports: `repair-content-value-dispatch-xna{-build,}.log`. Preserve these controls
when repairing the shared invocation boundary; the existing native value-instance
test expectation must be reviewed against this evidence rather than copied blindly.

## Full-module regression investigation

The first complete1918 gate finishes in262.712s:1912PASS/four unchangedSKIP/twoFAIL.
Both failures are retained in `repair-content-runtime-content-module-audit.json`.
The earlier math fix correctly rejects a negative BoundingSphere constructor input,
so one adversarial writer seed never reaches its writer. Assigning the same NaN
center and negative radius through the public fields restores the intended input;
the48-seed corpus and expected Written/Refused outcomes are unchanged.

The other failure is an older end-to-end Song test expecting raw FileNotFoundException.
Expanded genuine `ContentOwnershipOracle.cs` now passes91/91 and confirms that
this IOException subtype is wrapped at ReadAsset, while user exceptions remain
unwrapped and streams close. Report `repair-content-file-not-found-xna.log` retains
the new measurement; the previous89-check log remains. The end-to-end expectation
is corrected to ContentLoadException; the four direct Song reader tests keep their
separate FileNotFoundException control. Its renamed test has an explicit old-to-new
identity mapping in the baseline audit, not an ignored or newly accepted failure.

All four frozen hashes matched after the failed run; only then was that native
freeze released. The eight-step incremental build of these two fixture corrections
is warning-free. Complete manager/writer/song focus passes33/33 in146ms, including
48 adversarial seeds (32 refused/16 written) and400 generated mutations
(204 refused/196 written). Corrected1918 gate is running under prefix
`repair-content-runtime-corrected`; its four artifacts are frozen separately.
No whole-module pass or039/045 closure is claimed until it finishes.

Corrected module71175 now completes0 in188.735s:1914PASS/four unchangedSKIP of1918,
all1905 original outcomes (including the explicitly mapped Song test) and13 later
additions match, `differences: []`. All four hashes match afterward. The isolated
writer fixture correction is committed as CNA1ec77a734; runtime039/045 and its
regressions are CNA583af2a5e. Full native pipeline552 job66736 and external build6996
are running. Both issues remain FIXED_PENDING_REGRESSION, not closed. No new native
rebuild during the remaining pipeline/consumer/hash gates.

The full552 pipeline now finishes successfully in1212.106s:551PASS/one unchanged
XACT skip. The exact per-test comparison against048 reports `errors: []`, and
all four frozen artifacts match after the run. Broader1412 consumers also finish
with1408PASS/four unchanged skips in234.190s; runtime241 gives238PASS/three unchanged
skips11.223s. Default XML gives4PASS/two unchanged skips; with the original sample
fixtures all six pass in4ms. All four exact consumer comparisons have `errors: []`,
and all four final native hashes match. Native freeze is released. External039/045
is still building; no external pass is claimed and production edits must wait.

Three more043 compatibility controls are prepared alongside the original eight:
direct same-name factories, explicit manual factory-key associations and a factory
that constructs a reader but throws without returning it. These retain documented
CNA registration behavior and prohibit publishing an unsuccessful factory result.
The expanded eleven-case lookup probe compiles with Wall/Wextra/Werror and gives
fourPASS/sevenFAIL in0ms: all original eight outcomes remain unchanged and the
three new controls pass. Reports `repair-reader-lookup-before{,-build}.log`.
The earlier32-case combined baseline remains unchanged evidence, not overwritten
by this addition. No043 production repair is present yet.

`ReaderTargetLookupOracle.cs` now passes5/5 against genuine Microsoft.Xna.Framework
4.0 (strict C#4/x86, warnings-as-errors): the root Initialize resolves a differently
named item reader, preserves TargetType, resolves itself, and the implicit raw read
consumes the exact integer73 without a dispatch index. Repeated genuine lookups
return the same table reader. CNA deliberately documents fresh unique_ptr ownership;
the043 repair must retain that documented C++ ownership surface. Reports
`repair-reader-lookup-xna{,-build}.log`. No service or physical-device coverage claimed.

## External039/045 focus and next lookup candidate

External build6996 completes successfully without warnings (1037 scheduled steps).
Initial strict146 focus11347 retains two kinds of failures rather than accepting
them: the ReadAsset ownership demo catches raw EOF where the measured XNA boundary
now correctly throws ContentLoadException, aborting before its behavior checks;
the comparison helper incorrectly counts printed failure lines as all checks in
the two repaired in-process demos. DemoContext::Check prints only failures, while
the JSON `checks` field still records every assertion. Both repaired demos already
PASS with their original5/14 check counts on both renderers.

The ownership demo catch is changed to exact ContentLoadException, not broadened.
Reader name, zero recorded resources, one stream open/destruction and every later
assertion remain unchanged;011 stays open and marked. The helper now requires
exactly one fewer printed failure for each repaired in-process witness AND the
unchanged total JSON check count. All unrelated printed assertions/failure multisets
remain exact. Original failed reports are retained.

After removing only039/045 markers, the11-step incremental external build96000 is
warning-free. Strict146 focus61606 passes:62PASS/11 unchangedFAIL on each renderer,
`errors: []`, with all original counts preserved. Both cache/root screenshots are
inspected: expected text and checkerboard, identical SHA256 and zero differing
pixels across SOFTWARE/OPENGLES3.319 external artifacts are frozen in
`repair-content-runtime-binaries.sha256`. Initial sandbox hashing failed EPERM
before writing the manifest; the authorized read-only repeat completed successfully.
Both95 math, metadata/tool gates and six exact zero-difference visuals pass; full
719 renderer sweeps and final CTest remain pending.039/045 are NOT closed yet.

The separate043 local registry candidate associates a returned reader's target
name with its actual factory registration, publishes only while that registration
is still current, and invalidates associations on withdrawal/replacement/clear.
Factories remain lazy and invoked outside the lock; each call still copies the
callable and returns a fresh reader. Actual registry TU plus the eleven focused
tests now passes11/11, compared with4PASS/sevenFAIL before. The same tests have
been added to CNA-native coverage. Sanitizer and canonical build/full regression
are still pending; no043 closure or external improvement is claimed.

### Retained full-sweep C contract regression

The039/045 SOFTWARE719 sweep completes with619PASS/49FAIL/48SKIP/three manual, but
the strict comparison fails: `c_api.content.content_manager` now fails its live-root
mutation check instead of reaching its historical103 exception-identity witness.
The failing conditional skips11 later child checks (54 ->43); the parent still
reports failure, so status totals alone would hide this regression.103 is NOT
resolved or validated by this run. The comparison JSON and full stdout are retained.

The C demo follows an explicit public contract: CNA `CNA/C/content.h` and
`docs/c-api/CONTENT.md` promise root changes preserving the existing cache, introduced
by ef6a72525 (CBIND-028). CNA's C ContentSmoke test also relies on it. The route is
currently a direct native-setter forwarder, so039's correct XNA/C++ guard exposes
this conflict. No C API oracle has been weakened or changed. An owner contract
question is pending: unify the C API with measured XNA/C++ and correct its docs/tests,
or preserve its documented behavior as an intentional C-only extension. Neither
answer is presumed. GLES and independent043 native validation continue;039/045
remain pending and this is not an accepted full checkpoint.

Both full sweeps now finish: GLES658PASS/51FAIL/sevenSKIP/three manual retains the
same C contract conflict. All319 hashes match after both sweeps and the independent
before-fix probes below. The failed external freeze is released; neither039 nor045
is closed, and no final CTest pass is claimed.

043 canonical build succeeds with51 existing warnings (no new diagnostic against
the preceding52-warning set). Whole module1929 gives1925PASS/four unchangedSKIP:
all1918 previous outcomes and11 new lookup passes match, differences[]. Four native
hashes match. Actual-TU ASan/UBSan/leak11/11 passes; full552 pipeline is running.
No external043 result or closure yet.

###103 isolated eight-route exception regression

`ContentExceptionIdentityProbe.c` exercises only public C calls, creates a fresh
manager, and neither loads a successful asset nor changes roots. A four-byte
`XNB\n` file consistently reaches ContentLoadException in Texture2D, TextureCube,
SoundEffect, Effect, Model, ObjectDictionary, SpriteFont and ForeignObject routes.
Every route checks IO result, null outputs, retained structured IO/message,
canonical exception type and empty argument parameter. Setup/inventory controls
bring the total to44. Both renderers give36PASS/eightFAIL, exclusively the missing
canonical type: `repair-content-exception-all-before-{SOFTWARE,OPENGLES3}.log`.
Strict C17/Wall/Wextra/Wpedantic/Werror compilation passes. A self-contained
CNA-native version also gives44 checks/eight failures; reports
`repair-content-exception-native-before{,-build}.log`.

The initial six-route missing-file probe is retained separately: eight failures
of34 include SoundEffect returning NotSupported, unlike the other five native
ContentLoadException routes. Its loose-audio path throws before the common XNB
boundary. This is not lack of audio hardware and not evidence that103 is fixed;
it needs separate native-layer investigation. The minimized malformed envelope
isolates the metadata defect without relaxing the expected canonical type.

The local candidate rethrows ContentLoadException from eight C catch arms to the
existing shared exception barrier, which already maps that type to IO and records
canonical metadata. No new translator or root-setter policy is introduced.
CNA-native CMake regression and fixture are added, but the two existing build
configurations do not enable its CTest target. Manual native-source runs must not
be reported as CTest registration execution. External C API target rebuild is
running; no after-fix result, repair commit or103 closure is claimed yet.

103 repair is now committed as CNA68a3c7b1f. The external C API target build
completes successfully with no warnings. Native and permanent evidence probes
each pass44/44 on both SOFTWARE and OPENGLES3. Exact comparison preserves all44
assertion labels and removes precisely eight canonical-type failures per renderer.
Existing ErrorSmoke/CoreExtSmoke/GeneratedRoutesSmoke/ContentReaderSmoke pass both
renderers; ContentModelEffectSmoke successfully loads a real model and exercises
its Effect routes on OPENGLES3. All six probe/smoke sources compile as strict
C17/Wall/Wextra/Wpedantic/Werror, as does the separate model/effect source.

The first helper returns1 solely because it looks for native `checks=44 failures=0`
in the demo harness, whose actual summary is `== PASSED (44 checks)`. Every binary
exits0. Its failed audit and raw logs are retained. Corrected read-only audit
recognizes the two exact output formats, verifies all44 original labels and eight
improvements, and returns `errors: []`; it does not rerun or rewrite evidence.
Reports `repair-content-exception-{after-*,corrected-audit.json,model-effect-*}`.
This is focused validation, not closure or a CTest execution claim. The original
marker and root-contract conflict remain. A complete external rebuild against
the stable043 candidate and committed103 repair has started for subsequent
focused demo validation; native043 pipeline continues independently.

### Prepared087 boundary controls (not executed yet)

The reference dispatch probe now appends14 checks to the original39: four
existing-instance overloads given a null reference must create and record one
new disposable; three incompatible requested-type reads must throw exact
ContentLoadException without recording the otherwise-disposable result.
The native source appends two grouped cases, preserving all original17 tests.
These additions are PREPARED, not yet runtime evidence. Helper
`build-probe/repair-content-dispatch-boundaries.mjs` is syntax-checked and will
require all39 old genuine labels and17 old native outcomes unchanged.

Source constraints for the next repair: shipped-XNA ContentReader.InvokeReader
casts the requested type before checking existing-instance identity/recording.
Explicit ReadObject(reader) dispatches according to the reader's target value/
reference semantics, not the requested T. Sharp System.Type explicitly documents
IsValueType as a permanent reflection stub returning false, so branching on it
would introduce another defect. C++ by-value representation is not an adequate
substitute either (String, Curve, Texture2D and Model are XNA reference types).
No metadata/ownership design or087 production patch has been selected yet.

These additions now execute against genuine XNA:53/53PASS with all39 original
labels unchanged. The expanded native probe givesfivePASS/fourteenFAIL of19,
all17 previous outcomes unchanged; both grouped boundary cases fail. Reports
`repair-content-dispatch-boundaries-*`, exact audit `errors: []`. The first
native Werror build refuses an unbraced `if` around a GoogleTest macro; adding
explicit braces fixes only that test syntax. Its failed build/driver are retained,
and the successful strict build/result use `native-before-braced`. No assertion
is removed, widened or disabled. No087 production change yet.

043 full552 pipeline completes in969.769s with551PASS/one unchanged XACT skip,
exact prior outcome audit `errors: []` and all four native hashes matching.
Broader1412/runtime241/XML runs continue on the same frozen build; external
972-step rebuild continues separately. Prepared146 focused comparisons require
only043's two lookup failures to disappear from the preceding039/045 focused
reports, with all check counts and unrelated failures unchanged.

043 is now committed as CNA252e9bca1 after the remaining native gates:
consumers1412=1408PASS/four unchangedSKIP183.079s; runtime241=238PASS/three unchanged
SKIP10.238s; default XML4PASS/two unchanged fixtureSKIP; original XML6/6PASS12ms.
All exact previous-outcome comparisons and four final native hashes match,
`errors: []`. Native freeze is released, but compiled content sources remain
unchanged while the complete external rebuild continues. Status remains
FIXED_PENDING_REGRESSION; no043 marker or final disposition change yet.

043 external whole build54284 now succeeds (972 scheduled steps), warning-free.
Strict146 focus99460 gives63PASS/ten unchangedFAIL per renderer; all original
check counts and unrelated failure multisets match. ExistingObjects passes all13
controls both renderers, removing precisely two lookup failures. Only then is
its043 marker removed. Nine-step incremental build14100 is warning-free and
repeated marker-aware146 focus84481 has identical expected results, errors[].
All319 external artifacts are frozen in `repair-reader-lookup-binaries.sha256`;
full math/metadata/tools/visuals/both719 regression8777 is running. The separate
C root-contract failure is not waived by the comparator. No full checkpoint or
043 closure is claimed yet.103 generator consistency also passes:3197 routes in57
files current, four generator regressions pass2.604s, no generated edits.
