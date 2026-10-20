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
Prepared runtime patch is unapplied while the external storage build runs.

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
`build-probe/cna-repair-xml-nul-before{,-build}.log`. Production remains unchanged.

Local .NET11 reference corroborates the measured default: System.Private.Xml
`System/Xml/Core/XmlEncodedRawTextWriter.cs:1631–1655` calls
`XmlConvert.CreateInvalidCharException` when checking is enabled;
`System/Xml/XmlConvert.cs:1624–1627` defaults that helper to ArgumentException.
