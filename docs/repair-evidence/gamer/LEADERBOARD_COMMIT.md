# ISSUE-025: offline leaderboard final column writes

Validated/published085 baseline: CNA b6d5ca097f2cd2982cc797690227659b73c829cf,
demos90d98d66e2daecb412ffd77a9d74128559d8ecd9. Streams now persist when Rating commits;
the separate025 timing defect remains. GamerServices737=736PASS/1declaredSKIP,35 gamer scenarios
per renderer=34PASS/1manualNOTRUN. Last full cross-subsystem checkpoint is still075.

Source confirms offline writers install only the immediate Rating hook. NetworkSession's
Playing->Lobby transition finalizes service writes only when leaderboardGameplay_ is nonempty;
offline LocalWithLeaderboards has no final column flush. Thus a column mutation after Rating,
including a write through a retained stream or std::any reference, is absent from the store.
The historical reproduction uses Rating4200 then Result=Win, EndGame/Update, and a public read.
Available genuine Windows XNA leaderboard classes are ProFeatureNotSupported stubs; no live Xbox
service result is claimed. The documented final-row/game-boundary behavior and CNA's working
service commit path support retaining the original025 expectation, not the ordering workaround.

Six deterministic native witnesses precede product changes. In addition to the original order,
they cover references retained before StartGame, both aliases (SignedInGamer/LocalNetworkGamer),
an untouched cached entry not overwriting another writer, merely opening a writer not creating
a row, and failed final stream persistence leaving the previous row and game state recoverable.
The first build/red run is pending. No025 runtime changes or demo oracle changes yet.

Repair direction under review: compare the actual final column contents with each writer's last
loaded/successfully persisted snapshot, and flush changed offline rows at EndGame before GameEnded.
Retain immediate Rating persistence for explicit offline fixtures. An unconditional save of every
cached row would corrupt another writer's update; a callback before SetValue misses the actual
mutation and retained references/streams. Avoid either shortcut. Keep online service behavior and
unrelated Local sessions intact. No new public XNA API is needed.

The first red run failed all six fixtures at setup: maxGamers=1 is outside CNA's documented
2..31 range. This is a fixture mistake, not reproduction evidence. Corrected to4, rebuilt, and
reran without product edits:3FAIL/3PASS in0.006s. The original Result lookup throws map::at;
retained scalar/stream values stay1 instead of2/9; an oversized final stream is not checked and
GameEnded is raised. Both stale-cache controls and the no-write/no-row control already pass.
Reports repair-leaderboard-commit-before-native2.{xml,log}; first setup failure is retained.

Candidate keeps a canonical snapshot produced by the same local column serializer as persistence.
The Rating hook prepares the snapshot, saves, then swaps the baseline without allocation; failure
does not advance it. At LocalWithLeaderboards Playing->Lobby, both local gamer writer aliases
flush only changed contents before readiness/state/GameEnded. Service-backed writers remain on
their existing transaction path; Local sessions are untouched. Comparing contents catches retained
stream/reference mutations without adding per-byte callbacks. No025 closure pending validation.

Native full validation passes: Net564/564 in48.590s (all558 previous018 results retained, six
newPASS); GamerServices737=736PASS/1unchanged declared rendererSKIP in75.187s, exact parity with085.
Build48 pre-existing nodiscard warnings, no errors/new fixture warnings. Rebuilt subprocess harness.
Targeted sanitizer validation and native C/consumer rebuilds remain pending.

The demo removes its columns-before-Rating workaround, preserving all21 assertions. It first
clears its own row's columns and commits Rating, then writes the expected scalar/stream columns
before EndGame. This makes the reproduction deterministic on repeated runs: a previous correct
row cannot accidentally supply the missing final values. The prior085 stream assertion and exact
nine-column checks remain. No025 knownIssue marker existed, so none is converted or suppressed.

Added a seventh native control after the initial validation: LocalNetworkGamer's own writer must
also persist post-Rating columns. It is not claimed as separately run before repair. Final full
Net565/565PASS48.580s, all558 previous outcomes retained plus7PASS. GamerServices remains the
exact737-result run above. Seven rebuilt native C gamer/net/boundary controlsPASS0.24s.
Final targeted ASan/UBSan/leak validation48PASS0.029s: eleven translation units instrumented,
including NetworkSession and GamerConstruction in addition to the085 closure. Other native static
dependencies and third-party libraries are not instrumented. No findings, no changed frozen inputs.
The first sanitizer47-pass run is retained; v2 adds the seventh session control, not a retry to
hide a failure. Reports repair-leaderboard-commit-sanitizer-v2-* contain exact commands and scope.

Final consumer validation complete:
- Complete consumer buildPASS with no compiler warnings; generated3197 routes/57 files current.
- SOFTWARE59 and OPENGLES3 59 each58PASS/1manualNOTRUN. Exact085 gamer/018 net comparison
  preserves every outcome, check count, marker, message and logged assertion. The changed paging
  setup now proves post-Rating persistence while retaining all21 assertions, including085 coverage.
- Eleven registered focused/catalog CTestsPASS26.40s. Catalog719 is byte-equivalent as parsedJSON
  to085 and validates cleanly. All275 frozen executable/library hashes verify after the complete
  sequence; freeze released. No025 expected-failure marker was removed because none existed.
- No new full719 smoke/native aggregate/visual/other-platform/live-service claim. The last full
  cross-subsystem checkpoint remains075 with six separately tracked residual CTest failures.

Validated source repair: `8a5e5d8aa50c56599ddc8649b738cb2ca4caab15`.
025 permanent disposition FIXED_VALIDATED;156 historical issues now129 final/27 without.
Next batch028 uses the existing genuine constructor/path and retained-file triage; the historical
title-root expectation is wrong and must not be implemented. No028 code changed by this batch.
