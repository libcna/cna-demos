# ISSUE-038: dynamic audio disposal guard preparation

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
