# Content repair evidence

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
