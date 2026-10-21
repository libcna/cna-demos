# ISSUE-099/100 candidate validation and ISSUE-101 baseline

2026-10-21. CNA `d1ed3d9ae` / demos `c847549` plus the recorded uncommitted
099/100 candidate and inherited, unaccepted011 changes. This is **not** an accepted
product checkpoint: the original C VideoPlayer contract currently regresses.
No production repair was committed, no marker removed, and nothing pushed or merged.

## Completed gates

| Gate | Result |
| --- | --- |
| Native build | 45,964 ms; zero warnings/errors |
| Native media | 340/340 PASS; 11,990 ms |
| Full native | 10,833 PASS, 101 unchanged SKIP; 1,328,138 ms |
| Native outcome comparison | All original10,905 outcomes preserved, one reference-corrected Volume test rename, exactly29 newPASS; 121 ms |
| Integrated sanitizer (previous gate) | 29/29 PASS; actual Video/VideoPlayer and Sharp Runtime instrumented, ASan/UBSan/float-cast/leaks; no findings/suppressions |
| External full build | 129,390 ms; zero warnings/errors |
| Original29 against external library | Strict build8,254 ms, run643 ms, XML audit20 ms; allPASS |
| External112 scenarios | Each renderer49PASS/6FAIL/1manual NOT RUN; **not accepted** |
| Unchanged pure-C VideoSmoke | Strict build317 ms; run212 ms exits2; **FAIL** |
| ISSUE-101 focused reproduction | Strict build2,332 ms; run351 ms; audit23 ms; fourPASS/five reproducedFAIL, not a product pass |

Native session86932, external37654, URI baseline94692 all terminated. No heavy
job remains. All frozen hashes were unchanged. All jobs were serial, used one
compiler worker, shared ccache, existing build directories, private GPU displays,
6GiB hard memory limit and no swap. Native full used an isolated network namespace.
Native peak3,127,988,224 bytes; external peak4,302,004,224 bytes with1,192 soft-limit
events; URI peak475,160,576 bytes. Hard-limit/OOM/OOM-kill events were zero throughout.
The external soft limit throttled the job; this is not a zero-throttling claim.

## Exact external delta and unresolved C decision

On **both SOFTWARE and OPENGLES3**, the unchanged C++099 pause/loop scenario now
passes all27 checks and100 track selection passes all9 checks, including its
public-pixel detached Video preference. Previously each was a known failure.
Five unrelated historical failures and the manual microphone case remain unchanged.
No other scenario changed status, check count, marker or failed-assertion set except
the formerly passing `c_api.media.video_player`, which now fails these five of its
33 unchanged assertions:

- Fresh frame query should report absence and generation zero.
- GetTexture before Play should succeed with no frame.
- Volume should clamp below/above the public range.
- Stop should clear the associated Video and reset state/position.
- The stopped frame should be absent without resetting lifetime generation.

`VideoSmoke.c` independently fails its initial fresh-texture expectation. The
original reports retain every failure; none became a known issue. C API source,
headers, specification, smoke and demo expectations were not edited. Genuine XNA
evidence and the candidate C++ contract instead require a fresh-texture refusal,
range refusal, and retained Video/frame after Stop. The owner's choice between
explicit XNA/C++ alignment and deliberate C-adapter differences remains unanswered.
The earlier039 RootDirectory approval is not treated as authorization for099.

The first comparison also flagged four **log-parser false positives** for the
two improving C++ scenarios: `DemoContext::Check` logs only failed labels, whereas
standalone C logs every assertion. Offline reanalysis verified the exact source
bytes against accepted demos `2cc3fca`, retained counts27/9, and confirmed disappearance
of the old failures. It removes only those four parser findings. The two C renderer
regressions and pure-C failure remain, and the gate still exits1. The original audit
was not overwritten; the separate reanalysis records its SHA-256 and corrections.

Reports under `build/linux-full/reports/`:

- `repair-video-player-native-v2-{audit.json,compare.log,full.xml,media.xml}`
- `repair-video-player-sanitizer-v2-*`
- `repair-video-external-*` and `repair-video-external-reanalysis.json`
- `repair-video-uri-before-*`

The pure-C linkage check verifies external `libcna_c_api.so`: that library embeds
CNA static module archives and does not dynamically depend on `libcna.so`. The
C++ witnesses separately verify actual external `libcna.so` linkage. No stale native
library was accepted as the external implementation.

## ISSUE-101 preparation

`VideoUriContractTests.cpp` now reproduces five named failures: absolute file URI,
percent-escaped filename, title-relative URI with a different current directory,
non-file URI exception type, and deferred missing-file refusal. Four controls pass:
plain absolute path, raw missing-file refusal, trusted metadata without IO, and
atomic metadata-mismatch refusal. Existing generated-demo101 failures remain exactly
the original four on both renderers; the percent-escape test adds a focused control.

FNA `b35512475ed7980169574d2c40927381c1764d5a` resolves file URIs through
`System.Uri.LocalPath`, relative URIs through TitleLocation, and refuses non-file
absolute URIs. Microsoft's [LocalPath documentation](https://learn.microsoft.com/en-us/dotnet/api/system.uri.localpath?view=net-6.0)
specifies an unescaped local path; this supports the `%63` filename test. This is
FNA extension evidence, not an XNA Video API claim. The current Sharp Runtime URI
surface lacks LocalPath/IsFile and discloses no percent decoding. Repair URI semantics
at that layer after reading its complete guidance and validating the .NET contract;
do not duplicate URI parsing in CNA. No101 production code was changed.

## Next task and boundaries

Resolve the099 C contract choice before committing its production repair or
accepting the batch. Implement the chosen adapter/explicit contract correction,
preserve all behavior assertions, then repeat affected native, sanitizer, external
and registered gates. Keep100 open until coherent source/metadata acceptance.
For101, address the URI substrate and separately review the documented C Play
success-on-undecodable-file extension; absence and invalid codecs are not automatically
the same contract. Preserve all011 ContentReader/ContentTypeReader and ownership work.

Campaign totals remain155 IDs:97 final /58 pending. No final full719 smoke, complete
CTest sweep, visual-reference comparison, physical audio, decoder-free runtime,
Windows/macOS/iOS runtime or third-party sanitizer coverage is claimed here.
