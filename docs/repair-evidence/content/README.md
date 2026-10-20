# Content repair evidence

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
independent compilers. Production remains unchanged; no090 closure is claimed.
