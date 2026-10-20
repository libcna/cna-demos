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
- 045: LoadXnbAsset invokes ParseXnbHeader outside an exception-wrapping boundary.
  Genuine XNA's stream/header and object-body boundaries wrap IOException in
  ContentLoadException; do not indiscriminately wrap caller callback exceptions.
- 039: both RootDirectory setters assign directly. Genuine XNA refuses assignment
  with any loaded asset, even assignment of the same root value.

## Reference preparation

`ContentOwnershipOracle.cs` is **prepared, not yet compiled or executed**. It uses
the real XNA ContentManager and ContentReader with an authored in-memory XNB and
public custom type readers. Planned checks cover inline/shared callback order,
alias fixups, default ownership, partial-load retention, disposal-failure cleanup,
stream closure, virtual OpenStream routing, cache/root transitions and truncated
header/body exception boundaries. No GPU, online service or replacement manager
implementation is involved; only the public stream source is overridden.

The local shipped-XNA decompilation additionally warns against an attractive but
incorrect assumption: RecordDisposableObject appends to its list without identity
deduplication. Unload calls each recorded disposable in order, stops on the first
exception, and clears both lists in finally. The prepared oracle must measure this
before implementation. A supplied callback similarly must not be silently
deduplicated. Source paths are under
`../xna4-decomp/reference/xna4/decompiled/windows/Microsoft.Xna.Framework/`.

Next: execute the genuine probe when a worker is free, preserve any expectation
corrections, then add corresponding native before/after tests. Keep011,012,039,
043,045 and087 open until their own repairs and full relevant gates finish.
