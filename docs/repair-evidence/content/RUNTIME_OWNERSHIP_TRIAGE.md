# Next content-runtime batch: source audit, not repair closure

Date:2026-10-20. Current registry and sections SOFTWARE sweeps still reproduce
the documented runtime failures. No production change for these IDs is included
in the pending048 build.

## Concrete current-source findings

- 011: ContentManager::Unload only clears loadedAssets. ContentReader's typed
  RecordDisposable calls an explicit caller recorder, but the manager fallback
  remains a TODO. These are independently actionable from shared-table dispatch.
  Value-handle Texture2D is an additional lifetime concern: copies currently own
  independent wrappers around a shared renderer, and Dispose clears only that
  wrapper. Recording and disposing another copy is not sufficient to invalidate
  the returned handle. Do not close011 on a shared_ptr-only fix.
- 087: InnerReadObjectAny returns the result of ReadUntyped without recording it;
  ReadSharedResources uses exactly that route. The typed reader knows its carrier
  before erasure, so preserve disposable capability at that boundary instead of
  guessing a std::any payload type or adding demo-specific type names.
- 012: ordinary Load still requires the protected loadsThroughOpenStream opt-in.
  The cold ResourceContentManager failure needs an updated root-cause explanation:
  ReadAsset already calls BuiltinContent<T>::Register, but a family invokes
  RegisterDeclaredXnbReaders, which does nothing unless Game previously declared
  the built-ins. Existing family-closure tests explicitly establish Game's declared
  state and therefore cannot prove a cold standalone manager works. Preserve the
  per-type link boundary and explicit-removal semantics when repairing this.
- 043: CreateReaderForTargetTypeEXT still sends the associated target name directly
  to a factory table keyed by reader name. Distinct target/reader identities remain
  necessary in a minimized regression, unlike the existing same-name control.
  CNA explicitly documents fresh unique_ptr instances and a native test requires
  them, whereas genuine XNA caches readers process-wide. Do not silently change
  that documented C++ ownership surface as part of a target-name mapping repair.
  A factory's actual key is known when CreateRegisteredReader creates the table
  reader; any association must also respect factory removal, clear operations,
  constructor callbacks and lock order. Do not eagerly instantiate every factory
  merely to discover its target.
- 045: LoadXnbAsset invokes ParseXnbHeader outside an exception-wrapping boundary.
  Genuine XNA's stream/header and object-body boundaries wrap IOException in
  ContentLoadException; do not indiscriminately wrap caller callback exceptions.
- 039: both RootDirectory setters assign directly. Genuine XNA refuses assignment
  with any loaded asset, even assignment of the same root value.

## Reference preparation

`ContentOwnershipOracle.cs` now **passes35/35 against genuine XNA4.0**. It uses
the real XNA ContentManager and ContentReader with an authored in-memory XNB and
public custom type readers. Executed checks cover inline/shared callback order,
alias fixups, default ownership, partial-load retention, disposal-failure cleanup,
stream closure, virtual OpenStream routing, cache/root transitions and truncated
header/body exception boundaries. No GPU, online service or replacement manager
implementation is involved; only the public stream source is overridden. The
actual SDK framework DLL is used in the existing private Wine reference prefix;
the C#4/x86 warning-as-error build and run both exit0. Reports:
`build/linux-full/reports/repair-content-ownership-xna{-build,}.log`.

The local shipped-XNA decompilation additionally warns against an attractive but
incorrect assumption: RecordDisposableObject appends to its list without identity
deduplication. Unload calls each recorded disposable in order, stops on the first
exception, and clears both lists in finally. The genuine oracle now confirms this,
including three disposal calls when a custom reader returns one identity for
three separate materializations. A shared alias fixup itself adds no callback.
Caller recorder exceptions remain InvalidOperationException and do not transfer
ownership back to the manager; truncated header/body failures instead become
ContentLoadException. Source paths are under
`../xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework/`.

The local FNA checkout at1358793096d556388be798d24dc3728b2abb10ff differs in
these exact places: its RecordDisposable suppresses duplicate identities,
RootDirectory is an unrestricted auto-property, and Unload lacks XNA's finally
cleanup. CNA's owner policy says measured XNA wins. A future implementation must
record these specific departures from FNA in the required CNA divergence log;
do not substitute FNA's convenient deduplication for the measured35-check result.

Next: add corresponding native before/after tests. Keep011,012,039,
043,045 and087 open until their own repairs and full relevant gates finish.

## ISSUE-039 native baseline

Three native cases are now prepared in ContentManagerXnbTests.cpp and were linked
against the frozen048 dependency closure, without rebuilding checkpoint binaries.
Result:twoPASS/oneFAIL,3ms. Both lvalue and rvalue RootDirectory setters wrongly
accept same-root and different-root assignment while an asset is cached, and mutate
the root. Positive controls confirm cache preservation and reload after Unload;
failed Load and uncached ReadAsset correctly leave root assignment available.

The first Werror compilation exposed six pre-existing ignored-nodiscard results in
the reused fixture. Exception assertions now explicitly discard the value, and the
initial cached-load control is strengthened to assert its exact value. No exception
assertion is removed or relaxed. The corrected compilation passes Werror; the
original compile failure remains in `repair-content-root-native-before-build.log`.
Actual baseline results: `repair-content-root-werror-native-before.log`.
No039 production repair or post-fix pass is claimed yet.

## ISSUE-045 boundary and bounds baseline; candidate039/045 repair

Expanded genuine oracle passes89/89 (strict C#4/x86): all header lengths0..9
through Load and ReadAsset, stream closure, root mutation after failed loads, and
reader IOException versus InvalidOperationException are verified. The original
35-check run remains archived; the new report is `repair-content-envelope-xna.log`.
Four native envelope cases fail as expected in10ms against frozen048 dependencies:
short typed/raw/untyped headers leak EndOfStreamException, and the body boundary
also leaks IOException. User InvalidOperationException is already preserved.

An additional actual-TU ASan baseline confirms a **four-byte heap-buffer-overread**
in ContentManagerUntyped.cpp:154 when an external XNB declares a ten-byte LZX
container. The typed path checks the compressed14-byte prefix; the erased path
does not and constructs a four-byte MemoryStream at the end of the input vector.
The retained report `repair-content-envelope-sanitizer-before.log` identifies the
read immediately after the ten-byte allocation. This is a related045 parser-boundary
subfinding, not a fabricated runtime pass or a new historical issue ID.

The uncommitted candidate moves existing header/range checks into one private
ContentManager::ReadXnbHeader shared by both paths, adds IOException wrapping at
the manager header and typed/erased ContentReader asset boundaries, and preserves
the low-level ParseXnbHeader EndOfStream contract and non-I/O user exceptions.
Both RootDirectory setters reject a nonempty cache before assigning, matching XNA
rather than FNA's unrestricted property. The required divergence log is updated.

Actual ContentManager.cpp plus ContentManagerUntyped.cpp and the whole manager
fixture pass ASan/UBSan/leak **23/23 in29ms**, exit0, after the candidate. Eight new
native cases cover039/045; unchanged positive controls also pass. Report:
`repair-content-envelope-sanitizer-after.log`. The candidate has NOT yet entered
the canonical native build or external demos: both048 checkpoint binary sets
remain frozen. Neither039 nor045 is closed; complete native/external regression
is still required. Other ownership findings011/012/043/087 remain open.
