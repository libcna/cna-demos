# ISSUE-038: dynamic audio disposal guard preparation

**Resolved:** CNA85a23569cf8df6ce048eb34fcd9bb0f30ba54f53. The four guards,
native regressions and preserved private cleanup/stress checks pass the full native,
integrated sanitizer27, external112 and32native/19demo CTest gates. The original
streaming demo retains all13 checks, without its knownIssue marker. See
[the accepted checkpoint](AUDIO_INSTANCE_GUARDS_CHECKPOINT.md). Earlier RUNNING/
not-fixed statements below are retained history, not current status.

16641 native-v4 accepted: full10905=10804PASS/101sameSKIP, every prior10882 outcome
preserved with the two explicitly corrected genuine-XNA helper IDs and23newPASS;
full1202840ms/compare115ms, audio775=771PASS/4sameSKIP, focus175PASS. Source hashes
unchanged;peak1140305920/all memory events0. The previous v3 comparison failure
from forced-X11 Wayland SKIP remains recorded and was corrected only in the launcher.
76943 integrated sanitizer27/27PASS includes both prior cleanup/stress assertions
and corrected helper cases, as well as the original23 new fixtures. Audio and
Sharp Runtime are instrumented; ASan/UBSan/float-cast-overflow/leaks enabled, no
suppressions/findings. Library build32465ms, strict fixtures15999ms/run223ms/XML22ms;
hashes unchanged,peak1258749952/events0. Original external112 gate37318 is running;
both knownIssue markers remain. No038 closure before real demos and final gates.

41722 focused candidate completed0: native build116708ms/no diagnostics; unchanged
five-case fixture5/5PASS (strict compile2702ms/run53ms/XML29ms); pan18/18 alsoPASS.
Hashes unchanged,peak1260371968/all events0.21670 native audio/focus/full gate is
running; demo knownIssue stays until its original runtime assertions pass.

Native34449 completed the unchanged five-case fixture: exactly four disposal
failures and the existing setter control PASS. Strict compile1968ms/run45ms/XML22ms;
hashes unchanged,peak410947584/all events0. Reports `repair-dynamic-audio-disposed-before-*`.
Local candidate adds the four missing guards and copies this exact fixture into
CNA's audio tests. The older disposal queue-cleanup test keeps its actual zero
buffer assertion through a narrow read-only friend seam, plus the public refusal.
Two FNA-derived helper no-throw tests are corrected and renamed to ThrowsObjectDisposed;
full regression must explicitly map these two IDs, retaining their inputs and
checking the newly measured XNA exception contract. 41722 focused candidate build
and original23-case validation (including073) is RUNNING; no closure yet.
Older preparation statements below are historical.

18009 TERMINAL0 (2026-10-21): all24 genuine managed controls PASS with exact
Framework hash/MVID and unchanged source. Strict C# build3248ms, run780ms;
peak268726272/all memory events0 in a separate1GiB no-swap scope. Reports
`repair-dynamic-audio-disposed-xna-*`. The native five-case fixture remains
UNCOMPILED/UNRUN. This verifies real XNA guard/exception order on the deliberately
limited managed fixture, not native voice creation, disposal or audible output.

2026-10-21. Read-only source investigation while015 external regression runs.
No production repair, new runtime reproduction or closure is claimed here.

The current installed genuine XNA4 Framework identity is the same as the media
path reference: SHA-256
`8b7aa4c4939f243b21432747281cc8aacdcda56191a16d9eaa036b4136cf0da4`,
MVID `34d977ae-c3ec-4d07-aa6d-6fed6d8e3864`.
Its complete methods in `build-probe/repair-media-reference/framework.il` guard
IsDisposed before accessing native state or validating the argument:

- DynamicSoundEffectInstance.GetSampleDuration: RVA0x1b0d0;
- GetSampleSizeInBytes: RVA0x1b15c;
- get_IsLooped: RVA0x1b280;
- get_PendingBufferCount: RVA0x1b364.

The last two are the historical finding. The first two share the missing disposal
check and have native tests explicitly preserving the FNA divergence. The genuine
XNA contract wins; those tests are not justification to keep returning values from
a disposed object. This is distinct from changing valid live-instance numerical
conversion rules, which should not be broadened into this repair without evidence.

Prepared, NOT compiled/run:

- `DynamicAudioDisposedOracle.cs`:24 genuine managed-boundary checks. A reflection
  fixture supplies only the disposed flag and lock needed by the guard; finalization
  is suppressed and no native voice is created. It checks concrete/base dispatch,
  exact exception type/object name, zero/positive/invalid conversion arguments and
  the existing loop setters. It cannot establish real voice creation or disposal.
- `DynamicAudioDisposedTests.cpp`: five native cases. Four are expected to fail
  before the guard repair; the existing setter guard is a positive control. Uses
  real CNA construction/Dispose, no playback, no added waits or ignored assertions.

The native existing PendingBufferCountResetsToZeroAfterDisposeWhilePlaying case
currently checks cleanup through the disposed public getter. Preserve its cleanup
test through a narrow private test seam if changing that public getter to throw;
do not replace a resource-cleanup assertion with a bare exception and lose coverage.
Keep its original test ID and the live Stop cleanup controls. The two existing
sample-helper after-dispose tests likewise need an explicit XNA-justified expected
exception, not removal. No changes have been applied to these tests yet.
