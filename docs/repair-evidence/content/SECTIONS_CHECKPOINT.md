# Section ordering, external identities and corrected network checkpoint

Date:2026-10-20. Status: **PENDING complete regression**, not issue closure.
Historical inventory remains155 IDs,68 final/87 pending. This checkpoint combines
pending088/090/111 with046 and verification of pre-campaign-fixed030;148 receives
only a partial repair and remains open. It supersedes the strict-failed registry
GLES gate without erasing that result; see [registry evidence](REGISTRY_CHECKPOINT.md).

## Source and reference evidence

- CNAf421948fe resolves external XML target identities in namespace/generic scope,
  including declared fixups, without requiring an instantiable target serializer.
  Genuine15/15;17 native cases include two actual writer round trips. Assembly-
  qualified whitespace is accepted, unqualified surrounding whitespace refused.
  Earlier incorrect whitespace expectations and failing candidates are retained.
- CNAe0318ab52/de1b6e259 repairs self-closing sections and per-section fixup order.
  Genuine18/18. An initial whole-corpus sanitizer run exposed changed diagnostics
  and cyclic test-fixture leaks; the follow-up repairs ordering and adds post-
  assertion RAII cleanup. It does not weaken goldens or production ownership.
- Final actual Reader/Serializer translation units, whole intermediate corpus,
  18 section tests and17 external cases pass ASan/UBSan/leak44/44 in565ms.
  Native focused58/58 passes706ms. Full native552/123 suites completes with
  551PASS/one unchanged XACT SKIP,973.298s: all517 previous outcomes unchanged,
  all35 added section/order/external identity cases PASS. Four native hashes match
  afterward. Original-fixture XML consumers6/6PASS4ms. Broader consumers remain
  running; no full consumer pass is inferred from the content gate.
- Network030 was fixed in CNA01534c2e7 before the initial campaign revision.
  Demo0b4d2ce corrects sender-side configuration and the mistaken minimum-latency
  expectation using actual XNA4 simulator27/27 evidence. Exact packet counts and
  six checks remain. See [network evidence](../NETWORK_SIMULATOR.md).

## Completed external focused gate

Existing build/linux-full rebuild succeeded without warnings. Strict56 custom-
content executions (28 per renderer) give25PASS/three remaining FAIL each, with
`errors: []` for every row: metadata, parent/child assertion counts and failure
multisets all match the expected changes. ISSUE-046 gives20/20 on each renderer.
ISSUE-148 retains exactly the string-BOM and inter-element processing-instruction
failures. ISSUE-089 and047/048 remain visible and unchanged.

The rebuilt aggregate runner also passes network030 once per renderer with all
six host checks: SOFTWARE13.319s, OPENGLES3 9.033s. Its previous staged build passed
both renderer selections, and the native simulator/ENet integration gate11/11.
These are local SystemLink tests, not online-service or Xbox hardware validation.

Only after these passes are046/030 markers removed (demos3d1f833). The final marker
build is warning-free and319 artifacts frozen. Both95-case math sweeps preserve
the exact48 expected improvements; metadata/tools and all six pixel-exact visual
references pass. The GLES font capture was also inspected. Both complete719-demo
sweeps, final isolated serial CTest and native consumer/hash audits must finish
before closure. Expected cumulative improvements are60 versus
initial HEAD and62 versus Complete1128; these full results are not yet claimed.

Reports: `build/linux-full/reports/repair-sections-*`,
`repair-external-alias-final-sanitizer.log`, and `repair-network-*`.
No vendor changes, extra platforms, service passes, pushes or merges are claimed.
