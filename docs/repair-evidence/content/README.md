# Content repair evidence

## ISSUE-012: stream routing baseline (2026-10-20)

`ResourceContentColdOracle.cs` now passes14/14 against genuine XNA4 in the private
SDK environment, strict C#4/x86 warnings-as-errors. No Game or graphics service is
created. It verifies the authored Curve, loop values, managed cache identity,
single resource lookup, fresh decode after Unload, exact missing-resource exception
and cache retention after that failure. Reports `repair-content-resource-cold-xna-*`,
exact audit errors[]. Native Curve's existing by-value carrier is not falsely
claimed to reproduce managed reference identity; its resource/cache/read behavior
is the relevant counterpart.

`ContentStreamRoutingTests.cpp` runs without a Game, routing opt-in or manual
built-in registration. Four strict native cases giveonePASS/threeFAIL against
the frozen native tree (uncommitted089 initialization does not change these paths).
Direct ReadAsset and its original exception propagation pass. Ordinary Load does
not call the override, and a cold ResourceContentManager cannot find CurveReader.
The cold case preserves additional cache and explicit-withdrawal controls for the
repair; they cannot execute until the first load succeeds and are not claimed
validated now. No012 production patch. Reports
`repair-content-stream-routing-native-before-*`, actual test exit1, exact audit
errors[], warning-free strict build. Genuine virtual Load/cache evidence remains
in ContentOwnershipOracle; dedicated cold-resource reference evidence is next.

## ISSUE-089: initialization cycle and failure baseline (2026-10-20)

Repair committed CNA `5d32ca58b` (full external regression pending). Canonical
native559=558PASS/one unchangedSDK-fixtureSKIP818.582s; content1965=1961PASS/four
unchangedSKIP; consumers1412=1408PASS/fourSKIP129.299s; runtime241=238PASS/threeSKIP
9.367s; default XML4PASS/two fixtureSKIP and original XML6/6PASS11ms. Exact audits
and all four post-gate native hashes match. Whole external105-step build passes
warning-free; nine source hashes match.

Initial146 external focus gives **65PASS/eight unchangedFAIL per renderer**.
The tool's historical18-control count
was stale: the immediately preceding frozen logs already contain20 labels
(19 passing, one089 failure). The first helper rejected actual20 solely because
of that18 assumption; its failed audit remains. A read-only re-audit of those same
logs proves all20 ordered labels unchanged and only089 improving on both renderers,
with no test rerun or demo assertion edit. Reports `*-focused-initial-label-audit.json`.
Marker089 and its false never-initialized restriction were removed only afterward;
warning-free nine-step marker build completes, marker focus is running. Full719,
original-baseline residual audit, six visuals and final CTest remain pending.

Candidate validation11:00 UTC (not a closure): four actual implementation TUs
(`IntermediateSerializer`, `ContentTypeSerializer`, `IntermediateWriter`,
`IntermediateReader`) pass the21 basic,11 cycle and6 failure controls, plus the
original two owner-isolation cases. Expanded owner coverage adds registration
during a writer callback and simultaneous first use of a new worker; all four
cases pass. Normal and ASan/UBSan/leak variants have exact audits errors[], no
diagnostics, warning-free builds (production policy for implementation TUs,
Wall/Wextra/Werror for test TUs). Linked libraries are not all instrumented.
Reports `repair-intermediate-initialization-candidate{,-sanitizer}{,-extended}-*`.

Seven equivalent CNA-native GTests are installed: basic lifetime/cache, cycles,
fresh-process failed initialization, and four isolation/late/concurrent cases.
The failure case re-executes a child so a deliberately failed registry does not
contaminate other tests. All7 pass in normal and sanitized actual-TU fixture builds;
a separate fresh-process first-use run also passes1/1 in both variants, covering
cold built-in discovery without a prior warmup. Reports
`repair-intermediate-initialization-native-fixtures{,-sanitizer}-*`.
Canonical whole native build is running; no native full regression or issue closure
is claimed. The candidate retains a registry-owned callback owner, scopes document
state per thread/operation, and publishes workers before callbacks without holding
the registry lock while running user code.

Canonical follow-up11:05 UTC:79-step whole build passes. Its two material-test
nodiscard warnings exactly match the prior048 build of those TUs; the initial
audit wrongly compared against087's51-warning build, which did not compile those
material tests. Failed audit and two gate setup failures (missing freeze manifest,
no test launched) remain. Corrected provenance/accounting audit passes, nine source
hashes match, four native artifacts are frozen. Canonical7/pipeline559 and content
module1965 gates are running; no full native regression or089 closure yet.

The explicit unique_ptr-registration boundary below remains deliberate and visible:
the candidate retains and propagates a failed initialization instead of retrying
that same object. It does not claim XNA's fresh reflected singleton reconstruction,
and introduces no factory/clone API. Raw published lookup remains usable on the
captured failed owner, as the six measured failure controls verify.

`IntermediateInitializationEdgesProbe.cpp` adds native counterparts of the already
measured genuine15 publication/cycle/failure controls. Two fresh processes avoid
cross-contamination from a deliberately throwing initializer. Strict C++23 build
with Wall/Wextra/Werror is warning-free. Cycles give11 controls:onePASS/tenFAIL;
failure gives6:onePASS/fiveFAIL. The passing controls show that unused payload
writers are not invoked. Missing eager initialization, published self/mutual lookup,
retained owner/cache and original exception propagation are explicit failures.
Reports `repair-intermediate-initialization-edges-native-before-*`; exact audit
errors[], actual probe exits1. No089 runtime patch or successful fix is claimed.

These17 native controls deliberately do not pretend explicit CNAEXT unique_ptr
registration can reconstruct fresh instances after failure: unlike XNA discovery,
it retains no constructor factory. Genuine fresh-owner/fresh-instance recovery is
still recorded by `IntermediateInitializationEdgesOracle.cs`, not discarded.
Any implementation must explicitly resolve/document that registration boundary;
retrying the same failed instance would not reproduce the measured XNA behavior.

## Completed checkpoint (2026-10-20)

146/147/149 are FIXED_VALIDATED; see [the full checkpoint](CHECKPOINT.md).

CNA repairs146/149 are committed as7c4431fdf and9dd0826bf; Sharp147 isaf1b0bea
(current Sharp99c2f79c). Full native content504:503PASS/one unchanged XACT fixture
SKIP,999.450s; exact identity/status comparison against the pre-fix run shows only
the six new expected failures becoming PASS. Broader consumers1408PASS/four same
SKIP of1412, runtime238PASS/three same SKIP of241. XML consumers initially4PASS/two
missing-fixture SKIP; using the existing original ShipGame fixture alias gives6/6.
The initial skip log is retained. Native hashes verified before the next batch.

Actual OpaqueDataDictionary.cpp, IntermediateReader.cpp and IntermediateSerializer.cpp
instrumented with ASan/UBSan/leak detection:9/9 selected native regression cases
PASS. This is not a claim of whole-library instrumentation. Initial shell-quoting
and incomplete static-library-closure harness failures remain in the reports;
the corrected build uses CnaTests' exact static content-pipeline closure.

External strict28-demo focus on each renderer:56 expected results,23PASS/five
unchanged FAIL per renderer. Exact parent statuses/counts AND child assertion counts/
failure multisets are checked. Only146/147 become PASS; exactly two149 assertions
stop failing while all four148 failures remain. Demo markers71e6992 changed only
after this validation. Both external builds are warning-free; native initial build
retains two pre-existing material-test nodiscard warnings, final small rebuild is
warning-free. Reports `repair-content-*` preserve the failures and successful runs.

Full SOFTWARE719:614PASS/54FAIL/48SKIP/three manual, strict57 improvements versus
campaign HEAD (59 versus Complete1128), differences[]. Metadata/tools and all six
zero-pixel-difference visuals pass; the GLES font image was also inspected. Full
OPENGLES3 likewise passes strict57 improvements/differences[]:653PASS/56FAIL/seven
SKIP/three manual. Final isolated serial CTest accepts all732 enabled tests, zero
unexpected failures,48same skips/three disabled,747.99s.319 frozen executable/
DSO/tool hashes are in `repair-content-binaries.sha256`; external binaries must not
rebuilt only after the final audit. All319 hashes passed before/after CTest;
the external freeze is now released and146/147/149 are closed.

The sections below retain the chronological reproduction and intermediate results.

## ISSUE-146: exact null-value and empty-key contracts

`OpaqueDictionaryOracle.cs` passes14/14 against genuine XNA4's content-pipeline
assembly (Version4.0.0.0, PublicKeyToken842cf8be1de50553). Original SDK framework,
graphics and pipeline DLLs are symlinked only into the isolated ignored directory
`build-probe/repair-content-reference`, beside this probe's executable. They are
not placed in the shared probe directory, which would change other probes' binding.
Reports: `repair-opaque-dictionary-staged-xna{,-build}.log`.

Add and setter reject null values with exact ArgumentNullException("value"),
including duplicate/new-key cases. Empty-key Add/set/Remove reject with exact
ArgumentNullException("key"); key validation precedes value validation. Rejected
operations preserve entries and byte-exact XML. Successful insertion via setter
and replacement preserve insertion order. The generic NamedValueDictionary<object>
confirms this is partly a shared base contract, not only an opaque-data special case.

Three new native OpaqueDictionaryCompatibilityTest tests against unchanged CNA
production give1PASS/two expectedFAIL (3ms): exact exception/value validation and
key precedence fail, successful insertion/replacement passes. The native test-only
build succeeds. Reports `repair-opaque-native-before{,-build}.log`. These tests and
the genuine source are preparatory evidence, not a repaired/closed issue.

The initial genuine attempt failed to load its pipeline assembly before behavior
execution. It entered Wine's debugger; only the identified owned probe and debugger
were terminated, wrapper exit143. That failure remains in
`repair-opaque-dictionary-xna{,-build}.log`; it is not a behavior result. Isolated
staging and a no-inlining setup/error boundary corrected the probe setup without
changing product code or weakening any assertion.

`NamedDictionaryHooksOracle.cs` additionally passes7/7 genuine checks: the public
setter calls only SetItem for both new and existing keys, Add calls only AddItem,
and the protected base setter inserts missing keys while validating key before value.
The expanded native suite gives1PASS/four expectedFAIL (five tests,7ms) before any
production change. Reports `repair-named-dictionary-hooks-xna*`,
`repair-opaque-hooks-native-before*`. The original three-test evidence is retained.
After freezing the external storage binaries and completing the native content
baseline, the narrow runtime patch is applied; post-fix validation is pending.

## ISSUE-147: XML writer NUL exception identity

`XmlWriterNulOracle.cs` passes21/21 on both Framework4.0.30319.1/Win32NT and
.NET8.0.31/Unix: seven writer doors, two NUL placements at each door, seven ordinary
content controls. Default character checking reports exact ArgumentException,
not XmlException. Reports `repair-xml-nul-{xna,net8}{,-build}.log`.

The seven CheckCharacters=false results are diagnostics, not additional asserted
passes: text/attribute/element-string encode a character reference; CDATA, comments,
processing instructions and DOCTYPE subsets emit literal NUL. The current tinyxml2
substrate cannot safely carry such literal NUL. The planned narrow exception repair
does not claim implementation of this optional mode or silently truncate content.
Its existing unconditional rejection limitation must remain explicitly documented.

Sharp native test-only build passes warning-free, then49 XmlWriterValidationTests
give46PASS/three expectedFAIL (2ms): the existing all-door and useful-diagnostic tests
now assert the measured exception, and a new test requires byte-identical existing
content after rejection at all six direct mutation doors. WriteElementString keeps
its separate existing rejection control; no new atomicity claim is made for its
compound start/write/end operation. Sharp reports
`build-probe/cna-repair-xml-nul-before{,-build}.log`. The subsequent writer repair
passes49/49 tests; full dependency validation is still pending.

Local .NET11 reference corroborates the measured default: System.Private.Xml
`System/Xml/Core/XmlEncodedRawTextWriter.cs:1631–1655` calls
`XmlConvert.CreateInvalidCharException` when checking is enabled;
`System/Xml/XmlConvert.cs:1624–1627` defaults that helper to ArgumentException.

`XLinqNulOracle.cs` passes74/74 against both genuine Framework4 and .NET8.0.31:
fourteen content doors,
four NUL placements plus ordinary content, and four standalone attribute namespace
controls. The initial65/65 probe was expanded; its71/75 failure disproved an assumed
namespace rejection because standalone XAttribute omits its namespace URI in .NET.
The corrected74-check probe retains that measured behavior. Reports
`repair-xlinq-nul-{xna,expanded-xna,namespace-xna}*`. CNA's underlying Sharp port
emits a standalone attribute's namespace URI, so its existing URI guard stays;
the content-value category repair does not claim to fix that formatting difference.

Native LINQ evidence-backed19-test suite gives7PASS/twelve FAIL before repair.
After a shared character-data guard correction, twenty tests pass, including a new
namespace-output preservation control. The new fixture initially reversed the C++
XName constructor arguments (19/20); changing it to documented XName::Get order
corrected the fixture, not the oracle. All logs remain. Two full dependency gates
failed earlier: an unrelated watcher callback barrier, then stale LINQ categories.
The watcher barrier test is corrected without sleeps/retries; a separate complete
stress run still has two other rename-event failures (678/680), explicitly not fixed.
Full dependency validation remains required before147 can be closed.
The modern .NET build/run reports are `repair-xlinq-nul-net8{,-build}.log`.

## ISSUE-148/149: distinguish deserialization from reserialization

`IntermediateEnvelopeOracle.cs` freshly measures direct reads and subsequent writes
separately. Its final genuine XNA run passes16/16 checks. The earlier diagnostic run
is also retained. Reports `repair-intermediate-envelope-{direct,asserted}-xna*`.
Processing instructions are accepted; self-closing Resources/ExternalReferences
and an in-memory BOM are rejected; expanded paired sections and Null=false remain
valid. A null value-type member raises exact NullReferenceException.

**The historical null-root read expectation was wrong.** Direct XNA deserialization
of `<Asset Null="true" />` succeeds and returns null. Only reserializing that null
raises ArgumentNullException("value"). CNA's original oracle `Accept<T>` in
`tools/xna-pipeline-oracle/intermediate/IntermediateOracle.cs:506–529` combines both
operations in one try/catch, marking the entire round trip rejected. Its manifest
is evidence of round-trip rejection, not proof of a failed read. Keep that historical
manifest intact; add separate direct-read coverage and correct the demo's two read
checks, while retaining its independent null-write rejection. CNA's current read
refusal still needs a real runtime fix, not a change merely to satisfy the old demo.

The first fresh setup passed an empty relocation string; genuine XNA rejected that
path before any behavior case. The original setup failure and wrapper exit143 remain
in `repair-intermediate-envelope-xna*`. Only the verified owned probe1396356 and
debugger1396535 were stopped. The corrected probe uses null relocation (matching
the original corpus default) and catches setup errors explicitly. No production
code or golden corpus was changed by these measurements.

Two new native direct-read149 tests now both fail against unchanged CNA (15ms),
after a successful test-only build: null roots are refused instead of returned,
and null value members have the wrong exception category. The Null=false positive
control passes. Reports `repair-intermediate-null-native-before{,-build}.log`.

Full native content baseline:504 tests/117 suites in1306.990s,497PASS/one existing
XACT fixture SKIP/exactly six new expected failures (four146, two149). No other
failure. Reports `repair-content-native-before*`. Only after this run completed
were dictionary/null runtime repairs applied; full post-fix validation is pending.

Post-fix native focus now passes19/19 (735ms): dictionary tests, entire intermediate
corpus and hardening. The first18/19 result exposed a diagnostic-format distinction:
existing Sharp Serialize(null) uses modern .NET's `(Parameter 'value')`, while the
Framework4 corpus uses a separate `Parameter name: value` line. Both exact forms
are now pinned alongside exception type, parameter and timing; no runtime or golden
message was rewritten. The original failed log remains. Native full504 regression
and actual-production-TU sanitizers are running. The demo's two null-root read
assertions now require successful completion and a null result, preserving count
and independent write rejection; external rebuilding waits for storage CTest.

Sharp repair af1b0bea is committed; full dependency gate passes18146/18146 across
41 executables, zero failures/skips. Watcher TEST-only synchronization is separate
e0601cd2; the two stress-only rename findings above remain unresolved. An invocation
missing supported XNA_SAMPLES_ROOT produced five fixture skips; it is retained as
*-components-missing-fixtures.log, and the complete final gate uses the original
linked sample fixtures and isolated original Yacht service.

## ISSUE-148: reader positions, BOM and exception timing

`XmlReaderEnvelopeOracle.cs` passes61/61 on Framework4 and .NET8.0.31. Both accept
processing instructions before, inside and after the root, after a comment, and
with a non-reserved `xml-stylesheet` target. PI-looking comment/CDATA/escaped
attribute data remains ordinary data. Nested/trailing XML declarations, reserved
uppercase XML targets, unterminated instructions and a character-string BOM are
rejected; a UTF-8 BOM in a byte stream is accepted.

Five malformed-input controls also prove XmlReader creation succeeds in Initial
state, Read throws XmlException and enters Error, subsequent Read returns false,
and Close leaves a terminal Closed state. This matters because CNA's intermediate
serializer must wrap a reader failure at the correct boundary; simply throwing
early from CreateFromString would not match the genuine XNA contract.
Reports `repair-xml-envelope-timing-{xna,net8}*`; the initial31-check position/BOM
runs remain as `repair-xml-envelope-{xna,net8}*`. One modern compile command listed
a nonexistent XPath facade; its failed build log remains and the corrected build
uses actual installed reference assemblies. No XML reader/PI runtime repair yet.

The tinyxml2 node model already represents both XML declarations and ordinary PIs
as XMLDeclaration, and its placement check erroneously restricts both. A narrow
vendor repair versus preserving the repository's documented unchanged-vendor
policy was asked asynchronously; no answer or permission is inferred. No vendor
file has been modified, and unrelated campaign work continues.

## ISSUE-090: initialization callback deadlock

The narrow two-phase AddWriter repair now builds warning-free;10/10 native focus
passes, including four new bounded callback regressions. The original standalone
probe now passes9/9 against the actual native library closure. It verifies the
callback returns, nested/cache identities remain stable, initialization runs once,
and pre-existing duplicate/frozen checks occur before the callback. Full508 native
content regression is running; no external090 pass or issue closure is claimed.

`CompilerInitializationOracle.cs` passes8/8 on genuine XNA4. The real internal
compiler constructor discovers attributed writers; its build-host lookup creates
the initialization context. Inside Initialize, the PUBLIC GetTypeWriter(string)
completes, preserves cached writer identity and runs once per compiler-owned writer.
Reflection supplies only the unavailable build-host entry, not replacement behavior.

## ISSUE-088/089: intermediate registration and initialization references

2026-10-20 native089 baseline: permanent `IntermediateInitializationProbe.cpp`
strict C++23/Wall/Wextra/Werror build is warning-free;21 controls givefourPASS/
seventeenFAIL, exactly audited in `repair-intermediate-initialization-native-before-*`.
Both XML round trips and byte-identical repeated output pass, but initialization,
callback owner identity and retained owner/nested-cache controls fail. No089 runtime
patch is applied. Unlike the genuine process-wide owner, CNA currently constructs
a stack-local IntermediateSerializer for each operation and keeps serializers in a
process-wide registry. Initializing those serializers with that stack object would
introduce dangling retained context, not fix the contract. Per-operation namespace
aliases and relocation paths must also remain isolated when repairing owner lifetime.

Further089 source constraints (no patch selected): the two per-operation fields
are `referenceRelocationPath_` and `aliases_`; their CNAEXT accessors are public,
so a stable callback owner must not silently lose the active operation's state or
leak it between nested/concurrent operations. Reader/Writer currently return their
temporary `serializer_`, and namespace scans pass that same temporary to callbacks.
Raw registry `FindTypeSerializer` functions are noexcept; do not make them invoke
throwing user initialization. Initialization must permit published self/mutual
lookups without holding the nonrecursive registry mutex across a user callback.
Built-in registration is itself recursive and marks its flag before completing;
check lock ordering before adding any once/initialization gate. CNA's explicit
registration consumes a unique_ptr and has no cloning factory: genuine failed
singleton reconstruction cannot be imitated by inventing fresh user instances.
That boundary needs an explicit, evidence-backed translation, not an assumed retry.

Additional native guard `IntermediateOwnerIsolationTests.cpp` now builds warning-free
with C++23/Wall/Wextra/Werror. Both cases fail with exactly ten owner/initialization
assertions and no other assertion failures: nested serialization already preserves
the outer relocation path/namespace aliases; two simultaneous writes already keep
their paths separate and emit identical XML. The missing stable initialized owner
is exposed without sleeps or retries; a barrier guarantees simultaneous callbacks.
Reports `repair-intermediate-owner-isolation-native-before-*`, exact audit errors[].
These context-isolation checks protect existing documented CNAEXT behavior; they
are not a claim that genuine XNA was concurrency-tested. No089 runtime patch yet.

`IntermediateCollisionOracle.cs` passes11/11 on genuine XNA4. The real private
registration entry rejects a distinct target with the same XML shortcut using
exact InvalidOperationException. Its diagnostic identifies XmlTypeName and the
colliding shortcut. The original writer/mapping/output remains unchanged; the
rejected target can subsequently register with a distinct shortcut, and both
public Serialize/Deserialize round trips preserve exact content. Reflection only
supplies XNA's registration host entry. Reports `repair-intermediate-collision-asserted-xna*`;
the initial diagnostic showing reflective fallback after rejection is retained.

`IntermediateInitializationOracle.cs` passes21/21 on genuine XNA4. One custom
serializer instance receives Initialize exactly once across two writes and two
reads. Nested string lookup succeeds inside Initialize, before any ScanChildren,
Serialize or Deserialize. All callbacks see the same long-lived owner; its custom
and nested serializer caches remain usable after all four operations. Reports
`repair-intermediate-initialization-asserted-xna*`; the shape and initial diagnostic
probes remain. This disproves a per-operation new-instance assumption: a fix must
not initialize a shared serializer with a temporary context it could retain.

Current external strict content focus still reproduces both088/089 assertions
(20 child controls, two failures) on both renderers. Source confirms unchecked
name-map insertion after ownership/type mutation and no production InvokeInitialize
caller.

`IntermediateCollisionProbe.cpp` minimizes088 against the real native static
library closure:16 controls, six PASS/ten FAIL. It observes unwanted target,
carrier, full-name and additional-alias publication, failed clean re-registration,
and a failed round trip, while preserving the original serializer and CNA's existing
first-registration-wins policy for repeated targets. Its first harness compile
used incorrect XML factory/ownership signatures; corrected strict C++23/Werror
build is warning-free, and both logs remain. Reports
`repair-intermediate-collision-native{,-corrected}-before*`.

Five new CNA-native registry tests likewise reproduce one PASS/four FAIL against
the unchanged frozen library in an isolated executable. An initial control wrongly
expected an existing shortcut to be unknown; source shows IsKnownTypeName includes
aliases. The corrected known-before/known-after control preserves that contract;
the corrected baseline remains one PASS/four FAIL (2ms). Reports
`repair-intermediate-registry-native{,-corrected}-before*`. A narrow preflight check
has now been applied to source; its actual-TU sanitizer build is running separately.
It is not in the frozen compiler or external full-regression binaries yet.

`IntermediateInitializationEdgesOracle.cs` passes15/15 genuine checks: self and
mutual nested lookups resolve already-published serializer identities. Discovered
custom serializers initialize eagerly in singleton construction. A thrown initializer
aborts singleton publication; the captured failed owner still retains its serializer
without reinitializing, while the next successful construction owns fresh instances.
The initial lazy-initialization setup hypothesis failed before its checks and is
retained; it is not a passing reference result. The corrected explicit eager-failure
probe is `repair-intermediate-initialization-edges-eager-xna*`.

Targeted genuine reflection/IL inspection confirms the owner is process-wide, not
ThreadStatic, and GetTypeSerializer registers a newly created serializer before
calling Initialize. A native monodis attempt crashed before useful output; that log
is retained. No089 production repair is applied yet; callback timing and lifetime
must not be guessed from the initially incomplete hypothesis.

### ISSUE-088: shortcut priority is not a duplicate-name error

`IntermediateNamePriorityOracle.cs` passes12/12 genuine XNA checks. An explicit
XML shortcut may equal another target's full type name, in either registration
order. All four emitted aliases and exact public round trips pass, with stable
typed serializer identities. Reports `repair-intermediate-name-priority-asserted-xna*`;
the initial diagnostic is also retained.

This disproved two over-strict newly written native rejection expectations. The
first all-name preflight candidate passed its five tests and actual-TU ASan/UBSan/
leak checks, but was rejected on compatibility evidence, not committed as a repair.
Its source, tests and sanitizer log remain as `repair-intermediate-registry-overstrict-*`.
The corrected tests require both allowed shadowing orders and exact round trips,
while preserving duplicate-shortcut/additional-alias rejection and first-target
registration behavior. Against the original library they still give one PASS/
four FAIL (2ms): shared-name lookup also reinterprets a canonical type identity
as another XML alias. Reports `repair-intermediate-registry-priority-before*`.

The current local candidate therefore keeps full type names and aliases in
separate private maps: XML spelling resolves aliases first, canonical identity
resolves full names first with the existing array-alias fallback. All collision
checks precede registry mutation. No public API or object layout changes. Native,
external and actual-TU sanitizer builds are running; this is not an088 closure.

## Next registry/compiler/environment checkpoint — validation in progress

CNA088 is committed as `80b8e45b2`,090 as `7864dd5b1`, and111 as `6823af99a`.
Corrected registry actual-TU ASan/UBSan/leak5/5 and the original16-check corruption
probe16/16 pass. Native34/34 includes the complete intermediate corpus, compiler
and registry controls. An initial invocation from the demo cwd produced ten missing-
fixture failures; it is retained, and the corrected CNA-root invocation passes34/34
in705ms. Always run native corpus tests from the CNA repository root.

111's public CNA extension contract requires explicit build-environment targets.
Four new native tests cover all three target bytes/two profiles, explicit override
of nondefault registration with LZX retained, old no-environment defaults, and cache
identity. Before repair:one PASS/three FAIL,18ms. The writer component version
changes from1 to2 because ContentBuildManifest includes that version in fingerprints;
otherwise old incorrect platform/profile bytes could remain cached. The demo's
existing exact identity assertion changes only its required version to2, not its
strength or count. No Windows Phone/Xbox runtime or genuine-XNA execution is claimed
by these CNA extension tests.

Combined native focus gives52PASS/one existing opt-in >2GiB-file SKIP of53,
1007ms, including existing cache-fingerprint tests. Both111 builds are warning-free;
the preceding registry native rebuild retains the same two pre-existing material
nodiscard warnings. First strict56 external runs remove only088/090 failures;
after111 repair, final strict56 runs give24PASS/four unchanged FAIL per renderer,
no unexpected status/count/assertion differences. XML-to-XNB is33/33 on both.
Only after these passes do markers change:088->089 for the remaining initialization
defect;090/111 removed from the now-passing XML-to-XNB demo. All assertions remain.

Full517 native content regression is running. External319-binary snapshot is frozen
in `repair-registry-binaries.sha256` for full sweeps and final CTest. Expected full
delta:58 demo-status improvements versus campaign HEAD (60 versus Complete1128),
plus the existing two149 improvements and one088 assertion improvement in a still-
failing089 demo. No088/090/111 closure until the full checkpoint passes.

Initial setup attempts are retained: manual AddTypeWriter duplicated a writer that
the genuine constructor had already discovered; public GetTypeWriter outside an
initialization context failed its dependency-stack precondition. Neither is a CNA
defect or a passing reference result. Reports `repair-compiler-{shape,initialization}-*`.

`CompilerInitializationProbe.cpp` strictly builds with C++23/Wall/Wextra/Werror
against the current native CNA's actual static ContentPipelineBuild closure. It
passes the built-in lookup control, prints ENTER Initialize and deadlocks in the
nested lookup. A single five-second process deadline terminates this isolated
reproduction (SIGTERM/ETIMEDOUT, nonzero); it is not a retry or a pass. Nine required
post-fix checks cover lookup/callback/cache, duplicate/frozen registration and
independent compilers. This records the pre-repair baseline; the committed090
repair and subsequent validation are described above. No090 closure is claimed yet.

## ISSUE-148: section traversal isolated from XML parser limitations

`IntermediateSectionsOracle.cs` freshly passes13/13 against genuine XNA4.0 on
Framework4. The five ordinary/paired-empty controls retain integer37. Seven
self-closing variants fail with exact InvalidContentException and the measured
`None` or `Element` XmlNodeType diagnostic; reversed paired-section order is also
rejected with `Element`. This verifies traversal behavior, not simply a blanket
ban on empty XML nodes. Reports: `repair-intermediate-sections-xna*`.

The matching13 native parameterized cases, strictly compiled against the frozen
registry checkpoint library, initially give sixPASS/sevenFAIL in2ms. Removing only
the two premature empty-section returns in IntermediateReader preserves the
ordinary traversal and its error timing. The actual modified production TU and
the13 tests pass ASan/UBSan/leak checking in8ms, using the existing static library
closure; the frozen checkpoint binaries are not rebuilt. Three obsolete corpus
exemptions are removed, with all historical golden inputs/messages retained.

The first exploratory sanitizer compile added Wall/Wextra and found an unchanged
parentheses warning in DotNetTypeString at line57. That failed log is retained;
it is not a passing stronger-warning gate. The successful sanitizer build uses
the established actual-TU/project flags with Werror, as the prior sanitizer batch.
Reports: `repair-intermediate-sections-native-before*`,
`repair-intermediate-sections-sanitizer{,-policy}.log`.

This is a **partial148 repair, not closure**. Full native/corpus/external validation
must follow release of the088/090/111 binary freezes. The processing-instruction
vendor-policy question remains unanswered; string-BOM handling must also preserve
the measured deferred Read exception timing. Neither parser issue is repaired or
hidden, and the demo148 knownIssue remains unchanged.

The subsequent **whole-corpus sanitizer gate failed**, retained as
`repair-intermediate-sections-sanitizer-corpus.log`:21PASS/one FAIL of22, with two
missing-reference messages displaced by the end-document diagnostic. LeakSanitizer
also reports792 bytes in11 allocations, all the corpus's deliberately cyclic Node
fixtures. The13-case focused pass above is not a whole-corpus pass or sufficient
validation of the initial section repair.

Expanded genuine `IntermediateSectionsOracle.cs` now passes18/18. Shared fixups
run after consuming their section end and before reading externals; external fixups
run before the final envelope check. A malformed section end still wins over a
missing-reference error. The initial contrary expectation for that last ordering
is retained17/18, then corrected from measured XNA behavior. CNA's private fixup
method is split by section; tests preserve exact missing-reference diagnostics.
Test-owned strong Node cycles now have RAII teardown **after** serialization and
all graph assertions, including exception/ASSERT exits. No graph behavior assertion
or golden XML is removed. The corrected actual-TU/corpus sanitizer run is pending.

The new native fixture classes initially omitted Object::GetTypeName overrides,
making them abstract. Those setup failures are retained; corrected frozen-library
baseline18 tests gives sevenPASS/elevenFAIL in2ms. Reports
`repair-intermediate-section-order-{verified-xna,corrected-native-before}*`.

## ISSUE-046: namespace-aware external target identity

`IntermediateExternalAliasOracle.cs` passes12/12 against genuine XNA. Root and
entry-local aliases, local namespace shadowing, aliases inside generic arguments,
assembly qualification and types without serializers are verified. A target with
no public constructor remains a valid external identity; it is never instantiated.
Surrounding TargetType whitespace is refused, not trimmed. The initial contrary
whitespace expectation is preserved11/12; the corrected expected contract follows
the observed genuine exception, not current CNA behavior.

Matching corrected native baseline gives fourPASS/eightFAIL in2ms, with controls
ensuring no target serializer or global XML alias is introduced. The initial native
abstract-fixture setup failures are retained separately. No046 runtime change yet.
Reports `repair-intermediate-external-alias-{verified-xna,corrected-native-before}*`.

Follow-up: CNAde1b6e259 corrects148 per-section fixup ordering and cyclic fixture
teardown. The actual Reader/Serializer TUs plus the entire intermediate corpus and
18 section/order cases now pass **27/27 ASan/UBSan/leak checks**,1567ms, exit0;
the previous failing22-case log remains. Native baseline freeze was released only
after the517/content-consumer/XML gates and final four-hash audit completed and
this isolated sanitizer finished using its archives.

The046 candidate now reuses the existing namespace/generic spelling resolver with
a private external-identity mode, validates the complete identity without requiring
a target serializer, and canonicalizes both table identities and pending fixups.
Surrounding TargetType whitespace remains refused. No global alias or public API
is added. Native rebuild and39-case sanitizer compilation are running; no046 pass
or issue closure is claimed yet.

Final046 repair is CNAf421948fe. Additional genuine assembly-name boundary cases
found that qualified names accept leading type/trailing assembly whitespace, while
unqualified and generic trailing whitespace is refused. The first extra reference
expectation is retained14/15; corrected genuine15/15 passes. The initial candidate
fails exactly those two valid qualified inputs (15PASS/twoFAIL of17); the final
resolver distinguishes the CLR-qualified path instead of rejecting all outer spaces.

Two native writer-roundtrip controls are added to the15 reference-matched reads.
Final native focus58/58 passes706ms; actual Reader/Serializer TUs plus the entire
intermediate corpus,18 section/order cases and17 external cases pass
**44/44 ASan/UBSan/leak checks**,565ms, exit0. Earlier41/41 candidate run673ms is
retained. The initial72-step native build has only the two unchanged material
nodiscard warnings; the final12-step incremental build is warning-free. Full552
content tests/123 suites are running against four frozen artifacts. No046 closure
or demo marker removal until focused external and full regression gates finish.
