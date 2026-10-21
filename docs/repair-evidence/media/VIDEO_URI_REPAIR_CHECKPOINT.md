# ISSUE-101 URI dependency and CNA candidate

2026-10-21. Starting revisions: CNA ba56ddf5a, demos ca6310b, Sharp Runtime
97713eb292093b4d81f7e29c1965d5a317a57680. No issue is closed by this checkpoint.
The earlier VIDEO_REPAIR_VALIDATION_CHECKPOINT.md remains the historical099/100
gate with rejected C regressions. Those C assertions and public contracts are
unchanged; the owner's099 choice remains unanswered. No new push or merge.

## Reproduction and reference

Original URI9:5FAIL/4PASS against native-v2. New file-boundary5: allFAIL against
the same old native library and its actual Sharp Runtime IO archive; strict build
2025ms, run240ms, audit22ms. The initial standalone link attempt failed because
the old shared library did not export unused File::Exists; it was not a runtime
test. The corrected attempt links the actual existing archive, without changing
any assertion. Logs preserve both attempts.

`UriLocalPathOracle.cs` was built by genuine Framework4 csc and run on that CLR
under Wine and .NET8.0.31/Linux:34 cases each; build2883ms, runs640/157ms,
unchanged source/compiler hashes, peak308244480 bytes and no memory events.
It classifies inputs only, with no network/file access. Ordinary escaped file
paths, NUL preservation, DOS/UNC and relative guards agree. Actual differences:
Framework4 decodes malformed UTF-8 differently, retains a file query in LocalPath
and normalizes some backslashes that modern Linux .NET retains. New runtime
getters follow measured.NET8/Linux plus local.NET11-preview source. This is not
an all-versions or native-Windows parity claim.

FNA b35512475ed7980169574d2c40927381c1764d5a provides FromUriEXT's file/relative/
non-file behavior. The factory is an FNA extension, not an XNA Video API.
The candidate adds no CNA URI parser: it uses System::Uri, File::Exists and the
existing TitleLocation bridge. Native absolute strings keep literal percent signs.
Trusted metadata construction still performs no IO; missing-file Play detaches
the failed association and throws FileNotFoundException. Existing but undecodable
files still take the pre-existing silent decoder-failure path: **not** a claim of
complete FNA codec-failure parity. The C Play contract needs separate review.

## Completed runtime gates

| Gate | Result |
| --- | --- |
| Baseline before-v2 |18,158/18,158 PASS across41 executables; URI311; build101ms/full50743ms |
| URI candidate after |18,192/18,192 PASS; URI345; build143545ms/full49168ms |
| URI + File.Exists after-v2 |18,193/18,193 PASS; URI345; build27807ms/full49314ms |
| Module graph |44 physical modules,109 dependency edges; valid |

Zero build diagnostics, test failures or skips in all accepted gates. The first
baseline attempt had five fixture-path skips because its default samples path was
stale. Accepted reruns use XNA_SAMPLES_ROOT with a symlink view of the unchanged
original ShipGame/Spacewar samples. No skip allowance or source-fixture edit.
Existing SOAP harness remains private loopback. Latest runtime peak830410752
bytes, all memory events0, frozen input hashes unchanged. One worker/shared cache,
existing build/, no temporary build tree. Migration/source/version boundaries and
downstream spelling inventory are in runtime docs/Migration-UriLocalPathAndFileExists.md.

## CNA validation now running

Latest: native-v3 job59093 completed build18,503ms/zero diagnostics, focused43/43
PASS1,037ms and media345/345PASS12,715ms. Full10948 is RUNNING, not a completed
result. Previous14068 stopped before build: its source guard found libcna.so
rebuilt at step52 of18544. The driver now records that intentional build-artifact
replacement and rehashes it after rebuilding; unrelated source hashes are still
required to match. No assertion change or retry of a test failure is involved.

Update: runtime repair is local commit c26614e85dffff4cadc5c4803b00d78953706b21,
clean runtime worktree, no push. Native18544 failed linking CnaMediaTests after
324516ms: the new public TitleLocation witness belongs to cross-module tests,
not the isolated media link closure. Moved only its native copy to
tests/MediaVideoUriContractTests.cpp, preserving all assertions/public calls;
no production dependency expansion. The standalone original remains in this
evidence directory. Native-v2 job14068 now runs43focus through CnaTests,
345 media (original340 plus5boundary), and10948 full. Full inventory unchanged
by the relocation. Prepared sanitizer uses the canonical CnaTests link closure
for the cross-module fixture. Original failure preserved, not treated as a pass.

Native job18544, cna-repair-video-uri-native.scope, is the only heavy job. It
builds CnaTests/CnaMediaTests in cmake-build-debug, then requires43 focused tests,
354 media tests and full10948 exact outcome parity against the prior10934.
Private GPU/network/owner UID/no capabilities;4GiB high/6GiB max/no swap; one worker.
These counts are planned, **not yet a runtime result**. Sources are frozen.

The original URI9 witness now uses PathToGenericUtf8 instead of generic_string
for filesystem→API text, avoiding Windows ANSI narrowing. Linux input bytes and
all assertions are unchanged; native and evidence copies match. Existing
FromUriEXT local-file tests now provide an explicit absolute fixture path; they
still test the same actual file, independent of global TitleLocation left by
earlier tests. New relative-URI witness protects title-vs-current-directory behavior.

Prepared, not run: repair-video-uri-sanitizer.mjs (43 original witnesses, actual
instrumented Video/runtime); repair-video-uri-external.mjs (actual external
library,43 witnesses,112 unchanged scenarios, pure-C smoke). C regressions still
reject the external gate; no new expected failure is introduced.

Reports: build/linux-full/reports/repair-uri-local-path-oracle-*,
repair-uri-runtime-{before,before-v2,after,after-v2}-*, repair-video-uri-before-*,
repair-video-file-path-before-{*,v2-*}, repair-video-uri-native-*.

Next: inspect native outcome comparison; run sanitizer/external sequentially,
record results and coherent dependency commit. Preserve inherited011 and all
historical issue IDs.099/100/101 remain open; matrix155 IDs/97final/58pending.
