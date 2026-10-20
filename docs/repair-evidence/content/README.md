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
