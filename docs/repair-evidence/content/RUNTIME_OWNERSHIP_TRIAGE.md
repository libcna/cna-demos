# Next content-runtime batch: source audit, not repair closure

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
