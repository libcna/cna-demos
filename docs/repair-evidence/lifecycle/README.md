# Prepared lifecycle audit (141–144; not repaired)

Read-only audit while the geometry checkpoint is frozen. No lifecycle code or
demo assertions have been changed. The fresh `ComponentContractsOracle.cs` now
passes29/29 genuine checks (`repair-lifecycle-contracts-xna{,-build}.log`): hook
dispatch/suppression and identity, repeated explicit disposal/finalizer silence,
collection index ordering/null Clear events, and launch parsing/duplicate Add.
This does not yet validate the separate live-Game removal/ownership path.

Current CNA still contains all four reported causes:

- 141: GameComponent::Dispose(bool) returns when disposed_ is set, suppressing
  the next explicit Disposed event. Genuine GameComponent.Dispose(bool) also
  removes the component from Game.Components before dispatch; CNA does not.
  Do not blindly add that removal: CNA's owning shared_ptr collection can destroy
  its last owned component inside Remove. Inspect Game teardown/iteration and
  preserve lifetime through dispatch, with an ASan regression, before changing
  this path. Destruction must not dispatch a managed-style event. Genuine Game's
  Dispose copies its component collection to an array before invoking Dispose;
  CNA currently walks the live shrinking collection forward. A lifetime-holding
  snapshot is therefore a reference-backed companion, not an arbitrary redesign.
- 142: all four property setters raise events directly, then call empty virtual
  hooks. Genuine setters call only the hooks; the base hooks dispatch with `this`
  as sender. An override omitting base suppresses the event. Existing native
  GameComponentTests.cpp and DrawableGameComponentTests.cpp are two-line stubs,
  so new native tests need a real Game on the private display, not a fake reference.
- 143: CNA checks duplicates before Insert's range. Genuine Collection.Insert
  validates its public index before invoking the overridden InsertItem, explaining
  why looking only at the XNA InsertItem decompilation gives the wrong conclusion.
  Clear raises for every slot including null in genuine XNA. CNA skips null there.
  Add/Remove null silence and pre-clear event observation must remain distinct.
- 144: CNA drops flags with no value and ignores duplicate Add. Genuine parser
  splits at the first colon, skips empty keys, preserves empty values, trims all
  leading slash/dash characters, and keeps the first duplicate parsed key.
  It skips argv[0]. CNA's explicit-vector constructor is a documented extension
  taking arguments without an executable entry, whereas its default constructor
  reads the real process argv. Preserve that distinction: do not skip the first
  explicit argument or accidentally insert the executable as a parameter.
  Current native tests explicitly expect the three wrong empty-value behaviors.

References inspected: local genuine Windows Game assembly decompilation's
GameComponent.cs, DrawableGameComponent.cs, GameComponentCollection.cs and
LaunchParameters.cs; matching CNA public headers/sources; native collection and
launch tests; demos game.components.* and game.launch_parameters.parse; permanent
issue141–144 histories. Next: reproduce exact contracts with a small genuine
runtime probe, then add failing native tests before any product edit. Keep this
batch separate from geometry and the prepared Color132 work.
