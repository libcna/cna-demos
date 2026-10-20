# ISSUE-148: XML character/byte boundaries and processing instructions

## Reference and completed parser repair

The permanent `XmlReaderEnvelopeOracle.cs` passes **61/61** on genuine .NET
Framework 4/XNA's environment and .NET 8.0.31. It distinguishes ordinary processing
instructions from the reserved XML declaration, preserves literal comments/CDATA,
rejects a U+FEFF at the start of character input, accepts an encoded byte-stream
preamble, and records error timing/state. `IntermediateEnvelopeOracle.cs` adds
the actual XNA serializer's behavior, including `InvalidContentException` for
character BOM and rejection of self-closing root sections.

Owner-approved Sharp Runtime commit **16993ac6** repairs PI placement at the
responsible layer, the namespace-isolated vendored tinyxml2 parser. It does not
strip or rewrite XML, merge nodes, decode PI data as entities, or change the
parser ABI. Invalid targets and non-leading/reserved declarations remain refused.
Provenance and update instructions are in Sharp Runtime's `docs/VendoredTinyXml2.md`.

Before the parser repair, the four new vendor tests produce three failures and
one pass; all pre-existing 564 XML tests pass. After repair, **569 XML** and
**350 XML.Linq** tests pass. The actual parser compiled with ASan/UBSan and leak
detection passes all four new tests without diagnostics (GoogleTest itself is
not instrumented). The full library build is warning-free and all **18,151 tests
across 41 executables pass, zero skipped**. The unchanged Yacht SOAP service ran
on a private loopback fixture; this is not an online service validation claim.

The first full gate failed in an unrelated Process reader test because all
workers could be stopped before taking any snapshot. A fixed 100-iteration
diagnostic reproduced `reads == 0` once. Separate test-only commit **d326d37b**
adds a four-reader first-snapshot latch, strengthening the read-count requirement
and preserving every output assertion. No Process runtime change or sleep/retry
was added. The corrected fixed 100-iteration run and all 233 Diagnostics tests
pass. Initial failed builds/tests and diagnostic runs remain retained.

Reports: `build/linux-full/reports/repair-xml-pi-*`, including
`failed-components.log`, `passed-components.log`, `focused-after.json`, and the
instrumented parser results. Existing Sharp `build/` / `build-probe/` were reused,
with shared ccache and at most two compilation jobs.

## Character-input preamble follow-up (not yet closed)

Five new native character-input controls reproduce two failures on the repaired
PI baseline: `CreateFromString`/`LoadXml` silently consume U+FEFF, while the
heuristic `Create` route misclassifies BOM-prefixed XML containing `/` as a file.
The encoded-file, embedded-character and close-before-read controls already pass.
See `repair-xml-bom-before{,-build}.log`.

Sharp Runtime commit **97713eb2** distinguishes character input from encoded file bytes
inside Sharp Runtime, not in CNA's serializer. A character-input preamble must
raise on the reader's first `Read`, setting `ReadState::Error`, so CNA wraps the
exception at the actual deserialization boundary. DOM `LoadXml` rejects directly.
Embedded U+FEFF and file preambles must stay intact. Other pre-existing eager
tinyxml2 parse-error timing is not silently advertised as streaming .NET parity.

After the dependency fix, all **574 XML / 352 XML.Linq** cases pass. Full Sharp
gate: **18,158/18,158 across 41 executables, zero failures/skips**. The actual
XmlReader.cpp/XmlDocument.cpp and five native controls also pass ASan/UBSan/leak
checks, with normal linked dependencies; no full-dependency instrumentation
claim. The incremental build is warning-free and module boundaries pass.

Both native corpus exemptions for PI/BOM have now been removed in CNA commit
**dbfe6b4c1**. Its native build and all nine original corpus tests pass. The
original golden data and exact diagnostic comparisons remain unchanged.

The complete external build and subsequent marker-only build pass. The actual
serializer demo passes all **36 unchanged labeled assertions on both SOFTWARE
and OPENGLES3**, before and after removing its verified148 metadata marker.
The separate owner-approved C-root correction also passes all67 controls on both
renderers (commit `dab9d4f`, marker follow-up `aab4bf3`); C++ cache retains5/5.
Focused comparisons have errors[] and preserve every previous serializer label.

The full native suite now passes **10,690 / 101 unchanged skips** (10,791 cases)
in 1,647.295 seconds. Exact case-by-case comparison against the published frustum
baseline and all three frozen native artifact hashes pass. No failed native case
was suppressed; the two removed corpus exemptions strengthen the original oracle.
The full SOFTWARE719 sweep also passes its original-baseline and predecessor
audits: 627 PASS / 41 unchanged FAIL / 48 SKIP / three manual, exactly the two
expected XML/C-root improvements since ISSUE-016. Reports include
`repair-xml-substrate-native-gates.json` and `full-SOFTWARE-{comparison,local-audit}.json`.

OPENGLES3 and final CTest now also pass their exact audits: OPENGLES3
666PASS/43 existingFAIL/7SKIP/three manual; CTest684PASS/48 unchangedSKIP/three
disabled in887.44s,319 pre/post artifact hashes verified. Only the expected
XML/C-root transitions differ from the previous checkpoint. ISSUE-148 is now
resolved with full evidence in `XML_CONTENT_FINAL_CHECKPOINT.md`; all original
historical text remains. These completed gates do not cover the subsequent
uncommitted ISSUE-109 encoder candidate.
